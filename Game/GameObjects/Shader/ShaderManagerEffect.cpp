/**
* @file ShaderManagerEffect.cpp
* @brief エフェクト専用のシェーダーおよびレンダリングパイプラインを管理するクラスの実装ファイル
* @brief シールド等の特殊な描画効果を伴うオブジェクトの処理をまとめています
* @brief このクラスは、加算合成や深度バッファ読み取り専用設定を用いた透過エフェクトの描画、および時間経過によるアニメーション用パラメータの更新を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "ShaderManagerEffect.h"

#include "Lyselib/BinaryFile.h"
#include "Game/Common/DeviceResources.h"
#include <DDSTextureLoader.h>

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// シェーダーファイルパス
	const wchar_t* PATH_VS_SHIELD_EFFECT = L"Resources/Shaders/ShieldEffectVS.cso";
	const wchar_t* PATH_PS_SHIELD_EFFECT = L"Resources/Shaders/ShieldEffectPS.cso";

	// モデルファイルパス
	const wchar_t* PATH_MODEL_SHIELD = L"Resources/Models/shieldEffect.sdkmesh";

	// テクスチャファイルパス
	const wchar_t* PATH_TEX_SKY = L"Resources/Texture/sky.jpeg";

	// シールドの色設定 (RGBA)
	const DirectX::SimpleMath::Vector4 COLOR_SHIELD_PLAYER(0.0f, 0.8f, 1.0f, 1.0f); // プレイヤー用（青系）
	const DirectX::SimpleMath::Vector4 COLOR_SHIELD_ENEMY(1.0f, 0.0f, 0.0f, 1.0f);  // 敵用（赤系）

	// 定数バッファスロット
	constexpr int SLOT_CBUFFER = 1; // DirectXTKが0を使用するため1

	// サンプラースロット
	constexpr int SLOT_SAMPLER_START = 1;
	constexpr int NUM_SAMPLERS = 2;
}

/// <summary>
/// インプットレイアウト定義
/// </summary>
const std::vector<D3D11_INPUT_ELEMENT_DESC> Lyse::ShaderManagerEffect::INPUT_LAYOUT_DESC =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, sizeof(SimpleMath::Vector3),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
	// TEXCOORDが必要な場合はここに追加
};

/**
* @brief コンストラクタ。累積時間の初期化等を行う
*/
Lyse::ShaderManagerEffect::ShaderManagerEffect()
	: m_deviceResources(nullptr)
	, m_time(0.0f)
{
}

/**
* @brief デストラクタ
*/
Lyse::ShaderManagerEffect::~ShaderManagerEffect()
{
}

/**
* @brief 初期化処理。エフェクト用シェーダーの生成と、シールドモデルの読み込みを行う
* @param deviceResources : デバイスリソースへのポインタ
*/
void Lyse::ShaderManagerEffect::Initialize(DX::DeviceResources* deviceResources)
{
	m_deviceResources = deviceResources;
	ID3D11Device1* device = m_deviceResources->GetD3DDevice();

	// シェーダー作成
	CreateShaders();

	// プリミティブバッチ作成
	m_primitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColorTexture>>(m_deviceResources->GetD3DDeviceContext());

	// ステート作成
	m_commonStates = std::make_unique<CommonStates>(device);

	// モデル読み込み（シールドエフェクト用）
	std::unique_ptr<EffectFactory> fxFactory = std::make_unique<EffectFactory>(device);
	fxFactory->SetDirectory(L"Resources/Models"); // ディレクトリ設定は適宜調整

	m_model = DirectX::Model::CreateFromSDKMESH(
		device,
		PATH_MODEL_SHIELD,
		*fxFactory
	);

	// テクスチャ作成
	CreateTextures();
}

/**
* @brief 更新処理。シェーダーに渡すための累積時間を更新する
* @param elapsedTime : 経過時間
*/
void Lyse::ShaderManagerEffect::Update(float elapsedTime)
{
	m_time += elapsedTime;
}

