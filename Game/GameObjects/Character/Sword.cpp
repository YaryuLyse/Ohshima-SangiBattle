/**
* @file Sword.cpp
* @brief 敵の武器（剣）を管理するクラスの実装ファイル
* @brief 敵が装備する剣のモデル描画や攻撃時のアニメーション制御をまとめています
* @brief このクラスは、敵の身体の回転に同期した位置計算や、攻撃ステートに応じたスイング動作（回転）の更新を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "Sword.h"
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include "Game/GameObjects/Character/Character.h"
#include "Game/State/AttackState.h"
#include "Game/State/GuardBreakState.h"
#include "Game/State/DeathState.h"
#include "LyseLib/DebugFont.h"

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// エフェクト設定
	constexpr int ALPHA_REF_VALUE = 255;

	// モデルファイル名
	// ※元のコードのパス（スペース含む）を維持しています
	const wchar_t* MODEL_FILENAME = L"Resources/Models/medieval  dagger .sdkmesh";

	// 初期配置
	constexpr float INIT_POS_X = 0.0f;
	constexpr float INIT_POS_Y = 0.5f;
	constexpr float INIT_POS_Z = 5.0f;

	// 描画時のオフセット（剣の初期位置）
	constexpr float RENDER_OFFSET_X = -3.0f;
	constexpr float RENDER_OFFSET_Y = 0.0f;
	constexpr float RENDER_OFFSET_Z = -2.0f;

	// 描画時のスケール
	constexpr float RENDER_SCALE = 0.5f;

	// アニメーション設定
	constexpr float SWING_ROTATION_BASE_ANGLE = 90.0f; // 基準回転角度
	constexpr float SWING_SPEED_MULTIPLIER = 1.0f;  // 回転速度倍率

	// 回転速度 (度/秒)
	constexpr float ROT_SPEED_ATTACK = 180.0f; // 90 * 2
	constexpr float ROT_SPEED_NORMAL = 90.0f;

	constexpr float THRUST_SPEED = 100.0f;
	constexpr float MAX_THRUST = 4.0f;

}

/**
* @brief コンストラクタ。デバイスの初期化や武器モデルのロードを行う
* @param device : DirectXデバイス
* @param fx : エフェクトファクトリ
*/
Lyse::Sword::Sword(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx)
	: m_rotateX(0.0f)
	, m_rotateZ(0.0f)
	, m_rotate(0.0f)
	, m_attackTimer(0.0f)
	, m_attacksTimer(0.0f)
	, m_guardBreakTimer(0.0f)
	, m_thrustOffset(0.0f)
	, m_thrustDistance(0.0f)
	, m_character(nullptr)
{
	// アルファテストエフェクトの作成（透過処理の設定）
	m_batchEffect = std::make_unique<AlphaTestEffect>(device);
	m_batchEffect->SetAlphaFunction(D3D11_COMPARISON_EQUAL);
	m_batchEffect->SetReferenceAlpha(ALPHA_REF_VALUE);

	// 入力レイアウト生成
	void const* shaderByteCode;
	size_t byteCodeLength;
	m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	device->CreateInputLayout(
		VertexPositionTexture::InputElements,
		VertexPositionTexture::InputElementCount,
		shaderByteCode,
		byteCodeLength,
		m_inputLayout.GetAddressOf()
	);

	// 共通ステートの作成
	m_states = std::make_unique<CommonStates>(device);

	// モデルのロード
	// Resources/Models/medieval  dagger .sdkmesh を読み込む
	m_sword = DirectX::Model::CreateFromSDKMESH(
		device,
		MODEL_FILENAME,
		fx
	);

	// テクスチャ設定（m_Textureがロードされていれば適用される）
	m_batchEffect->SetTexture(m_texture.Get());
}

/**
* @brief デストラクタ
*/
Lyse::Sword::~Sword()
{
}

/**
* @brief 武器の初期化処理
*/
void Lyse::Sword::Initialize()
{
	// 初期変数の設定
	m_position = DirectX::SimpleMath::Vector3(INIT_POS_X, INIT_POS_Y, INIT_POS_Z);
	m_prePosition = DirectX::SimpleMath::Vector3::Zero;
	m_scale = DirectX::SimpleMath::Vector3::One;
	m_rotation = DirectX::SimpleMath::Vector3::Zero;
}

