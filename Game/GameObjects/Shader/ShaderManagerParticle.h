#pragma once
/**
* @file ShaderManagerParticle.h
* @brief パーティクルの生成・更新・描画を統括管理するクラスのヘッダーファイル
* @brief 衝突演出やガード演出などのエフェクト処理をまとめています
* @brief このクラスは、ジオメトリシェーダーを用いたビルボード処理、カメラ距離によるソート、およびランダムな物理挙動を持つパーティクル群の寿命管理を担います。
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
#include <list>
#include <memory>
#include <wrl/client.h>

class UserResources;
namespace Lyse
{
	class TPSCamera;
	class ParticleUtility;

	class ShaderManagerParticle
	{
	public:
		// シェーダー定数バッファ
		struct ParticleConstantBuffer
		{
			DirectX::SimpleMath::Matrix		matWorld;
			DirectX::SimpleMath::Matrix		matView;
			DirectX::SimpleMath::Matrix		matProj;
			DirectX::SimpleMath::Vector4	diffuse;    // 基本色
			DirectX::SimpleMath::Vector4	time;       // 時間情報
			DirectX::SimpleMath::Vector4	state;      // 状態フラグ等
		};

		// 入力レイアウト定義
		static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT_DESC;

	public:
		// コンストラクタ / デストラクタ
		ShaderManagerParticle();
		~ShaderManagerParticle();

		// --- 初期化・ロード ---
		void Initialize(DX::DeviceResources* deviceResources, UserResources* userResources); // 旧: Create
		void LoadTexture(const wchar_t* path);

		// --- 更新処理 ---
		void Update(float elapsedTime);

		// ノックバック等の衝撃発生時の更新
		void Update(float elapsedTime, bool isCollision, bool isKnock, DirectX::SimpleMath::Vector3 pos);

		// ガード等の接触時の更新
		void Update(float elapsedTime, bool isCollision, DirectX::SimpleMath::Vector3 pos, bool isGuard);

		// --- 描画処理 ---
		void Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

		// --- シェーダー設定 ---
		void LoadStageShader(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj);
		void LoadGuardShader(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix proj);

		// --- ビルボード計算 ---
		void CreateBillboard(
			DirectX::SimpleMath::Vector3 target,
			DirectX::SimpleMath::Vector3 eye,
			DirectX::SimpleMath::Vector3 up
		);

	private:
		// 内部リソース作成
		void CreateShaders();
		void CreateTextures();
		void RenderModel(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

	public:
		// ビルボード行列（外部参照用）
		DirectX::SimpleMath::Matrix m_billboardMatrix;

	private:
		// --- デバイスリソース参照 ---
		DX::DeviceResources* m_deviceResources;

		// --- ユーザーリソース参照 ---
		UserResources* m_userResources;

		// --- 定数バッファ ---
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

		// --- ユーティリティ ---
		DX::StepTimer m_timer;
		std::unique_ptr<DirectX::CommonStates> m_commonStates;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>> m_primitiveBatch;

		// --- テクスチャリソース ---
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textureList;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_secondaryTexture;

		// --- シェーダーオブジェクト ---
		Microsoft::WRL::ComPtr<ID3D11VertexShader>   m_vertexShaderStage;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>    m_pixelShaderStage;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShaderStage;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>    m_stageInputLayout;

		// --- 行列 ---
		DirectX::SimpleMath::Matrix m_world;
		DirectX::SimpleMath::Matrix m_view;
		DirectX::SimpleMath::Matrix m_proj;

		// --- モデルリソース（カップ等） ---
		std::unique_ptr<DirectX::Model> m_model;

		// --- オフスクリーン描画用リソース ---
		Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_renderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;

		// --- カメラ・制御パラメータ ---
		Lyse::TPSCamera* m_camera;
		DirectX::SimpleMath::Vector3 m_cameraPosition;
		DirectX::SimpleMath::Vector3 m_cameraTarget;

		/// --- エフェクト制御フラグ・パラメータ ---
		float m_time;
		bool  m_isGuard;
		bool  m_isCollision;
		bool  m_isKnock;

		// --- パーティクルデータ ---
		// 通常頂点リスト
		std::vector<DirectX::VertexPositionColorTexture> m_vertexList;
		// 特殊頂点リストS（Spark/Sub等）
		std::vector<DirectX::VertexPositionColorTexture> m_vertexListS;
		// 特殊頂点リストD（Destruction/Dynamic等）
		std::vector<DirectX::VertexPositionColorTexture> m_vertexListD;

		// パーティクルロジックリスト
		std::list<ParticleUtility> m_particleList;
	};
}