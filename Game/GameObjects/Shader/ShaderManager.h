#pragma once
/**
* @file ShaderManager.h
* @brief シェーダーおよびレンダリングパイプラインを統括管理するクラスのヘッダーファイル
* @brief フィールドやスカイドーム等、各オブジェクト専用シェーダーの適用処理をまとめています
* @brief このクラスは、頂点/ピクセルシェーダーの生成、入力レイアウトの定義、定数バッファの転送、およびDirectXTKのモデル描画と連携した描画ステートの切り替えを管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "Game/Common/StepTimer.h"
#include "Game/Common/DeviceResources.h"
#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <Model.h>
#include <vector>
#include <memory>
#include <wrl/client.h>

namespace Lyse
{
	class ShaderManager
	{
	public:
		// 共通定数バッファ（View/Projなど）
		struct CommonConstantBuffer
		{
			DirectX::SimpleMath::Matrix		matWorld;
			DirectX::SimpleMath::Matrix		matView;
			DirectX::SimpleMath::Matrix		matProj;
			DirectX::SimpleMath::Vector4	Diffuse;
		};

		// モデル用定数バッファ（フラッシュエフェクト等）
		struct ModelConstantBuffer
		{
			DirectX::SimpleMath::Matrix		matWorld;
			DirectX::SimpleMath::Matrix		matView;
			DirectX::SimpleMath::Matrix		matProj;
			DirectX::SimpleMath::Vector4	flashColor;
		};

		// 入力レイアウト定義
		static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT_DESC;

	public:
		// コンストラクタ / デストラクタ
		ShaderManager();
		~ShaderManager();

		// --- 初期化・ロード ---
		void Initialize(DX::DeviceResources* deviceResources); // 旧: Create
		void LoadTexture(const wchar_t* path);

		// --- 描画実行 ---
		void Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

		// --- シェーダー設定 ---
		void LoadStageShader(
			DirectX::SimpleMath::Matrix view,
			DirectX::SimpleMath::Matrix world,
			DirectX::SimpleMath::Matrix proj,
			DirectX::DX11::Model* model,
			float flashIntensity
		);

		void LoadSkydomeShader(
			DirectX::SimpleMath::Matrix view,
			DirectX::SimpleMath::Matrix world,
			DirectX::SimpleMath::Matrix proj,
			DirectX::DX11::Model* model
		);

		// SRVの手動設定
		void SetShaderResourceView(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv); // 旧: SetSRV

	private:
		// 内部描画処理
		void RenderModel(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);
		void CreateShaders(); // 旧: CreateShader
		void CreateTextures(); // 旧: CreateTexture

	private:
		// --- デバイスリソース参照 ---
		DX::DeviceResources* m_deviceResources;

		// --- 定数バッファ ---
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer; // 旧: m_CBuffer

		// --- ユーティリティ ---
		DX::StepTimer m_timer;
		std::unique_ptr<DirectX::CommonStates> m_commonStates; // 旧: m_states

		// --- プリミティブバッチ ---
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>> m_primitiveBatch; // 旧: m_batch

		// --- テクスチャリソース ---
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textureList; // 旧: m_texture
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_secondaryTexture;          // 旧: m_texture2
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_floorTexture;              // 旧: m_floorTex
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skydomeTexture;            // 旧: m_skydomeTex

		// --- シェーダーオブジェクト（汎用） ---
		Microsoft::WRL::ComPtr<ID3D11VertexShader>   m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>    m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;

		// --- ステージ描画用シェーダー ---
		Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_stageInputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_stageVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_stagePixelShader;

		// --- スカイドーム描画用シェーダー ---
		Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_skydomeInputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skydomeVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_skydomePixelShader;

		// --- 行列 ---
		DirectX::SimpleMath::Matrix m_world;
		DirectX::SimpleMath::Matrix m_view;
		DirectX::SimpleMath::Matrix m_proj;

		// --- モデルリソース ---
		std::unique_ptr<DirectX::Model> m_model; // カップモデル等

		// --- レンダーターゲット（オフスクリーン描画用） ---
		Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_renderTargetTexture; // 旧: m_capture
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_renderTargetView;    // 旧: m_rtv
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;  // 旧: m_srv
	};
}