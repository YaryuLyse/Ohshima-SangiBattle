/**
* @file Shield.cpp
* @brief 敵の盾を管理するクラスの実装ファイル
* @brief 敵が装備する盾のモデル描画やエフェクト制御をまとめています
* @brief このクラスは、敵の状態（ガード中か否か）に応じた盾の回転制御や、ガード時のシールドエフェクトの表示管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "Shield.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/State/GuardState.h"
#include "Game/State/DeathState.h"
#include "Game/GameObjects/Shader/ShaderManagerEffect.h"
#include "LyseLib/DebugFont.h"

// DirectXTK
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>

using namespace DirectX;

namespace
{
	// --- 定数定義 ---

	// エフェクト設定
	const int ALPHA_REF_VALUE = 255;

	// モデルファイルパス
	const wchar_t* MODEL_FILENAME = L"Resources/Models/Shield.sdkmesh";

	// 初期配置パラメータ
	const float INIT_POS_X = 0.0f;
	const float INIT_POS_Y = 0.5f;
	const float INIT_POS_Z = 5.0f;

	// 描画パラメータ：モデル用
	const float MODEL_OFFSET_X = 0.0f;
	const float MODEL_OFFSET_Y = -30.0f;
	const float MODEL_OFFSET_Z = 70.0f;
	const float MODEL_SCALE_FACTOR = 0.01f;
	const float MODEL_ROT_X_BASE = 270.0f; // 基本姿勢のX回転

	// 描画パラメータ：エフェクト用
	const float EFFECT_OFFSET_Y = 0.5f;
	const float EFFECT_SCALE_FACTOR = 2.5f;

	// 状態ごとのZ回転角度
	const float ROT_Z_GUARD = 0.0f;  // ガード中
	const float ROT_Z_IDLE = 90.0f;  // 通常時
}

namespace Lyse
{
	// 静的メンバ変数の定義（必要であれば）
	const DirectX::VertexPositionTexture Shield::VERTICES[VERTEX_COUNT] = {};

	/**
	* @brief コンストラクタ。デバイスの初期化やモデルのロードを行う
	* @param device : DirectXデバイス
	* @param fx : エフェクトファクトリ
	*/
	Shield::Shield(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx)
		: m_rotate(0.0f)
		, m_character(nullptr)
	{
		// アルファテストエフェクトの作成
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
		m_shield = DirectX::Model::CreateFromSDKMESH(
			device,
			MODEL_FILENAME,
			fx
		);

		// テクスチャ設定（ロード済みの場合）
		m_batchEffect->SetTexture(m_texture.Get());
	}

	/**
	* @brief デストラクタ
	*/
	Shield::~Shield()
	{
	}

	/**
	* @brief 盾の初期化処理
	*/
	void Shield::Initialize()
	{
		m_position = DirectX::SimpleMath::Vector3(INIT_POS_X, INIT_POS_Y, INIT_POS_Z);
		m_prePosition = DirectX::SimpleMath::Vector3::Zero;
		m_scale = DirectX::SimpleMath::Vector3::One;
		m_rotation = DirectX::SimpleMath::Vector3::Zero;
	}

	/**
	* @brief 盾の更新処理（親モデルとの同期）
	* @param elapsedTime : 経過時間
	* @param position : 親モデルの座標
	* @param rot : 親モデルの回転行列
	*/
	void Shield::Update(float elapsedTime, DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix rot)
	{
		UNREFERENCED_PARAMETER(elapsedTime); // 現状、経過時間はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
		// キーボード状態の更新
		auto kb = DirectX::Keyboard::Get().GetState();
		m_keyboardTracker.Update(kb);

		// 親（敵）の位置と回転を同期
		m_enemyPos = position;
		m_enemyRot = rot;
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
	* @brief 盾およびガードエフェクトの描画処理
	* @param context : デバイスコンテキスト
	* @param states : コモンステート
	* @param view : ビュー行列
	* @param proj : プロジェクション行列
	* @param debugFont : デバッグフォント描画用クラス
	*/
	void Shield::Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, DebugFont* debugFont)
	{
		UNREFERENCED_PARAMETER(debugFont); // 現状、デバッグフォントはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
		// 敵モデルがセットされていない場合は描画しない
		if (!m_character) return;

		// 行列定義
		DirectX::SimpleMath::Matrix matScale;
		DirectX::SimpleMath::Matrix matWorldTrans; // ワールド位置への移動
		DirectX::SimpleMath::Matrix matLocalTrans; // ローカルオフセット
		DirectX::SimpleMath::Matrix matRotX;
		DirectX::SimpleMath::Matrix matRotZ;
		DirectX::SimpleMath::Matrix world;

		// --- 1. モデルの描画 ---

		// ローカル位置調整
		matLocalTrans = DirectX::SimpleMath::Matrix::CreateTranslation(
			DirectX::SimpleMath::Vector3(MODEL_OFFSET_X, MODEL_OFFSET_Y, MODEL_OFFSET_Z)
		);

		// ワールド位置移動（敵の座標）
		matWorldTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_enemyPos);

		// 状態に応じた回転設定
		if (m_character->GetCurrentState() == m_character->GetGuardState())
		{
			matRotZ = DirectX::SimpleMath::Matrix::CreateRotationZ(DirectX::XMConvertToRadians(ROT_Z_GUARD));
		}
		else
		{
			matRotZ = DirectX::SimpleMath::Matrix::CreateRotationZ(DirectX::XMConvertToRadians(ROT_Z_IDLE));
		}

		matRotX = DirectX::SimpleMath::Matrix::CreateRotationX(DirectX::XMConvertToRadians(MODEL_ROT_X_BASE));

		// モデル用スケール
		matScale = DirectX::SimpleMath::Matrix::CreateScale(MODEL_SCALE_FACTOR * m_dropScale);

		// ワールド行列合成
		world = matLocalTrans * matScale * matRotZ * matRotX * m_enemyRot * matWorldTrans;

		// モデル本体の描画
		m_shield->Draw(context, *states, world, view, proj);


		// --- 2. シールドエフェクトの描画 ---

		// エフェクト用スケール
		matScale = DirectX::SimpleMath::Matrix::CreateScale(EFFECT_SCALE_FACTOR);

		// エフェクト用位置調整
		matLocalTrans = DirectX::SimpleMath::Matrix::CreateTranslation(
			DirectX::SimpleMath::Vector3(0.0f, EFFECT_OFFSET_Y, 0.0f)
		);

		// エフェクト用ワールド行列再計算
		// ※RotXを含まず、RotZのみ適用するロジックを維持
		world = matLocalTrans * matScale * matRotZ * m_enemyRot * matWorldTrans;

		// ガード中のみエフェクトを描画
		if (m_character->GetCurrentState() == m_character->GetGuardState())
		{
			m_character->GetShaderManagerEffect()->LoadShieldShaderEnemy(view, world, proj);
		}
	}

	/**
	* @brief 終了処理
	*/
	void Shield::Finalize()
	{
	}

	/**
	* @brief 盾の座標設定
	* @param pos : 設定する座標
	*/
	void Shield::SetPosition(DirectX::SimpleMath::Vector3 pos)
	{
		m_position = pos;
	}

	/**
	* @brief 盾のスケール設定（Vector3）
	* @param scale : 設定するスケール
	*/
	void Shield::SetScale(DirectX::SimpleMath::Vector3 scale)
	{
		m_scale = scale;
	}

	/**
	* @brief 盾のスケール設定（均等）
	* @param scale : 設定する倍率
	*/
	void Shield::SetScale(float scale)
	{
		m_scale = DirectX::SimpleMath::Vector3(scale, scale, scale);
	}

	/**
	* @brief 盾の回転設定
	* @param rot : 各軸の回転角
	*/
	void Shield::SetRotation(DirectX::SimpleMath::Vector3 rot)
	{
		m_rotation = rot;
	}
}