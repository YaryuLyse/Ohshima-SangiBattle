#pragma once
/**
* @file StageBreakCollision.h
* @brief ステージ破壊時の壁衝突判定を管理するクラスのヘッダーファイル
* @brief 壁が破壊された後に発生する進入不可領域の判定処理をまとめています
* @brief このクラスは、球体衝突判定を用いたオブジェクトの押し戻し補正や、壁に接触した際の速度ベクトル反射（弾ね返り）計算を管理します。
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

	class StageBreakCollision : public Lyse::SphereCollider
	{
	public:
		// コンストラクタ / デストラクタ
		StageBreakCollision();
		~StageBreakCollision();

		// --- 初期化---
		void Initialize();

		// --- 更新 ---
		void Update();

		/// --- 描画 ---
		void Render(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>& batch);

		/// --- 終了 ---
		void Finalize();

		// --- 衝突応答 ---
		virtual void OnCollision(SphereCollider* other) override;

		// --- アクション ---
		void Activate(const DirectX::SimpleMath::Vector3& position);
		void Deactivate();

		// --- アクセサ ---
		DirectX::SimpleMath::Vector3 GetPosition() const override { return m_pos; }
		float GetRadius() const override { return m_radius; }
		bool GetIsCollision() const { return m_isCollision; }
		bool IsActive() const { return m_isActive; }

		// 外部参照設定
		void SetPlayer(Character& player) { m_player = &player; }
		void SetEnemy(Character& enemy) { m_enemy = &enemy; }
		void SetIsActive(bool isActive) { m_isActive = isActive; }

		// 必要に応じて基底クラスのセッターを使用
		void SetPosition(const DirectX::SimpleMath::Vector3& pos) override { m_pos = pos; }

	private:
		// 定数データ
		static const DirectX::VertexPositionTexture VERTICES[4];

		// --- 変換パラメータ ---
		DirectX::SimpleMath::Vector3 m_position; // 初期化用座標
		DirectX::SimpleMath::Vector3 m_prePosition;
		DirectX::SimpleMath::Vector3 m_scale;
		DirectX::SimpleMath::Vector3 m_rotation;

		// --- 衝突判定用パラメータ ---
		DirectX::SimpleMath::Vector3 m_center;
		DirectX::SimpleMath::Vector3 m_pos;   // 現在の判定中心座標
		float m_radius;                       // 判定半径
		bool  m_isCollision;                  // 衝突フラグ
		bool  m_isActive;                     // 有効フラグ

		// --- DirectX リソース（将来的な拡張用） ---
		Microsoft::WRL::ComPtr<ID3D11InputLayout>        m_inputLayout;
		std::unique_ptr<DirectX::CommonStates>           m_states;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
		std::unique_ptr<DirectX::AlphaTestEffect>        m_batchEffect;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_batch;
		std::unique_ptr<DirectX::Model>                  m_modelCollision;

		// --- 外部参照・ユーティリティ ---
		Character* m_player;
		Character* m_enemy;
	};
}