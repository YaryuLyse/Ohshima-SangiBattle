/**
* @file ShaderManager.cpp
* @brief シェーダーおよびレンダリングパイプラインを統括管理するクラスの実装ファイル
* @brief フィールドやスカイドーム等、各オブジェクト専用シェーダーの適用処理をまとめています
* @brief このクラスは、頂点/ピクセルシェーダーの生成、入力レイアウトの定義、定数バッファの転送、およびDirectXTKのモデル描画と連携した描画ステートの切り替えを管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "ShaderManager.h"

#include "Lyselib/BinaryFile.h"
#include "Game/Common/DeviceResources.h"
#include <DDSTextureLoader.h>

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// シェーダーファイルパス
	const wchar_t* PATH_VS_FIELD = L"Resources/Shaders/FieldModelVS.cso";
	const wchar_t* PATH_PS_FIELD = L"Resources/Shaders/FieldModelPS.cso";
	const wchar_t* PATH_VS_SKYDOME = L"Resources/Shaders/SkydomeModelVS.cso";
	const wchar_t* PATH_PS_SKYDOME = L"Resources/Shaders/SkydomeModelPS.cso";

	// テクスチャファイルパス
	const wchar_t* PATH_TEX_SKY = L"Resources/Texture/sky.jpeg";

	// シェーダーリソース/バッファ設定
	constexpr int SLOT_CBUFFER = 1; // DirectXTKが0を使うため1を使用
	constexpr int SLOT_TEXTURE_SKY = 1;
}

/// <summary>
/// インプットレイアウト定義
/// VSINPUT構造体と一致させる必要がある
/// </summary>
const std::vector<D3D11_INPUT_ELEMENT_DESC> Lyse::ShaderManager::INPUT_LAYOUT_DESC =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, sizeof(SimpleMath::Vector3),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, sizeof(SimpleMath::Vector3) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

/**
* @brief コンストラクタ
*/
Lyse::ShaderManager::ShaderManager()
	: m_deviceResources(nullptr)
{
}

/**
* @brief デストラクタ
*/
Lyse::ShaderManager::~ShaderManager()
{
}

/**
* @brief 初期化処理。シェーダー、ステート、テクスチャの生成を統括する
* @param deviceResources : デバイスリソースへのポインタ
*/
void Lyse::ShaderManager::Initialize(DX::DeviceResources* deviceResources)
{
	m_deviceResources = deviceResources;
	ID3D11Device1* device = m_deviceResources->GetD3DDevice();

	// シェーダーの作成
	CreateShaders();

	// プリミティブバッチの作成
	m_primitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColorTexture>>(m_deviceResources->GetD3DDeviceContext());

	// ステートの作成
	m_commonStates = std::make_unique<CommonStates>(device);

	// エフェクトファクトリ（モデル読み込み用）
	std::unique_ptr<EffectFactory> fxFactory = std::make_unique<EffectFactory>(device);
	fxFactory->SetDirectory(L"Resources/Models");

	// テクスチャ作成
	CreateTextures();
}

/**
* @brief 指定パスからのテクスチャリソース読み込み
* @param path : 読み込むファイルの相対パス
*/
void Lyse::ShaderManager::LoadTexture(const wchar_t* path)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	DirectX::CreateWICTextureFromFile(
		m_deviceResources->GetD3DDevice(),
		path,
		nullptr,
		texture.ReleaseAndGetAddressOf()
	);

	m_textureList.push_back(texture);
}

