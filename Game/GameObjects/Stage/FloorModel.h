#pragma once
/**
* @file FloorModel.h
* @brief 地面（フロア）モデルを管理するクラスのヘッダーファイル
* @brief ステージの床となるモデルのロードおよび描画処理をまとめています
* @brief このクラスは、床モデルの描画、アルファテスト効果の設定、およびワールド空間における床の配置固定を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <memory>
#include "SimpleMath.h"

namespace Lyse
{
	class FloorModel
	{
	public:
		// コンストラクタ / デストラクタ
		FloorModel(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx);
		~FloorModel();

		// 描画
		void Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

		// 終了処理
		void Finalize();

		// ゲッター
		DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; };

	private:
		// 頂点定義
		const static DirectX::VertexPositionTexture VERTICES[4];

		// --- トランスフォーム情報 ---
		DirectX::SimpleMath::Vector3 m_position;
		DirectX::SimpleMath::Vector3 m_basePosition;
		DirectX::SimpleMath::Vector3 m_scale;
		DirectX::SimpleMath::Vector3 m_rotation;

		// --- 描画リソース ---
		Microsoft::WRL::ComPtr<ID3D11InputLayout>        m_inputLayout;
		std::unique_ptr<DirectX::CommonStates>           m_states;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
		std::unique_ptr<DirectX::AlphaTestEffect>        m_batchEffect;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_batch;

		// モデル
		std::unique_ptr<DirectX::Model> m_floorModel;
	};
}