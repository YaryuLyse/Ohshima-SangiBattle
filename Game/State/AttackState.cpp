/**
* @file AttackState.cpp
* @brief 敵の攻撃状態を管理するクラスの実装ファイル
* @brief 敵が攻撃アニメーションを行う際の処理をまとめています
* @brief このクラスは、敵が攻撃状態に入ったときのアニメーション制御や、攻撃判定の発生・消失を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "AttackState.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/State/IdleState.h"
#include "Game/GameObjects/Collider/AttackCollider.h"

// --- 定数定義 ---
// マジックナンバー（直接書かれた数字）を避けるため、名前をつけて管理します
namespace 
{
	const float ATTACK_START_TIME = 0.3f;   // 攻撃判定が発生する時間
	const float ATTACK_END_TIME = 0.7f;   // 攻撃判定が終了する時間
	const float STATE_DURATION = 1.0f;  // このステート全体の長さ（終了時間）
	const float ATTACK_RADIUS = 2.0f;   // 攻撃判定の大きさ（半径）
	const float ATTACK_OFFSET_Z = 2.0f;   // 敵の前方どれくらいの位置に判定を出すか
	const float HITBOX_HIDE_Y = -200.0f;// 判定を隠す場所（地面より遥か下）
}

/**
* @brief コンストラクタ
* @param enemy : 敵モデルへのポインタ
*/
AttackState::AttackState(Lyse::Character* chara)
	: m_chara(chara)
{
}

/**
* @brief 初期化処理
*/
void AttackState::Initialize()
{
	// ステート開始時の初期化処理があればここに記述
}

/**
* @brief 更新前処理
*/
void AttackState::PreUpdate()
{
	// 更新前の処理があればここに記述
}

/**
* @brief  敵の攻撃の更新処理
* @param  keyboardStateTracker : キーボードの状態トラッカー
* @param  elapsedTime : 経過時間
*/
void AttackState::Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime)
{
	UNREFERENCED_PARAMETER(keyboardStateTracker); // キーボード入力はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	// 経過時間を加算
	m_timer += elapsedTime;

	// ---------------------------------------------------------
	// 攻撃判定の制御ロジック
	// ---------------------------------------------------------

	// ■ フェーズ1: 攻撃発生中 (0.5秒 ～ 1.0秒)
	if (m_timer >= ATTACK_START_TIME && m_timer < ATTACK_END_TIME) 
	{
		// 1. ローカル座標での攻撃位置を決める（敵から見て正面 Z+3.0f の位置）
		m_attackOffset = DirectX::SimpleMath::Vector3(0.0f, 1.0f, ATTACK_OFFSET_Z);

		// 2. 敵の向き（回転行列）に合わせて、攻撃位置を回転させる
		//    これをしないと、敵が横を向いても攻撃判定がずっと北(Z+)に出続けてしまいます
		DirectX::SimpleMath::Vector3 rotatedOffset = DirectX::SimpleMath::Vector3::Transform(m_attackOffset, m_chara->GetRotMat());

		// 3. 敵の現在位置に、回転させたオフセットを足して、最終的な攻撃位置を決定
		m_chara->GetAttack()->SetPosition(m_chara->GetPosition() + rotatedOffset);

		// 4. 判定の半径を設定（攻撃有効化）
		m_chara->GetAttack()->SetRadius(ATTACK_RADIUS);
	}
	// ■ フェーズ2: 攻撃終了・フォロースルー (1.0秒 ～)
	else if (m_timer >= ATTACK_END_TIME)
	{
		// 攻撃判定を画面外（地下）に移動させ、半径を0にして無効化する
		m_chara->GetAttack()->SetPosition(DirectX::SimpleMath::Vector3(0.0f, HITBOX_HIDE_Y, 0.0f));
		m_chara->GetAttack()->SetRadius(0.0f);
	}

	// ■ フェーズ3: ステート終了判定 (1.25秒経過)
	if (m_timer >= STATE_DURATION) 
	{
		// 本来はここで「待機状態(Idle)」などへ遷移させます
		m_chara->ChangeState(m_chara->GetIdleState());

		// 【暫定対応】遷移先がないため、タイマーをリセットして攻撃をループさせます
		m_timer = 0.0f;
	}
}

/**
* @brief 更新後処理
*/
void AttackState::PostUpdate()
{
	// 更新後の後処理
	// 攻撃判定の位置をリセット
	m_chara->GetAttack()->SetPosition(DirectX::SimpleMath::Vector3(0.0f, -200.0f, 0.0f));
	m_chara->GetAttack()->SetRadius(0.0f);
	m_chara->GetAttack()->SetAttackFlag(false); // ← リセット
}

/**
* @brief 描画処理
*/
void AttackState::Render()
{
	// 描画処理があればここに記述
}

/**
* @brief 終了処理
*/
void AttackState::Finalize()
{
	// ステート終了時の処理があればここに記述
}