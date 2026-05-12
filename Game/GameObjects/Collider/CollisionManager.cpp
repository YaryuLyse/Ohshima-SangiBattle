/**
* @file CollisionManager.cpp
* @brief 衝突判定を統括管理するクラスの実装ファイル
* @brief ゲーム内の全オブジェクト間の衝突判定ロジックをまとめています
* @brief このクラスは、プレイヤー、敵、攻撃判定、ステージオブジェクトなどを各リストで管理し、円判定（距離の二乗）を用いた効率的な衝突検知と通知を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "CollisionManager.h"
#include <cmath>
#include "Game/GameObjects/Collider/SphereCollider.h"

using namespace DirectX;

// --- オブジェクト登録 ---

/**
* @brief 汎用衝突判定リストへオブジェクトを追加
* @param obj : 衝突判定を持つオブジェクト
*/
void Lyse::CollisionManager::AddCollisionObject(Lyse::SphereCollider& obj)
{
	m_collisionList.push_back(&obj);
}

/**
* @brief 敵攻撃判定リストへオブジェクトを追加
* @param obj : 衝突判定を持つオブジェクト
*/
void Lyse::CollisionManager::AddEnemyAttackObject(Lyse::SphereCollider& obj)
{
	m_enemyAttackList.push_back(&obj);
}

/**
* @brief プレイヤー攻撃判定リストへオブジェクトを追加
* @param obj : 衝突判定を持つオブジェクト
*/
void Lyse::CollisionManager::AddPlayerAttackObject(Lyse::SphereCollider& obj)
{
	m_playerAttackList.push_back(&obj);
}

/**
* @brief ステージオブジェクトリストへオブジェクトを追加
* @param obj : 衝突判定を持つオブジェクト
*/
void Lyse::CollisionManager::AddStageObject(Lyse::SphereCollider& obj)
{
	m_stageList.push_back(&obj);
}

/**
* @brief 破壊可能（または特殊判定）ステージオブジェクトリストへオブジェクトを追加
* @param obj : 衝突判定を持つオブジェクト
*/
void Lyse::CollisionManager::AddStageBreakObject(Lyse::SphereCollider& obj)
{
	m_stageBreakList.push_back(&obj);
}

/**
* @brief プレイヤーリストへオブジェクトを追加
* @param obj : 衝突判定を持つオブジェクト
*/
void Lyse::CollisionManager::AddPlayerObject(Lyse::SphereCollider& obj)
{
	m_playerList.push_back(&obj);
}

// --- 管理処理 ---

/**
* @brief 全ての衝突判定管理リストをクリアする
*/
void Lyse::CollisionManager::Clear()
{
	m_collisionList.clear();
	m_enemyAttackList.clear();
	m_playerAttackList.clear();
	m_stageList.clear();
	m_playerList.clear();
	m_stageBreakList.clear();
}

// --- 衝突判定処理 ---

/**
* @brief 汎用リスト内の全オブジェクト同士の衝突判定を行う
*/
void Lyse::CollisionManager::CheckAllCollisions()
{
	// 総当たり判定
	for (size_t i = 0; i < m_collisionList.size(); ++i) 
	{
		for (size_t j = i + 1; j < m_collisionList.size(); ++j) 
		{
			Lyse::SphereCollider* obj1 = m_collisionList[i];
			Lyse::SphereCollider* obj2 = m_collisionList[j];

			DirectX::SimpleMath::Vector3 diff = obj1->GetPosition() - obj2->GetPosition();
			float distanceSq = diff.LengthSquared();
			float radiusSum = obj1->GetRadius() + obj2->GetRadius();

			if (distanceSq <= radiusSum * radiusSum) 
			{
				obj1->OnCollision(obj2);
				obj2->OnCollision(obj1);
			}
		}
	}
}

/**
* @brief 敵攻撃オブジェクト同士の衝突判定を行う
*/
void Lyse::CollisionManager::CheckAttackToEnemy()
{
	for (size_t i = 0; i < m_enemyAttackList.size(); ++i) 
	{
		for (size_t j = i + 1; j < m_enemyAttackList.size(); ++j) 
		{
			Lyse::SphereCollider* obj1 = m_enemyAttackList[i];
			Lyse::SphereCollider* obj2 = m_enemyAttackList[j];

			DirectX::SimpleMath::Vector3 diff = obj1->GetPosition() - obj2->GetPosition();
			float distanceSq = diff.LengthSquared();
			float radiusSum = obj1->GetRadius() + obj2->GetRadius();

			if (distanceSq <= radiusSum * radiusSum) 
			{
				obj1->OnCollision(obj2);
				obj2->OnCollision(obj1);
			}
		}
	}
}

/**
* @brief プレイヤー攻撃オブジェクト同士の衝突判定を行う
*/
void Lyse::CollisionManager::CheckAttackToPlayer()
{
	for (size_t i = 0; i < m_playerAttackList.size(); ++i) 
	{
		for (size_t j = i + 1; j < m_playerAttackList.size(); ++j) 
		{
			Lyse::SphereCollider* obj1 = m_playerAttackList[i];
			Lyse::SphereCollider* obj2 = m_playerAttackList[j];

			DirectX::SimpleMath::Vector3 diff = obj1->GetPosition() - obj2->GetPosition();
			float distanceSq = diff.LengthSquared();
			float radiusSum = obj1->GetRadius() + obj2->GetRadius();

			if (distanceSq <= radiusSum * radiusSum) 
			{
				obj1->OnCollision(obj2);
				obj2->OnCollision(obj1);
			}
		}
	}
}

/**
* @brief プレイヤーとステージオブジェクトの衝突判定を行う
*/
void Lyse::CollisionManager::CheckPlayerToStage()
{
	for (auto* player : m_playerList)
	{
		for (auto* stage : m_stageList)
		{
			// 座標の差分を求める
			DirectX::SimpleMath::Vector3 diff = player->GetPosition() - stage->GetPosition();

			// 距離の二乗を計算
			float distanceSq = diff.LengthSquared();

			// 両方の半径を足す
			float radiusSum = player->GetRadius() + stage->GetRadius();

			// 当たっているかどうか判定
			if (distanceSq <= radiusSum * radiusSum)
			{
				// コリジョン処理を通知
				// player->OnCollision(stage); // 元コードに従いコメントアウト
				stage->OnCollision(player);
			}
		}
	}
}

/**
* @brief プレイヤーと破壊可能ステージオブジェクトの衝突判定を行う
*/
void Lyse::CollisionManager::CheckPlayerToStageBreak()
{
	for (auto* player : m_playerList)
	{
		for (auto* stageBreak : m_stageBreakList)
		{
			// 座標の差分を求める
			DirectX::SimpleMath::Vector3 diff = player->GetPosition() - stageBreak->GetPosition();

			// 距離の二乗を計算
			float distanceSq = diff.LengthSquared();

			// 両方の半径を足す
			float radiusSum = player->GetRadius() + stageBreak->GetRadius();


			//判定とってる場所
			// 当たっているかどうか判定
			if (distanceSq <= radiusSum * radiusSum)
			{
				// コリジョン処理を通知
				player->OnCollision(stageBreak);
				//stageBreak->OnCollision(player);
			}
		}
	}
}