/**
* @brief 武器の更新処理（親モデルとの同期）
* @param elapsedTime : 経過時間
* @param position : 親モデルの座標
* @param rot : 親モデルの回転行列
*/
void Lyse::Sword::Update(float elapsedTime, DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix rot)
{
	// キーボード状態の更新
	auto kb = DirectX::Keyboard::Get().GetState();
	m_keyboardTracker.Update(kb);

	// 敵の位置と回転情報を更新
	m_charaPos = position;
	m_charaPos.y += 0.5f; // 剣の位置を少し上にオフセット
	m_charaRot = rot;
	// ステートに応じた剣の回転処理
	if (m_character->GetCurrentState() == m_character->GetAttackState()) 
	{
		m_attackTimer += elapsedTime;

		// フェーズ1：構え（00.2秒）
		if (m_attackTimer < 0.3f) 
		{
			m_rotateX = DirectX::XMConvertToRadians(0.0f); // 上に構える
			m_rotateZ = DirectX::XMConvertToRadians(20.0f); // 少し斜めに構える
		}
		// フェーズ2：振り下ろし（0.2秒以降）
		else 
		{
			m_rotateX += DirectX::XMConvertToRadians(ROT_SPEED_ATTACK) * elapsedTime;
			m_rotateZ = DirectX::XMConvertToRadians(20.0f); // 少し斜めに構える
		}
	}
	else if (m_character->GetCurrentState() == m_character->GetGuardBreakState()) 
	{
		m_guardBreakTimer += elapsedTime;

		if (m_guardBreakTimer < 0.5f) 
		{
			// ため：オフセットを後ろに
			m_thrustOffset = DirectX::SimpleMath::Vector3::Transform(
				DirectX::SimpleMath::Vector3(0.0f, 0.5f, -1.5f), rot);
			m_rotateX = DirectX::XMConvertToRadians(70.0f);
			m_rotateZ = DirectX::XMConvertToRadians(20.0f);
		}
		else if(m_guardBreakTimer > 0.7f) 
		{
		// 突き：オフセットを前に蓄積
			if (m_thrustDistance < MAX_THRUST) 
			{
				m_thrustOffset += DirectX::SimpleMath::Vector3::Transform(
					DirectX::SimpleMath::Vector3(0.0f, 0.0f, THRUST_SPEED), rot) * elapsedTime;
				m_thrustDistance += THRUST_SPEED * elapsedTime;
			}
			m_rotateX = DirectX::XMConvertToRadians(90.0f);
			m_rotateZ = DirectX::XMConvertToRadians(0.0f);
		}
	
	}
	else 
	{
		// それ以外：リセット
		m_guardBreakTimer = 0.0f;
		m_attackTimer = 0.0f; 
		m_attacksTimer = 0.0f;
		m_thrustOffset = DirectX::SimpleMath::Vector3::Zero;
		m_thrustDistance = 0.0f;
		m_rotateX = 0.0f;
		m_rotateZ = 0.0f;
	}

	// 死亡時はその場で回転しながら縮小して消える
	if (m_character->GetCurrentState() == m_character->GetDeathState()) 
	{
		m_dropTimer += elapsedTime;

		// 回転
		m_rotateZ += DirectX::XMConvertToRadians(360.0f) * elapsedTime;

		// 徐々に縮小して消える
		float scale = 1.0f - (m_dropTimer / 2.0f); // 2秒で消える
		scale = std::max(scale, 0.0f); // 0以下にならないように
		m_dropScale = scale;
	}
	else 
	{
		m_dropTimer = 0.0f;
		m_rotateZ = 0.0f;
		m_dropScale = 1.0f;
	}

}

/**
* @brief 武器モデルの描画処理
* @param context : デバイスコンテキスト
* @param states : コモンステート
* @param view : ビュー行列
* @param proj : プロジェクション行列
* @param debugFont : デバッグフォント描画用クラス
*/
void Lyse::Sword::Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, SimpleMath::Matrix view, SimpleMath::Matrix proj, DebugFont* debugFont)
{
	UNREFERENCED_PARAMETER(debugFont); // 現状、デバッグフォントはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	// 行列定義
	DirectX::SimpleMath::Matrix matScale;
	DirectX::SimpleMath::Matrix matWorldTrans; // ワールド座標への移動
	DirectX::SimpleMath::Matrix matLocalTrans; // ローカル座標でのオフセット
	DirectX::SimpleMath::Matrix matRotX;        // X軸回転（スイング用）
	DirectX::SimpleMath::Matrix matRotZ;        // Z軸回転（ガードブレイク用）

	// 剣の初期位置（ローカルオフセット）の設定
	matLocalTrans = DirectX::SimpleMath::Matrix::CreateTranslation(
		DirectX::SimpleMath::Vector3(RENDER_OFFSET_X, RENDER_OFFSET_Y, RENDER_OFFSET_Z)
	);

	// ワールド座標への移動（敵の現在位置）
	matWorldTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_charaPos + m_thrustOffset);

	// スイングアニメーションの回転適用
	matRotX = DirectX::SimpleMath::Matrix::CreateRotationX(m_rotateX);

	matRotZ = DirectX::SimpleMath::Matrix::CreateRotationZ(m_rotateZ);

	// スケール設定
	matScale = DirectX::SimpleMath::Matrix::CreateScale((RENDER_SCALE, RENDER_SCALE, RENDER_SCALE) * m_dropScale);

	// ワールド行列の合成:
	// ローカル移動 -> スケール -> X回転(スイング) -> 敵の身体の回転 -> ワールド移動
	DirectX::SimpleMath::Matrix world = matLocalTrans * matScale * matRotX * matRotZ * m_charaRot * matWorldTrans;

	// モデル描画
	m_sword->Draw(context, *states, world, view, proj);
}

/**
* @brief 終了処理
*/
void Lyse::Sword::Finalize()
{
}

/**
* @brief 武器の座標設定
* @param pos : 設定する座標
*/
void Lyse::Sword::SetPosition(DirectX::SimpleMath::Vector3 pos)
{
	m_position = pos;
}

/**
* @brief 武器のスケール設定（Vector3）
* @param scale : 設定するスケール値
*/
void Lyse::Sword::SetScale(DirectX::SimpleMath::Vector3 scale)
{
	m_scale = scale;
}

/**
* @brief 武器のスケール設定（均等）
* @param scale : 設定する倍率
*/
void Lyse::Sword::SetScale(float scale)
{
	m_scale.x = scale;
	m_scale.y = scale;
	m_scale.z = scale;
}

/**
* @brief 武器の回転設定
* @param rot : 各軸の回転角
*/
void Lyse::Sword::SetRotation(DirectX::SimpleMath::Vector3 rot)
{
	m_rotation = rot;
}


