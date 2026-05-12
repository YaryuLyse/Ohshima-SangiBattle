/**
* @file GuardBreakState.cpp
* @brief 敵のガード崩し攻撃状態を管理するクラスの実装ファイル
* @brief 敵がガード崩し攻撃を行う際の処理をまとめています
* @brief このクラスは、敵がガード崩し状態に入ったときのアニメーション制御や、攻撃判定の発生・消失を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "GuardBreakState.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/State/IdleState.h"
#include "Game/GameObjects/Collider/GuardBreakCollider.h"

// 定数定義（マジックナンバーの排除）
namespace 
{
	const float ATTACK_START_TIME = 0.7f;   // 攻撃判定が発生する時間
	const float ATTACK_END_TIME = 1.0f;   // 攻撃判定が終了する時間
	const float STATE_DURATION = 1.25f;  // このステート全体の長さ（終了時間）

	constexpr float ATTACK_RADIUS = 2.0f;    // 攻撃判定の半径
	constexpr float ATTACK_OFFSET_DIST = 2.0f;    // 敵からの攻撃発生距離（Z軸）
	constexpr float HIDDEN_Y_POS = -200.0f; // 判定を隠す場所（Y座標）
}

/**
* @brief コンストラクタ
* @param enemy : 敵モデルへのポインタ
*/
GuardBreakState::GuardBreakState(Lyse::Character* chara)
	: m_chara(chara)
{
}

/**
* @brief 初期化処理
*/
void GuardBreakState::Initialize()
{
}

/**
* @brief 更新前処理
*/
void GuardBreakState::PreUpdate()
{
}

/**
* @brief  敵のガード崩し攻撃の更新処理
* @param  keyboardStateTracker : キーボードの状態トラッカー
* @param  elapsedTime : 経過時間
*/
void GuardBreakState::Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime)
{
	UNREFERENCED_PARAMETER(keyboardStateTracker); // キーボード入力はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	m_timer += elapsedTime;

	// 発生までの時間
	if (m_timer >= ATTACK_START_TIME && m_timer < ATTACK_END_TIME) 
	{
		m_attackOffset = DirectX::SimpleMath::Vector3(0.0f, 1.0f, ATTACK_OFFSET_DIST);

		// 敵の回転に合わせてオフセットを変換
		m_attackOffset = DirectX::SimpleMath::Vector3::Transform(m_attackOffset, m_chara->GetRotMat());

		// 攻撃判定の位置と半径を設定
		m_chara->GetGuardBreakCollider()->SetPosition(m_chara->GetPosition() + m_attackOffset);
		m_chara->GetGuardBreakCollider()->SetRadius(ATTACK_RADIUS);
	}
	// 判定が消えるまで
	if (m_timer >= ATTACK_END_TIME)
	{
		m_chara->GetGuardBreakCollider()->SetPosition(DirectX::SimpleMath::Vector3(0.0f, HIDDEN_Y_POS, 0.0f));
		m_chara->GetGuardBreakCollider()->SetRadius(0.0f);
	}
	// リセット
	if (m_timer >= STATE_DURATION)
	{
		m_chara->ChangeState(m_chara->GetIdleState());
		m_timer = 0.0f; // タイマーをリセット
	}
}

/**
* @brief 更新後処理
*/
void GuardBreakState::PostUpdate()
{
	// 判定位置をリセット
	m_chara->GetGuardBreakCollider()->SetPosition(DirectX::SimpleMath::Vector3(0.0f, HIDDEN_Y_POS, 0.0f));
	m_chara->GetGuardBreakCollider()->SetRadius(0.0f);
	m_chara->GetGuardBreakCollider()->SetGuardBreakFlag(false); // ← リセット
}

/**
* @brief 描画処理
*/
void GuardBreakState::Render()
{
}

/**
* @brief 終了処理
*/
void GuardBreakState::Finalize()
{
}