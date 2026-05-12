#pragma once
/**
* @file CollisionManager.h
* @brief 衝突判定を統括管理するクラスのヘッダーファイル
* @brief ゲーム内の全オブジェクト間の衝突判定ロジックをまとめています
* @brief このクラスは、プレイヤー、敵、攻撃判定、ステージオブジェクトなどを各リストで管理し、円判定（距離の二乗）を用いた効率的な衝突検知と通知を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include <vector>
#include "SimpleMath.h"

// 前方宣言
namespace Lyse {
	class SphereCollider;
}

namespace Lyse
{
	class CollisionManager
	{
	public:
		// コンストラクタ / デストラクタ
		CollisionManager() = default;
		~CollisionManager() = default;

		// --- オブジェクト登録 ---
		void AddCollisionObject(Lyse::SphereCollider& obj);
		void AddPlayerAttackObject(Lyse::SphereCollider& obj);
		void AddEnemyAttackObject(Lyse::SphereCollider& obj);
		void AddStageObject(Lyse::SphereCollider& obj);
		void AddStageBreakObject(Lyse::SphereCollider& obj);
		void AddPlayerObject(Lyse::SphereCollider& obj);

		// --- 管理 ---
		void Clear();

		// --- 衝突判定実行 ---
		void CheckAllCollisions();        // 汎用衝突判定
		void CheckAttackToEnemy();        // 敵への攻撃判定
		void CheckAttackToPlayer();       // プレイヤーへの攻撃判定
		void CheckPlayerToStage();        // プレイヤーとステージの判定
		void CheckPlayerToStageBreak();   // プレイヤーと破壊可能オブジェクトの判定

	private:
		// 判定対象リスト
		std::vector<Lyse::SphereCollider*> m_collisionList;
		std::vector<Lyse::SphereCollider*> m_playerAttackList;
		std::vector<Lyse::SphereCollider*> m_enemyAttackList;
		std::vector<Lyse::SphereCollider*> m_stageList;
		std::vector<Lyse::SphereCollider*> m_stageBreakList;
		std::vector<Lyse::SphereCollider*> m_playerList;
	};
}