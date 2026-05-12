/**
* @file Skydome.cpp
* @brief スカイドーム（背景天球）の挙動を管理するクラスの実装ファイル
* @brief 巨大な背景モデルの描画およびプレイヤーへの追従処理をまとめています
* @brief このクラスは、背景モデルのロード、アルファテストの設定、および描画時にプレイヤーの位置と同期させることで無限遠の背景を擬似的に表現する管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "Skydome.h"

#include <WICTextureLoader.h>
#include "Game/GameObjects/Shader/ShaderManager.h"

using namespace DirectX;

namespace
{
	// ファイルパス
	const wchar_t* PATH_MODEL_SKYDOME = L"Resources/Models/skydome.sdkmesh";

	// 設定値
	const int    ALPHA_REF_VALUE = 255;
	const float SKYDOME_SCALE_FACTOR = 20.0f;
	const DirectX::SimpleMath::Vector3 DEFAULT_ATTACK_VEC(0.0f, 0.0f, 1.0f);
}

/**
* @brief コンストラクタ。エフェクトの設定、入力レイアウトの生成、およびモデルのロードを行う
* @param device : Direct3Dデバイス
* @param fx : エフェクトファクトリ
*/
Lyse::Skydome::Skydome(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx)
	: m_shaderManager(nullptr)
	, m_attackVec(DEFAULT_ATTACK_VEC)
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

	// モデルのロード
	// TODO:02) CreateWICTextureFromFileの実装

	m_modelSkydome = DirectX::Model::CreateFromSDKMESH(
		device,
		PATH_MODEL_SKYDOME,
		fx
	);

	// TODO:03) テクスチャデータをBatchEffectに設定
	// 現状 m_texture は空の状態
	m_batchEffect->SetTexture(m_texture.Get());
}

/**
* @brief デストラクタ
*/
Lyse::Skydome::~Skydome()
{
}

/**
* @brief 初期化処理。座標、スケール、回転、およびマネージャーの参照を設定する
* @param shaderManager : シェーダーマネージャーへのポインタ
*/
void Lyse::Skydome::Initialize(ShaderManager* shaderManager)
{
	// パラメータ初期化
	m_prePosition = DirectX::SimpleMath::Vector3::Zero;
	m_scale = DirectX::SimpleMath::Vector3::One;
	m_rotation = DirectX::SimpleMath::Vector3::Zero;

	// マネージャー保持
	m_shaderManager = shaderManager;
}

/**
* @brief 更新処理。入力状態の取得などを行う
*/
void Lyse::Skydome::Update()
{
	auto kb = DirectX::Keyboard::Get().GetState();
}

/**
* @brief 描画処理。プレイヤー位置に合わせた行列計算を行い、シェーダーを適用してモデルを描画する
* @param context : デバイスコンテキスト
* @param states : 共通ステートへのポインタ
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::Skydome::Render(ID3D11DeviceContext1* context, DirectX::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(states);
	DirectX::SimpleMath::Matrix scaleMat;
	DirectX::SimpleMath::Matrix transMat;
	DirectX::SimpleMath::Matrix rotMat;

	// プレイヤー位置と同期させる（平行移動）
	transMat = DirectX::SimpleMath::Matrix::CreateTranslation(m_position);

	// 回転（現在は固定）
	rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(0.0f);

	// スケール設定
	scaleMat = DirectX::SimpleMath::Matrix::CreateScale(SKYDOME_SCALE_FACTOR);

	// ワールド行列合成
	DirectX::SimpleMath::Matrix world = scaleMat * transMat * rotMat;

	// シェーダーマネージャー経由で描画
	// m_modelSkydome->Draw(context, *states, world, view, proj); // 直接描画の場合はこちら
	if (m_shaderManager && m_modelSkydome)
	{
		m_shaderManager->LoadSkydomeShader(view, world, proj, m_modelSkydome.get());
	}
}

/**
* @brief 終了処理
*/
void Lyse::Skydome::Finalize()
{
}