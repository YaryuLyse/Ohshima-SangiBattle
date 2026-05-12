#pragma once
/**
* @file Skydome.h
* @brief スカイドーム（背景天球）の挙動を管理するクラスのヘッダーファイル
* @brief 巨大な背景モデルの描画およびプレイヤーへの追従処理をまとめています
* @brief このクラスは、背景モデルのロード、アルファテストの設定、および描画時にプレイヤーの位置と同期させることで無限遠の背景を擬似的に表現する管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <SimpleMath.h>
#include <Effects.h>
#include <Model.h>
#include <CommonStates.h>
#include <memory>
#include <wrl/client.h>

namespace Lyse
{
	class ShaderManager;

	class Skydome
	{
	public:
		// コンストラクタ / デストラクタ
		Skydome(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx);
		~Skydome();

		// --- 初期化・終了 ---
		void Initialize(ShaderManager* shaderManager);
		void Finalize();

		// --- 更新・描画 ---
		void Update();
		void Render(ID3D11DeviceContext1* context, DirectX::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

		// --- アクセサ ---
		void SetPlayerPosition(DirectX::SimpleMath::Vector3 pos) { m_position = pos; }

	private:
		// 定数データ
		static const DirectX::VertexPositionTexture VERTICES[4];

		// --- 変換パラメータ ---
		DirectX::SimpleMath::Vector3 m_position;
		DirectX::SimpleMath::Vector3 m_prePosition;
		DirectX::SimpleMath::Vector3 m_scale;
		DirectX::SimpleMath::Vector3 m_rotation;

		DirectX::SimpleMath::Vector3 m_attackVec;

		// --- DirectX リソース ---
		// 入力レイアウト
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		// 共通ステート
		std::unique_ptr<DirectX::CommonStates> m_states;

		// テクスチャ
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

		// エフェクト（プリミティブ用）
		std::unique_ptr<DirectX::AlphaTestEffect> m_batchEffect;

		// プリミティブバッチ
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_batch;

		// --- モデルリソース ---
		std::unique_ptr<DirectX::Model> m_modelSkydome;

		// --- 外部参照 ---
		ShaderManager* m_shaderManager;
	};
}