/**
* @brief 各種シェーダー、入力レイアウト、および定数バッファの生成（内部処理）
*/
void Lyse::ShaderManager::CreateShaders()
{
	ID3D11Device1* device = m_deviceResources->GetD3DDevice();

	// --- ステージ（フィールド）用シェーダー ---

	// バイナリ読み込み
	BinaryFile stageVSData = BinaryFile::LoadFile(PATH_VS_FIELD);
	BinaryFile stagePSData = BinaryFile::LoadFile(PATH_PS_FIELD);

	// 入力レイアウト作成
	device->CreateInputLayout(
		&INPUT_LAYOUT_DESC[0],
		static_cast<UINT>(INPUT_LAYOUT_DESC.size()),
		stageVSData.GetData(),
		stageVSData.GetSize(),
		m_stageInputLayout.GetAddressOf()
	);

	// 頂点シェーダー作成
	if (FAILED(device->CreateVertexShader(stageVSData.GetData(), stageVSData.GetSize(), nullptr, m_stageVertexShader.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreateVertexShader Failed (Stage).", nullptr, MB_OK);
		return;
	}

	// ピクセルシェーダー作成
	if (FAILED(device->CreatePixelShader(stagePSData.GetData(), stagePSData.GetSize(), nullptr, m_stagePixelShader.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreatePixelShader Failed (Stage).", nullptr, MB_OK);
		return;
	}


	// --- スカイドーム用シェーダー ---

	// バイナリ読み込み
	BinaryFile skydomeVSData = BinaryFile::LoadFile(PATH_VS_SKYDOME);
	BinaryFile skydomePSData = BinaryFile::LoadFile(PATH_PS_SKYDOME);

	// 入力レイアウト作成
	device->CreateInputLayout(
		&INPUT_LAYOUT_DESC[0],
		static_cast<UINT>(INPUT_LAYOUT_DESC.size()),
		skydomeVSData.GetData(),
		skydomeVSData.GetSize(),
		m_skydomeInputLayout.GetAddressOf()
	);

	// 頂点シェーダー作成
	if (FAILED(device->CreateVertexShader(skydomeVSData.GetData(), skydomeVSData.GetSize(), nullptr, m_skydomeVertexShader.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreateVertexShader Failed (Skydome).", nullptr, MB_OK);
		return;
	}

	// ピクセルシェーダー作成
	if (FAILED(device->CreatePixelShader(skydomePSData.GetData(), skydomePSData.GetSize(), nullptr, m_skydomePixelShader.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreatePixelShader Failed (Skydome).", nullptr, MB_OK);
		return;
	}


	// --- 定数バッファ作成 ---
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ModelConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&bd, nullptr, &m_constantBuffer);
}

/**
* @brief スカイドーム用など、特定のテクスチャリソースの生成（内部処理）
*/
void Lyse::ShaderManager::CreateTextures()
{
	ID3D11Device* device = m_deviceResources->GetD3DDevice();

	// スカイドーム用テクスチャ
	CreateWICTextureFromFile(device, PATH_TEX_SKY, nullptr, &m_skydomeTexture);
}

/**
* @brief シェーダーリソースビュー（SRV）の手動設定
* @param srv : 設定するSRVのスマートポインタ
*/
void Lyse::ShaderManager::SetShaderResourceView(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	m_shaderResourceView = srv;
}

/**
* @brief 描画メイン処理
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManager::Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(proj);
}

/**
* @brief フィールド用シェーダーを適用してモデルを描画する。フラッシュ強度等のパラメータ更新を含む
* @param view : ビュー行列
* @param world : ワールド行列
* @param proj : プロジェクション行列
* @param model : 描画対象のモデル
* @param flashIntensity : 発光強度（ダメージ演出用等）
*/
void Lyse::ShaderManager::LoadStageShader(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj, DirectX::DX11::Model* model, float flashIntensity)
{
	ID3D11DeviceContext1* context = m_deviceResources->GetD3DDeviceContext();

	// 定数バッファの更新
	D3D11_MAPPED_SUBRESOURCE mapped;
	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	ModelConstantBuffer* cb = (ModelConstantBuffer*)mapped.pData;
	cb->matWorld = XMMatrixTranspose(world);
	cb->matView = XMMatrixTranspose(view);
	cb->matProj = XMMatrixTranspose(proj);
	cb->flashColor = DirectX::SimpleMath::Vector4(flashIntensity, flashIntensity, flashIntensity, 0.0f);

	context->Unmap(m_constantBuffer.Get(), 0);

	// 定数バッファの設定
	ID3D11Buffer* cbuf_ps[] = { m_constantBuffer.Get() };
	context->PSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);
	context->VSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);

	// モデル描画
	model->Draw(context, *m_commonStates, world, view, proj, false, [&]() 
		{
		// サンプラ設定
		ID3D11SamplerState* samplers[] = 
		{
			m_commonStates->PointWrap(),
			m_commonStates->LinearWrap()
		};
		context->PSSetSamplers(1, 2, samplers);

		// シェーダー・IL設定
		context->PSSetShader(m_stagePixelShader.Get(), nullptr, 0);
		context->VSSetShader(m_stageVertexShader.Get(), nullptr, 0);
		context->IASetInputLayout(m_stageInputLayout.Get());
		});
}

/**
* @brief スカイドーム用シェーダーを適用してモデルを描画する
* @param view : ビュー行列
* @param world : ワールド行列
* @param proj : プロジェクション行列
* @param model : 描画対象のモデル
*/
void Lyse::ShaderManager::LoadSkydomeShader(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj, DirectX::DX11::Model* model)
{
	ID3D11DeviceContext1* context = m_deviceResources->GetD3DDeviceContext();

	// 定数バッファの更新
	D3D11_MAPPED_SUBRESOURCE mapped;
	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	ModelConstantBuffer* cb = (ModelConstantBuffer*)mapped.pData;
	cb->matWorld = XMMatrixTranspose(world);
	cb->matView = XMMatrixTranspose(view);
	cb->matProj = XMMatrixTranspose(proj);

	context->Unmap(m_constantBuffer.Get(), 0);

	// 定数バッファの設定
	ID3D11Buffer* cbuf_ps[] = { m_constantBuffer.Get() };
	context->PSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);
	context->VSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);

	// モデル描画
	model->Draw(context, *m_commonStates, world, view, proj, false, [&]() 
		{
		// サンプラ設定
		ID3D11SamplerState* samplers[] = 
		{
			m_commonStates->AnisotropicWrap(),
			m_commonStates->AnisotropicWrap()
		};
		context->PSSetSamplers(1, 2, samplers);

		// テクスチャ設定
		context->PSSetShaderResources(SLOT_TEXTURE_SKY, 1, m_skydomeTexture.GetAddressOf());

		// シェーダー・IL設定
		context->PSSetShader(m_skydomePixelShader.Get(), nullptr, 0);
		context->VSSetShader(m_skydomeVertexShader.Get(), nullptr, 0);
		context->IASetInputLayout(m_skydomeInputLayout.Get());
		});
}

/**
* @brief 個別処理用のモデル描画関数（将来的な拡張用）
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManager::RenderModel(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(proj);
}