/**
* @brief 外部から指定されたパスのテクスチャを読み込み、リストへ追加する
* @param path : テクスチャファイルの相対パス
*/
void Lyse::ShaderManagerEffect::LoadTexture(const wchar_t* path)
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
* @brief エフェクト用シェーダー、入力レイアウト、定数バッファの生成（内部処理）
*/
void Lyse::ShaderManagerEffect::CreateShaders()
{
	ID3D11Device1* device = m_deviceResources->GetD3DDevice();

	// バイナリファイルの読み込み
	BinaryFile shieldVSData = BinaryFile::LoadFile(PATH_VS_SHIELD_EFFECT);
	BinaryFile shieldPSData = BinaryFile::LoadFile(PATH_PS_SHIELD_EFFECT);

	// インプットレイアウトの作成
	device->CreateInputLayout(
		&INPUT_LAYOUT_DESC[0],
		static_cast<UINT>(INPUT_LAYOUT_DESC.size()),
		shieldVSData.GetData(),
		shieldVSData.GetSize(),
		m_shieldInputLayout.GetAddressOf()
	);

	// 頂点シェーダー作成
	if (FAILED(device->CreateVertexShader(shieldVSData.GetData(), shieldVSData.GetSize(), nullptr, m_shieldVertexShader.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreateVertexShader Failed (Shield).", nullptr, MB_OK);
		return;
	}

	// ピクセルシェーダー作成
	if (FAILED(device->CreatePixelShader(shieldPSData.GetData(), shieldPSData.GetSize(), nullptr, m_shieldPixelShader.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreatePixelShader Failed (Shield).", nullptr, MB_OK);
		return;
	}

	// 定数バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(EffectConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&bd, nullptr, &m_constantBuffer);
}

/**
* @brief エフェクトで使用する基本テクスチャの生成（内部処理）
*/
void Lyse::ShaderManagerEffect::CreateTextures()
{
	ID3D11Device* device = m_deviceResources->GetD3DDevice();

	// スカイドーム用テクスチャ読み込み
	CreateWICTextureFromFile(device, PATH_TEX_SKY, nullptr, &m_skydomeTexture);
}

/**
* @brief シェーダーリソースビュー（SRV）の手動設定
* @param srv : 設定するSRVのスマートポインタ
*/
void Lyse::ShaderManagerEffect::SetShaderResourceView(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	m_shaderResourceView = srv;
}

/**
* @brief エフェクトの描画実行（基本処理）
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerEffect::Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	// 必要に応じて実装
	m_world = DirectX::SimpleMath::Matrix::Identity;
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(proj);
}

/**
* @brief プレイヤー用シールド描画。青系の加算合成シェーダーを適用する
* @param view : ビュー行列
* @param world : ワールド行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerEffect::LoadShieldShaderPlayer(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj)
{
	ID3D11DeviceContext1* context = m_deviceResources->GetD3DDeviceContext();

	// カメラ位置の算出（ビュー行列の逆行列から取得）
	DirectX::SimpleMath::Matrix invView = view.Invert();
	DirectX::SimpleMath::Vector3 cameraPos = invView.Translation();

	// 定数バッファの更新
	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		EffectConstantBuffer* cb = (EffectConstantBuffer*)mapped.pData;
		cb->matWorld = XMMatrixTranspose(world);
		cb->matView = XMMatrixTranspose(view);
		cb->matProj = XMMatrixTranspose(proj);
		cb->color = COLOR_SHIELD_PLAYER; // プレイヤー色
		cb->cameraDir = DirectX::SimpleMath::Vector4(cameraPos.x, cameraPos.y, cameraPos.z, 0.0f);
		cb->time = DirectX::SimpleMath::Vector4(m_time, 0.0f, 0.0f, 0.0f);

		context->Unmap(m_constantBuffer.Get(), 0);
	}

	// 定数バッファの設定配列
	ID3D11Buffer* cbuf_ps[] = { m_constantBuffer.Get() };

	// モデル描画
	m_model->Draw(context, *m_commonStates, world, view, proj, false, [&]() 
		{

		// ブレンドステート：加算合成
		context->OMSetBlendState(m_commonStates->Additive(), nullptr, 0xffffffff);

		// 深度ステート：書き込み無効（DepthRead）
		context->OMSetDepthStencilState(m_commonStates->DepthRead(), 0);

		// サンプラー設定
		ID3D11SamplerState* samplers[] =
		{
			m_commonStates->PointWrap(),
			m_commonStates->LinearWrap()
		};
		context->PSSetSamplers(SLOT_SAMPLER_START, NUM_SAMPLERS, samplers);

		// シェーダーと入力レイアウトの設定
		context->PSSetShader(m_shieldPixelShader.Get(), nullptr, 0);
		context->VSSetShader(m_shieldVertexShader.Get(), nullptr, 0);
		context->IASetInputLayout(m_shieldInputLayout.Get());

		// 定数バッファの設定
		context->PSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);
		context->VSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);
		});
}

/**
* @brief 敵用シールド描画。赤系の加算合成シェーダーを適用する
* @param view : ビュー行列
* @param world : ワールド行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerEffect::LoadShieldShaderEnemy(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj)
{
	ID3D11DeviceContext1* context = m_deviceResources->GetD3DDeviceContext();

	// カメラ位置の算出
	DirectX::SimpleMath::Matrix invView = view.Invert();
	DirectX::SimpleMath::Vector3 cameraPos = invView.Translation();

	// 定数バッファの更新
	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		EffectConstantBuffer* cb = (EffectConstantBuffer*)mapped.pData;
		cb->matWorld = XMMatrixTranspose(world);
		cb->matView = XMMatrixTranspose(view);
		cb->matProj = XMMatrixTranspose(proj);
		cb->color = COLOR_SHIELD_ENEMY; // 敵色
		cb->cameraDir = DirectX::SimpleMath::Vector4(cameraPos.x, cameraPos.y, cameraPos.z, 0.0f);
		cb->time = DirectX::SimpleMath::Vector4(m_time, 0.0f, 0.0f, 0.0f);

		context->Unmap(m_constantBuffer.Get(), 0);
	}

	// 定数バッファの設定配列
	ID3D11Buffer* cbuf_ps[] = { m_constantBuffer.Get() };

	// モデル描画
	m_model->Draw(context, *m_commonStates, world, view, proj, false, [&]() 
		{

		// ブレンドステート：加算合成
		context->OMSetBlendState(m_commonStates->Additive(), nullptr, 0xffffffff);

		// 深度ステート：書き込み無効
		context->OMSetDepthStencilState(m_commonStates->DepthRead(), 0);

		// サンプラー設定
		ID3D11SamplerState* samplers[] = 
		{
			m_commonStates->PointWrap(),
			m_commonStates->LinearWrap()
		};
		context->PSSetSamplers(SLOT_SAMPLER_START, NUM_SAMPLERS, samplers);

		// シェーダーと入力レイアウトの設定
		context->PSSetShader(m_shieldPixelShader.Get(), nullptr, 0);
		context->VSSetShader(m_shieldVertexShader.Get(), nullptr, 0);
		context->IASetInputLayout(m_shieldInputLayout.Get());

		// 定数バッファの設定
		context->PSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);
		context->VSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf_ps);
		});
}

/**
* @brief スカイドーム用シェーダー適用と描画（将来的な拡張用）
* @param view : ビュー行列
* @param world : ワールド行列
* @param proj : プロジェクション行列
* @param model : 描画対象のモデル
*/
void Lyse::ShaderManagerEffect::LoadSkydomeShader(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj, DirectX::DX11::Model* model)
{
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(world);
	UNREFERENCED_PARAMETER(proj);
	UNREFERENCED_PARAMETER(model);
}

/**
* @brief 個別処理用のモデル描画関数（将来的な拡張用）
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerEffect::RenderModel(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(proj);
}