/**
* @file Stage.cpp
* @brief ステージ（フィールド）の構成要素やギミックを管理するクラスの実装ファイル
* @brief 壁のセグメント管理や破壊時の演出処理をまとめています
* @brief このクラスは、複数の壁モデルのロード、破壊フラグに応じたモデルの切り替え、および破壊時の発光（フラッシュ）タイマーや衝突判定の遅延発生を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "Stage.h"

#include <WICTextureLoader.h>
#include "Game/State/IdleState.h"
#include "Game/State/AttackState.h"
#include "Game/GameObjects/Shader/ShaderManager.h"

using namespace DirectX;

namespace
{
	// ファイルパスフォーマット
	const wchar_t* PATH_FORMAT_WALL_NORMAL = L"Resources/Models/walls%d.sdkmesh";
	const wchar_t* PATH_FORMAT_WALL_BROKEN = L"Resources/Models/wallBreak%d.sdkmesh";

	// 設定値
	const int    ALPHA_REF_VALUE = 255;

	// 壁破壊時の設定
	const float TIMER_FLASH_DURATION = 0.5f;     // 発光時間
	const float TIMER_COLLISION_DELAY = 0.5f;    // 衝突判定発生までの遅延

	// スケール設定
	const float SCALE_Y_BROKEN = 0.5f;           // 壊れた壁のYスケール
	const float SCALE_NORMAL = 1.0f;             // 通常時のスケール
	const float SCALE_WALL_DIS = 1.2f;           // 壊れた壁の間隔
}

/**
* @brief コンストラクタ。各壁セグメント（通常・破壊後）のモデル読み込みと初期化を行う
* @param device : Direct3Dデバイス
* @param fx : エフェクトファクトリ
*/
Lyse::Stage::Stage(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx)
	: m_shaderManager(nullptr)
	, m_attackVec(0.0f, 0.0f, 1.0f)
{
	// エフェクトの作成
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

	// テクスチャ設定（現在はnullptrが入る可能性あり）
	m_batchEffect->SetTexture(m_texture.Get());

	// 壁モデルの読み込み
	m_walls.resize(WALL_COUNT);
	for (int i = 0; i < WALL_COUNT; i++)
	{
		wchar_t normalPath[64];
		wchar_t brokenPath[64];

		// 1始まりのインデックスでファイル名を生成
		swprintf_s(normalPath, PATH_FORMAT_WALL_NORMAL, i + 1);
		swprintf_s(brokenPath, PATH_FORMAT_WALL_BROKEN, i + 1);

		// 壁セグメントの作成
		m_walls[i].m_normalModel = DirectX::Model::CreateFromSDKMESH(device, normalPath, fx);

		// 壊れた壁セグメントの作成
		m_walls[i].m_brokenModel = DirectX::Model::CreateFromSDKMESH(device, brokenPath, fx);

		// 初期化
		m_walls[i].m_isBroken = false;
		m_walls[i].m_flashTimer = 0.0f;
		m_walls[i].m_collisionSpawnTimer = 0.0f;
		m_walls[i].m_isColliding = false;
	}
}

/**
* @brief デストラクタ
*/
Lyse::Stage::~Stage()
{
}

/**
* @brief 初期化処理。ステージの基準座標やシェーダーマネージャーの参照を設定する
* @param shaderManager : シェーダーマネージャーへのポインタ
*/
void Lyse::Stage::Initialize(ShaderManager* shaderManager)
{
	m_position = DirectX::SimpleMath::Vector3(0.0f, 2.0f, 0.0f);
	m_prePosition = DirectX::SimpleMath::Vector3::Zero;
	m_scale = DirectX::SimpleMath::Vector3::One;
	m_rotation = DirectX::SimpleMath::Vector3::Zero;

	m_shaderManager = shaderManager;
}

