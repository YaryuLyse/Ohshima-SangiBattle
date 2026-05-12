/**
* @file FloorModel.cpp
* @brief 地面（フロア）モデルを管理するクラスの実装ファイル
* @brief ステージの床となるモデルのロードおよび描画処理をまとめています
* @brief このクラスは、床モデルの描画、アルファテスト効果の設定、およびワールド空間における床の配置固定を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "FloorModel.h"

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// モデルファイル
	const wchar_t* MODEL_FILE_NAME = L"Resources/Models/floor.sdkmesh";

	// アルファテスト設定
	constexpr int ALPHA_REF = 255;

	// レンダリング設定
	const DirectX::SimpleMath::Vector3 RENDER_POS(0.0f, -1.0f, 0.0f); // 描画時の固定位置
	constexpr float RENDER_ROT_Y = 0.0f;
	constexpr float RENDER_SCALE = 1.0f;
}

/**
* @brief コンストラクタ。デバイスの初期化、モデルのロード、各種エフェクトの設定を行う
* @param device : DirectXデバイス
* @param fx : エフェクトファクトリ
*/
Lyse::FloorModel::FloorModel(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx)
{
	// エフェクトの作成
	m_batchEffect = std::make_unique<AlphaTestEffect>(device);
	m_batchEffect->SetAlphaFunction(D3D11_COMPARISON_EQUAL);
	m_batchEffect->SetReferenceAlpha(ALPHA_REF);

	// 入力レイアウト生成
	void const* shaderByteCode;
	size_t byteCodeLength;
	m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	device->CreateInputLayout(
		VertexPositionTexture::InputElements,
		VertexPositionTexture::InputElementCount,
		shaderByteCode, byteCodeLength, m_inputLayout.GetAddressOf()
	);

	// 共通ステートの作成
	m_states = std::make_unique<CommonStates>(device);

	// モデルのロード
	m_floorModel = DirectX::Model::CreateFromSDKMESH(
		device,
		MODEL_FILE_NAME,
		fx
	);

	// テクスチャ設定（現在はnullptrのまま処理）
	m_batchEffect->SetTexture(m_texture.Get());

	// 変数初期化
	m_position = DirectX::SimpleMath::Vector3::Zero;
	m_basePosition = DirectX::SimpleMath::Vector3::Zero;
	m_scale = DirectX::SimpleMath::Vector3::One;
	m_rotation = DirectX::SimpleMath::Vector3::Zero;
}

/**
* @brief デストラクタ
*/
Lyse::FloorModel::~FloorModel()
{
}

/**
* @brief 地面モデルの描画処理
* @param context : デバイスコンテキスト
* @param states : コモンステート
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::FloorModel::Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, SimpleMath::Matrix view, SimpleMath::Matrix proj)
{
	using namespace DirectX::SimpleMath;

	Matrix scaleMat;
	Matrix transMat;
	Matrix rotMat;

	// 描画位置・回転・スケールの設定
	// (※元コードのロジックに従い、メンバ変数を上書きして使用)
	m_position = RENDER_POS;

	transMat = Matrix::CreateTranslation(m_position);
	rotMat = Matrix::CreateRotationY(RENDER_ROT_Y);
	scaleMat = Matrix::CreateScale(RENDER_SCALE);

	// ワールド行列合成
	Matrix world = transMat * rotMat * scaleMat;

	m_floorModel->Draw(context, *states, world, view, proj);
}

/**
* @brief 終了処理
*/
void Lyse::FloorModel::Finalize()
{
}