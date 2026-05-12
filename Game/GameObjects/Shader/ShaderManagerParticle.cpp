/**
* @file ShaderManagerParticle.cpp
* @brief パーティクルの生成・更新・描画を統括管理するクラスの実装ファイル
* @brief 衝突演出やガード演出などのエフェクト処理をまとめています
* @brief このクラスは、ジオメトリシェーダーを用いたビルボード処理、カメラ距離によるソート、およびランダムな物理挙動を持つパーティクル群の寿命管理を担います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "ShaderManagerParticle.h"

#include "Lyselib/BinaryFile.h"
#include "Game/Common/DeviceResources.h"
#include "Game/Common/UserResources.h"
#include <DDSTextureLoader.h>
#include <random>

#include "Game/GameObjects/Camera/TPSCamera.h"
#include "Game/GameObjects/Shader/ParticleUtility.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// シェーダーファイルパス
	const wchar_t* PATH_VS_PARTICLE = L"Resources/Shaders/ParticleVS.cso";
	const wchar_t* PATH_PS_PARTICLE = L"Resources/Shaders/ParticlePS.cso";
	const wchar_t* PATH_GS_PARTICLE = L"Resources/Shaders/ParticleGS.cso";

	// テクスチャファイルパス
	const wchar_t* PATH_TEX_EFFECT_NORMAL = L"Resources/Texture/eff.png";
	const wchar_t* PATH_TEX_EFFECT_SUB = L"Resources/Texture/sEff.png";
	const wchar_t* PATH_TEX_EFFECT_DEST = L"Resources/Texture/dEff.png";

	// パーティクル生成設定
	const int    PARTICLE_GEN_COUNT = 10;   // 1回あたりの生成数
	const float PARTICLE_LIFETIME = 0.7f; // 生存時間 (秒)
	const float PARTICLE_SPEED = 5.0f; // 拡散速度
	const float PARTICLE_OFFSET_Y = 2.0f; // 生成位置の高さオフセット
	const float PARTICLE_RANGE_RAD = 10.0f; // ランダム範囲係数（コード内で使用されている変数名に準拠）

	// シェーダー定数
	const int SLOT_CBUFFER = 0; // VS/PS/GSで使用するスロット
}

/// <summary>
/// インプットレイアウト定義
/// </summary>
const std::vector<D3D11_INPUT_ELEMENT_DESC> Lyse::ShaderManagerParticle::INPUT_LAYOUT_DESC =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                                                                                                   D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(DirectX::SimpleMath::Vector3),                                                              D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, sizeof(DirectX::SimpleMath::Vector3) + sizeof(DirectX::SimpleMath::Vector4),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

/**
* @brief コンストラクタ。各種フラグの初期化を行う
*/
Lyse::ShaderManagerParticle::ShaderManagerParticle()
	: m_deviceResources(nullptr)
	, m_time(0.0f)
	, m_isGuard(false)
	, m_isCollision(false)
	, m_isKnock(false)
	, m_camera(nullptr)
{
}

/**
* @brief デストラクタ
*/
Lyse::ShaderManagerParticle::~ShaderManagerParticle()
{
}

/**
* @brief 初期化処理。シェーダー、テクスチャ、およびステートの生成を行う
* @param deviceResources : デバイスリソースへのポインタ
*/
void Lyse::ShaderManagerParticle::Initialize(DX::DeviceResources* deviceResources, UserResources* userResources)
{
	m_deviceResources = deviceResources;
	m_userResources = userResources;
	ID3D11Device1* device = m_deviceResources->GetD3DDevice();

	// シェーダーリソース作成
	CreateShaders();

	// テクスチャ読み込み
	LoadTexture(PATH_TEX_EFFECT_NORMAL);
	LoadTexture(PATH_TEX_EFFECT_SUB);
	LoadTexture(PATH_TEX_EFFECT_DEST);

	// テクスチャ読み込み確認
	assert(!m_textureList.empty() && m_textureList[0] != nullptr);

	// プリミティブバッチ作成
	m_primitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColorTexture>>(m_deviceResources->GetD3DDeviceContext());

	// ステート作成
	m_commonStates = std::make_unique<CommonStates>(device);
}

