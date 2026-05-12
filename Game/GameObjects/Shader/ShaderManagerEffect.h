#pragma once
/**
* @file ShaderManagerEffect.h
* @brief エフェクト専用のシェーダーおよびレンダリングパイプラインを管理するクラスのヘッダーファイル
* @brief シールド等の特殊な描画効果を伴うオブジェクトの処理をまとめています
* @brief このクラスは、加算合成や深度バッファ読み取り専用設定を用いた透過エフェクトの描画、および時間経過によるアニメーション用パラメータの更新を管理します。
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
	class ShaderManagerEffect
	{
	public:
		// エフェクト用定数バッファ
		struct EffectConstantBuffer
		{
			DirectX::SimpleMath::Matrix		matWorld;
			DirectX::SimpleMath::Matrix		matView;
			DirectX::SimpleMath::Matrix		matProj;
			DirectX::SimpleMath::Vector4	color;      // エフェクト色
			DirectX::SimpleMath::Vector4	cameraDir;  // カメラ視線ベクトル
			DirectX::SimpleMath::Vector4	time;       // 時間経過（x成分に使用）
		};

		// 入力レイアウト定義
		static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT_DESC;

	public:
		// コンストラクタ / デストラクタ
		ShaderManagerEffect();
		~ShaderManagerEffect();

		// --- 初期化・ロード ---
		void Initialize(DX::DeviceResources* deviceResources); // 旧: Create
		void LoadTexture(const wchar_t* path);

		// --- 更新 ---
		void Update(float elapsedTime);

		// --- 描画実行 ---
		void Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

		// --- シェーダー設定 ---

		// プレイヤーシールド用シェーダー設定
		void LoadShieldShaderPlayer(
			DirectX::SimpleMath::Matrix view,
			DirectX::SimpleMath::Matrix world,
			DirectX::SimpleMath::Matrix proj
		);

		// 敵シールド用シェーダー設定
		void LoadShieldShaderEnemy(
			DirectX::SimpleMath::Matrix view,
			DirectX::SimpleMath::Matrix world,
			DirectX::SimpleMath::Matrix proj
		);

		// スカイドーム用シェーダー設定（継承・共存用）
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
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

		// --- ユーティリティ ---
		DX::StepTimer m_timer;
		std::unique_ptr<DirectX::CommonStates> m_commonStates;
		float m_time; // エフェクトアニメーション用時間

		// --- プリミティブバッチ ---
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>> m_primitiveBatch;

		// --- テクスチャリソース ---
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textureList;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_secondaryTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_floorTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skydomeTexture;

		// --- シェーダーオブジェクト（汎用） ---
		Microsoft::WRL::ComPtr<ID3D11VertexShader>   m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>    m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;

		// --- シールドエフェクト用シェーダー ---
		Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_shieldInputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shieldVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_shieldPixelShader;

		// --- スカイドーム用シェーダー ---
		Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_skydomeInputLayout; // 未使用の可能性あり
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skydomeVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_skydomePixelShader;

		// --- 行列 ---
		DirectX::SimpleMath::Matrix m_world;
		DirectX::SimpleMath::Matrix m_view;
		DirectX::SimpleMath::Matrix m_proj;

		// --- モデルリソース ---
		std::unique_ptr<DirectX::Model> m_model;

		// --- レンダーターゲット（オフスクリーン描画用） ---
		Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_renderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	};
}