/**
* @brief 更新処理。壁の破壊演出用タイマーや、衝突判定の発生待機時間を更新する
* @param elapsedTime : 経過時間
*/
void Lyse::Stage::Update(float elapsedTime)
{
	auto kb = DirectX::Keyboard::Get().GetState();

	// 壁ごとの更新処理
	for (int i = 0; i < m_walls.size(); i++)
	{
		// フラッシュタイマーの更新
		if (m_walls[i].m_flashTimer > 0.0f)
		{
			m_walls[i].m_flashTimer -= elapsedTime;
			if (m_walls[i].m_flashTimer < 0.0f)
			{
				m_walls[i].m_flashTimer = 0.0f;
			}
		}

		// 衝突判定発生タイマーの更新
		if (m_walls[i].m_isColliding)
		{
			m_walls[i].m_collisionSpawnTimer -= elapsedTime;

			if (m_walls[i].m_collisionSpawnTimer <= 0.0f)
			{
				// 座標リストの範囲内であれば衝突オブジェクトを有効化
				if (i < m_wallPositions.size())
				{
					m_stageBreakCollisions[i].Activate(DirectX::SimpleMath::Vector3(m_wallPositions[i].x * SCALE_WALL_DIS,m_wallPositions[i].y,m_wallPositions[i].z * SCALE_WALL_DIS));
				}

				m_walls[i].m_isColliding = false; // 待機状態解除
			}
		}
	}
}

/**
* @brief 描画処理。壁の状態（通常/破壊）に応じてモデルとスケールを選択し、描画を実行する
* @param context : デバイスコンテキスト
* @param states : 共通ステート
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::Stage::Render(ID3D11DeviceContext1* context, DirectX::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	UNREFERENCED_PARAMETER(states);
	UNREFERENCED_PARAMETER(context);
	DirectX::SimpleMath::Matrix scaleMat;
	DirectX::SimpleMath::Matrix transMat;
	DirectX::SimpleMath::Matrix rotMat;

	transMat = DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(0.0f);
	scaleMat = DirectX::SimpleMath::Matrix::CreateScale(SCALE_NORMAL);

	DirectX::SimpleMath::Matrix baseWorld = transMat * scaleMat;

	// 各壁セグメントの描画
	for (const auto& wall : m_walls)
	{
		DirectX::Model* targetModel = nullptr;
		DirectX::SimpleMath::Matrix world = baseWorld;

		if (wall.m_isBroken)
		{
			// 壊れている場合
			targetModel = wall.m_brokenModel.get();
			scaleMat = DirectX::SimpleMath::Matrix::CreateScale(1.0f, SCALE_Y_BROKEN, 1.0f);
			world = transMat * scaleMat;
		}
		else
		{
			// 通常の場合
			targetModel = wall.m_normalModel.get();
			scaleMat = DirectX::SimpleMath::Matrix::CreateScale(SCALE_NORMAL);
			world = transMat * scaleMat;
		}

		// ShaderManagerを使って描画
		if (targetModel && m_shaderManager)
		{
			m_shaderManager->LoadStageShader(view, world, proj, targetModel, wall.m_flashTimer);
		}
	}
}

/**
* @brief 終了処理
*/
void Lyse::Stage::Finalize()
{
}

/**
* @brief 指定されたインデックスの壁を破壊状態に移行させ、演出タイマーを始動する
* @param index : 壁のインデックス
*/
void Lyse::Stage::BreakWall(int index)
{
	// 範囲チェック
	if (index >= 0 && index < m_walls.size())
	{
		// まだ壊れていなければ破壊処理を実行
		if (!m_walls[index].m_isBroken)
		{
			m_walls[index].m_isBroken = true;

			// エフェクト用タイマー設定
			m_walls[index].m_flashTimer = TIMER_FLASH_DURATION;

			// 衝突判定発生用タイマー設定
			m_walls[index].m_collisionSpawnTimer = TIMER_COLLISION_DELAY;
			m_walls[index].m_isColliding = true;
		}
	}
}