/**
* @brief テクスチャの読み込みとリストへの追加
* @param path : テクスチャファイルの相対パス
*/
void Lyse::ShaderManagerParticle::LoadTexture(const wchar_t* path)
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
* @brief 基本的な更新処理。全パーティクルの寿命を確認し、終了したものを削除する
* @param elapsedTime : 経過時間
*/
void Lyse::ShaderManagerParticle::Update(float elapsedTime)
{
	// パーティクルの更新と寿命管理
	for (auto it = m_particleList.begin(); it != m_particleList.end(); )
	{
		// 更新実行（戻り値 false で寿命切れ）
		if (!it->Update(elapsedTime))
		{
			it = m_particleList.erase(it);
		}
		else
		{
			++it;
		}
	}
}

/**
* @brief ノックバック発生時の更新およびパーティクル生成
* @param elapsedTime : 経過時間
* @param isCollision : 衝突判定フラグ
* @param isKnock : ノックバックフラグ
* @param pos : パーティクル生成の基準座標
*/
void Lyse::ShaderManagerParticle::Update(float elapsedTime, bool isCollision, bool isKnock, DirectX::SimpleMath::Vector3 pos)
{
	m_time += elapsedTime;
	m_isCollision = isCollision;
	m_isKnock = isKnock;

	// ノックバックかつ衝突時にパーティクル生成
	if (isKnock && isCollision)
	{
		for (int i = 0; i < PARTICLE_GEN_COUNT; i++)
		{
			float randRad = m_userResources->GetRandomFloat(0.0f, DirectX::XM_2PI);
			Vector3 dir(cosf(randRad), 0.0f, sinf(randRad));

			int particleType = 0; // 通常タイプ

			// パーティクル情報の作成
			ParticleUtility particle(
				PARTICLE_LIFETIME,                                      // 生存時間
				Vector3(pos.x, pos.y + PARTICLE_OFFSET_Y, pos.z),       // 生成座標
				dir * PARTICLE_SPEED,                                   // 速度
				Vector3::Zero,                                          // 加速度
				Vector3::One, Vector3::Zero,                            // スケール（初期 -> 最終）
				Color(1.f, 1.f, 1.f, 1.f), Color(1.f, 0.f, 0.f, 1.f),   // カラー（初期 -> 最終）
				particleType
			);

			// リストへ追加
			m_particleList.push_back(particle);
		}
	}
}

/**
* @brief ガードおよび衝突時の更新およびパーティクル生成
* @param elapsedTime : 経過時間
* @param isCollision : 衝突判定フラグ
* @param pos : パーティクル生成の基準座標
* @param isGuard : ガード成功フラグ
*/
void Lyse::ShaderManagerParticle::Update(float elapsedTime, bool isCollision, DirectX::SimpleMath::Vector3 pos, bool isGuard)
{
	m_time += elapsedTime;
	m_isGuard = isGuard;

	// 衝突時にパーティクル生成
	if (isCollision)
	{
		for (int i = 0; i < PARTICLE_GEN_COUNT; i++)
		{
			float randRad = m_userResources->GetRandomFloat(0.0f, DirectX::XM_2PI);
			Vector3 dir(cosf(randRad), 0.0f, sinf(randRad));

			// タイプ分け（ガードか否か）
			int particleType = isGuard ? 1 : 2;

			// パーティクル情報の作成
			ParticleUtility particle(
				PARTICLE_LIFETIME,
				Vector3(pos.x, pos.y + PARTICLE_OFFSET_Y, pos.z),
				dir * PARTICLE_SPEED,
				Vector3::Zero,
				Vector3::One, Vector3::Zero,
				Color(1.f, 1.f, 1.f, 1.f), Color(1.f, 0.f, 0.f, 1.f),
				particleType
			);

			// リストへ追加
			m_particleList.push_back(particle);
		}
	}
}

