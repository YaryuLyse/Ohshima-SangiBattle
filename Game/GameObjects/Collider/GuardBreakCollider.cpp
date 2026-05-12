/**
* @file GuardBreakCollider.cpp
* @brief 敵の攻撃判定を管理するクラスの実装ファイル
* @brief 敵の攻撃判定（コリジョン）の生成や描画、衝突時の挙動をまとめています
* @brief このクラスは、攻撃判定の位置・半径の管理や、他オブジェクトとの衝突判定時の消失処理を制御します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "GuardBreakCollider.h"
#include "Game/GameObjects/Character/Character.h"

// 定数定義（マジックナンバーの排除）
namespace 
{
	constexpr float HIDDEN_POS_Y = -200.0f; // 判定を隠す場所（Y座標）
	constexpr size_t DEBUG_DIVISIONS = 16;      // デバッグ描画の分割数（球の滑らかさ）
}

/**
* @brief コンストラクタ
*/
Lyse::GuardBreakCollider::GuardBreakCollider()
	: m_position(0.0f, HIDDEN_POS_Y, 0.0f) // 初期位置は画面外へ
{
}

/**
* @brief デストラクタ
*/
Lyse::GuardBreakCollider::~GuardBreakCollider()
{
}

/**
* @brief 攻撃判定の位置を設定
* @param position : 設定する座標
*/
void Lyse::GuardBreakCollider::SetPosition(DirectX::SimpleMath::Vector3 position)
{
	m_position = position;
}

/**
* @brief 攻撃判定の半径を設定
* @param radius : 設定する半径
*/
void Lyse::GuardBreakCollider::SetRadius(float radius)
{
	m_radius = radius;
}

/**
* @brief 攻撃判定のデバッグ描画処理
* @param batch : プリミティブバッチへの参照
*/
void Lyse::GuardBreakCollider::Render(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>& batch)
{
	UNREFERENCED_PARAMETER(batch); // デバッグ描画は後で実装予定
}

/**
* @brief 衝突時の処理
* @param other : 衝突相手のコリジョンオブジェクト
*/
void Lyse::GuardBreakCollider::OnCollision(SphereCollider* other)
{
	// EnemyModel（敵自身や他の敵）と衝突した場合の処理
	if (dynamic_cast<Character*>(other)) 
	{
	}
}