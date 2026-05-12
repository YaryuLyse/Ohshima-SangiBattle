#pragma once
/**
* @file StageCollision.h
* @brief ステージ外周の衝突判定を管理するクラスのヘッダーファイル
* @brief 円形ステージの境界維持と、壁破壊のトリガー処理をまとめています
* @brief このクラスは、プレイヤーや敵がステージ外へ出ないように押し戻す処理に加え、衝突時の角度から破壊対象の壁を特定する管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <SimpleMath.h>
#include <CommonStates.h>
#include <Effects.h>
#include <Model.h>
#include <memory>
#include <wrl/client.h>

#include "Game/GameObjects/Collider/SphereCollider.h"

namespace Lyse
{
	class Character;
	class Stage;

	class StageCollision : public Lyse::SphereCollider
	{
	public:
		// コンストラクタ / デストラクタ
		StageCollision(const DirectX::SimpleMath::Vector3& center, float radius);
		~StageCollision();

		// --- 初期化 ---
		void Initialize();

		// --- 更新 ---
		void Update();

		/// --- 描画 ---
		void Render(ID3D11DeviceContext1* context, DirectX::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

		/// --- 終了 ---
		void Finalize();

		// --- 衝突応答 ---
		virtual void OnCollision(SphereCollider* other) override;

		// --- アクセサ ---
		DirectX::SimpleMath::Vector3 GetPosition() const override { return m_center; }
		float GetRadius() const override { return m_radius; }
		bool GetIsCollision() const { return m_isCollision; }

		// 衝突後の補正座標を取得
		DirectX::SimpleMath::Vector3 GetPrePosition() const { return m_pos; }

		// --- 外部参照設定 ---
		void SetStage(Stage& stage) { m_stage = &stage; }
		void SetPlayer(Character& player) { m_player = &player; }
		void SetEnemy(Character& enemy) { m_enemy = &enemy; }
		void ClampToStage(Lyse::Character* chara);

	private:
		// 定数データ
		static const DirectX::VertexPositionTexture VERTICES[4];

		// --- 変換パラメータ ---
		DirectX::SimpleMath::Vector3 m_position;
		DirectX::SimpleMath::Vector3 m_prePosition;
		DirectX::SimpleMath::Vector3 m_scale;
		DirectX::SimpleMath::Vector3 m_rotation;

		// --- DirectX リソース ---
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
		std::unique_ptr<DirectX::CommonStates> m_states;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
		std::unique_ptr<DirectX::AlphaTestEffect> m_batchEffect;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_batch;

		// モデル
		std::unique_ptr<DirectX::Model> m_modelCollision;

		// --- 衝突判定用パラメータ ---
		DirectX::SimpleMath::Vector3 m_center;  // 中心座標
		DirectX::SimpleMath::Vector3 m_pos;     // 衝突点（補正後）
		float m_radius;                         // 半径
		bool  m_isCollision;                    // 衝突フラグ

		// --- 外部参照 ---
		Stage* m_stage;
		Character* m_player;
		Character* m_enemy;
	};
}