/**
* @brief 頂点、ピクセル、ジオメトリシェーダーおよび定数バッファの生成（内部処理）
*/
void Lyse::ShaderManagerParticle::CreateShaders()
{
	ID3D11Device1* device = m_deviceResources->GetD3DDevice();

	// バイナリファイルの読み込み
	BinaryFile vsData = BinaryFile::LoadFile(PATH_VS_PARTICLE);
	BinaryFile psData = BinaryFile::LoadFile(PATH_PS_PARTICLE);
	BinaryFile gsData = BinaryFile::LoadFile(PATH_GS_PARTICLE);

	// インプットレイアウト作成
	device->CreateInputLayout(
		&INPUT_LAYOUT_DESC[0],
		static_cast<UINT>(INPUT_LAYOUT_DESC.size()),
		vsData.GetData(),
		vsData.GetSize(),
		m_stageInputLayout.GetAddressOf()
	);

	// 各シェーダー作成
	if (FAILED(device->CreateVertexShader(vsData.GetData(), vsData.GetSize(), nullptr, m_vertexShaderStage.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreateVertexShader Failed.", nullptr, MB_OK);
		return;
	}

	if (FAILED(device->CreatePixelShader(psData.GetData(), psData.GetSize(), nullptr, m_pixelShaderStage.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreatePixelShader Failed.", nullptr, MB_OK);
		return;
	}

	if (FAILED(device->CreateGeometryShader(gsData.GetData(), gsData.GetSize(), nullptr, m_geometryShaderStage.ReleaseAndGetAddressOf())))
	{
		MessageBox(0, L"CreateGeometryShader Failed.", nullptr, MB_OK);
		return;
	}

	// 定数バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ParticleConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&bd, nullptr, &m_constantBuffer);
}

/**
* @brief テクスチャリソースの生成（内部処理）
*/
void Lyse::ShaderManagerParticle::CreateTextures()
{
	// 必要であればここに記述
}

/**
* @brief カメラの位置関係に基づいたビルボード行列の計算
* @param target : 注視点
* @param eye : カメラ座標
* @param up : 上方向ベクトル
*/
void Lyse::ShaderManagerParticle::CreateBillboard(DirectX::SimpleMath::Vector3 target, DirectX::SimpleMath::Vector3 eye, DirectX::SimpleMath::Vector3 up)
{
	// カメラに向くビルボード行列を作成
	m_billboardMatrix = Matrix::CreateBillboard(Vector3::Zero, eye - target, up);

	// 回転補正（180度反転）
	Matrix rotation = Matrix::Identity;
	rotation._11 = -1.0f;
	rotation._33 = -1.0f;

	m_cameraPosition = eye;
	m_cameraTarget = target;
	m_billboardMatrix = rotation * m_billboardMatrix;
}

/**
* @brief パーティクルの距離ソートおよび描画頂点リストの構築と描画実行
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerParticle::Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	// 視線ベクトルの計算
	Vector3 cameraDir = m_cameraTarget - m_cameraPosition;
	cameraDir.Normalize();

	// カメラからの距離でソート（奥から手前へ描画するため）
	m_particleList.sort(
		[&](const ParticleUtility& lhs, const ParticleUtility& rhs)
		{
			// カメラ正面方向への射影距離で比較
			float distL = cameraDir.Dot(lhs.GetPosition() - m_cameraPosition);
			float distR = cameraDir.Dot(rhs.GetPosition() - m_cameraPosition);
			return distL > distR;
		}
	);

	// 頂点リストの構築
	m_vertexList.clear();

	for (const auto& particle : m_particleList)
	{
		// カメラより後ろにある場合は描画スキップ
		if (cameraDir.Dot(particle.GetPosition() - m_cameraPosition) < 0.0f)
		{
			continue;
		}

		VertexPositionColorTexture vPCT;

		// 座標設定（中心点のみ、ジオメトリシェーダで展開）
		vPCT.position = particle.GetPosition();

		// 色設定
		vPCT.color = particle.GetCurrentColor();

		// テクスチャ座標領域にスケールとタイプ情報を格納
		// x: 現在のスケール, y: パーティクルタイプ
		vPCT.textureCoordinate = XMFLOAT2(particle.GetCurrentScale().x, static_cast<float>(particle.GetType()));

		m_vertexList.push_back(vPCT);
	}

	// 描画データがない場合は終了
	if (m_vertexList.empty())
	{
		return;
	}

	// ワールド行列の構築（単位行列）
	Matrix scale = Matrix::CreateScale(1.0f);
	Matrix rot = Matrix::CreateRotationY(0.0f);
	Matrix trans = Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
	Matrix world = scale * trans * rot;

	// シェーダー適用と描画
	LoadStageShader(view, world, proj);
}

/**
* @brief 標準的なパーティクル用シェーダーを適用し、描画を行う
* @param view : ビュー行列
* @param world : ワールド行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerParticle::LoadStageShader(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj)
{
	ID3D11DeviceContext1* context = m_deviceResources->GetD3DDeviceContext();

	// 定数バッファ更新
	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		ParticleConstantBuffer* cb = (ParticleConstantBuffer*)mapped.pData;
		// ビルボード回転を適用
		cb->matWorld = XMMatrixTranspose(m_billboardMatrix * world);
		cb->matView = XMMatrixTranspose(view);
		cb->matProj = XMMatrixTranspose(proj);
		cb->diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		cb->time = Vector4::Zero;
		cb->state = Vector4::Zero;

		context->Unmap(m_constantBuffer.Get(), 0);
	}

	// 定数バッファ設定 (VS, PS, GS)
	ID3D11Buffer* cbuf[] = { m_constantBuffer.Get() };
	context->PSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf);
	context->VSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf);
	context->GSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf);

	// サンプラー設定
	ID3D11SamplerState* samplers[] = { m_commonStates->LinearWrap() };
	context->PSSetSamplers(0, 1, samplers);

	// ブレンドステート（半透明合成）
	context->OMSetBlendState(m_commonStates->NonPremultiplied(), nullptr, 0xFFFFFFFF);

	// 深度ステート（書き込みあり）
	context->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0);

	// ラスタライザ（カリングなし）
	context->RSSetState(m_commonStates->CullNone());

	// シェーダー設定
	context->VSSetShader(m_vertexShaderStage.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShaderStage.Get(), nullptr, 0);
	context->GSSetShader(m_geometryShaderStage.Get(), nullptr, 0);

	// シェーダーリソース（テクスチャ）設定
	for (size_t i = 0; i < m_textureList.size(); i++)
	{
		context->PSSetShaderResources(static_cast<UINT>(i), 1, m_textureList[i].GetAddressOf());
	}

	// 入力レイアウト設定
	context->IASetInputLayout(m_stageInputLayout.Get());

	// 描画実行（ポイントリストとして描画 -> GSでビルボード化）
	m_primitiveBatch->Begin();
	m_primitiveBatch->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, &m_vertexList[0], m_vertexList.size());
	m_primitiveBatch->End();

	// シェーダー解除
	context->VSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
}

/**
* @brief ガード成功時などの特殊な状態に応じたシェーダー適用と描画
* @param view : ビュー行列
* @param world : ワールド行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerParticle::LoadGuardShader(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj)
{
	ID3D11DeviceContext1* context = m_deviceResources->GetD3DDeviceContext();

	// 定数バッファ更新
	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		ParticleConstantBuffer* cb = (ParticleConstantBuffer*)mapped.pData;
		cb->matWorld = XMMatrixTranspose(m_billboardMatrix * world);
		cb->matView = XMMatrixTranspose(view);
		cb->matProj = XMMatrixTranspose(proj);
		cb->diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		cb->time = Vector4::Zero;

		// 状態フラグの設定
		if (m_isKnock)
		{
			cb->state = Vector4::Zero;
		}

		if (m_isGuard)
		{
			cb->state = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			cb->state = Vector4(1.0f, 1.0f, 0.0f, 0.0f);
		}

		context->Unmap(m_constantBuffer.Get(), 0);
	}

	// 定数バッファ設定
	ID3D11Buffer* cbuf[] = { m_constantBuffer.Get() };
	context->PSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf);
	context->VSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf);
	context->GSSetConstantBuffers(SLOT_CBUFFER, 1, cbuf);

	// ステート設定
	ID3D11SamplerState* samplers[] = { m_commonStates->LinearWrap() };
	context->PSSetSamplers(0, 1, samplers);

	context->OMSetBlendState(m_commonStates->NonPremultiplied(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0);
	context->RSSetState(m_commonStates->CullNone());

	// シェーダー設定
	context->VSSetShader(m_vertexShaderStage.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShaderStage.Get(), nullptr, 0);
	context->GSSetShader(m_geometryShaderStage.Get(), nullptr, 0);

	// テクスチャ設定
	for (size_t i = 0; i < m_textureList.size(); i++)
	{
		context->PSSetShaderResources(static_cast<UINT>(i), 1, m_textureList[i].GetAddressOf());
	}

	// 描画実行
	context->IASetInputLayout(m_stageInputLayout.Get());

	m_primitiveBatch->Begin();
	m_primitiveBatch->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, &m_vertexList[0], m_vertexList.size());
	m_primitiveBatch->End();

	// 解除
	context->VSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
}

/**
* @brief 個別処理用のモデル描画関数（将来的な拡張用）
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::ShaderManagerParticle::RenderModel(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(proj);
}