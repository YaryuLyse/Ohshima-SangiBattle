#pragma once
/**
* @file AttackCollider.h
* @brief 敵の攻撃判定を管理するクラスのヘッダーファイル
* @brief 敵の攻撃判定（コリジョン）の生成や描画、衝突時の挙動をまとめています
* @brief このクラスは、攻撃判定の位置・半径の管理や、他オブジェクトとの衝突判定時の消失処理を制御します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include <PrimitiveBatch.h>
#include "Game/GameObjects/Collider/SphereCollider.h"

namespace Lyse
{
	// 敵の攻撃判定（当たり判定）を管理するクラス
	// 衝突判定用クラス SphereCollider を継承しています
	class AttackCollider : public Lyse::SphereCollider
	{
	public:
		// コンストラクタ
		AttackCollider();
		// デストラクタ
		~AttackCollider();

		// 判定の位置を設定
		void SetPosition(DirectX::SimpleMath::Vector3 position);

		// 判定の半径を設定
		void SetRadius(float radius);

		// デバッグ描画（当たり判定の可視化）
		void Render(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>& batch);

		// --- SphereCollider のオーバーライド ---

		// 現在の位置を取得
		DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; };

		// 半径を取得（円形衝突用）
		float GetRadius() const override { return m_radius; };

		// --- フラグ管理 ---

		// 攻撃フラグを設定
		void SetAttackFlag(bool flag) { m_attackFlag = flag; }

		// 攻撃フラグを取得
		bool GetAttackFlag() const { return m_attackFlag; }

		// 衝突時のコールバック関数
		void OnCollision(SphereCollider* other) override;

	private:
		// 判定の中心座標
		DirectX::SimpleMath::Vector3 m_position;

		// 判定の半径
		float m_radius{ 0.0f };

		// 攻撃が有効かどうかのフラグ
		bool m_attackFlag = false;
	};
}