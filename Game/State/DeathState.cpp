/**
* @file DeathState.cpp
* @brief 敵の攻撃状態を管理するクラスの実装ファイル
* @brief 敵が攻撃アニメーションを行う際の処理をまとめています
* @brief このクラスは、敵が攻撃状態に入ったときのアニメーション制御や、攻撃判定の発生・消失を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "DeathState.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/State/IdleState.h"


// --- 定数定義 ---
// マジックナンバー（直接書かれた数字）を避けるため、名前をつけて管理します
namespace 
{
	constexpr static float DEATH_DURATION = 2.0f; // 倒れる時間
}

/**
* @brief コンストラクタ
* @param enemy : 敵モデルへのポインタ
*/
DeathState::DeathState(Lyse::Character* chara)
	: m_chara(chara)
{
}

/**
* @brief 初期化処理
*/
void DeathState::Initialize()
{
	// ステート開始時の初期化処理があればここに記述
}

/**
* @brief 更新前処理
*/
void DeathState::PreUpdate()
{
	// 更新前の処理があればここに記述
}

/**
* @brief  敵の攻撃の更新処理
* @param  keyboardStateTracker : キーボードの状態トラッカー
* @param  elapsedTime : 経過時間
*/
void DeathState::Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime)
{
	UNREFERENCED_PARAMETER(keyboardStateTracker); // キーボード入力はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	// 経過時間を加算
	m_timer += elapsedTime;

	// Z軸を徐々に回転させて倒れる
	// 0度 → 90度 まで DEATH_DURATION 秒かけて回転
	float rotZ = (m_timer / DEATH_DURATION) * 90.0f;
	rotZ = std::min(rotZ, 90.0f); // 90度で止まる
	m_chara->SetDeathRotZ(DirectX::XMConvertToRadians(rotZ));

	

   // 一定時間後に死亡フラグを立てる
	if (m_timer >= DEATH_DURATION) 
	{
		m_chara->IsDeath(); // m_isDead = true
	}
}

/**
* @brief 更新後処理
*/
void DeathState::PostUpdate()
{
	// 更新後の後処理
	// 攻撃判定の位置をリセット
	
}

/**
* @brief 描画処理
*/
void DeathState::Render()
{
	// 描画処理があればここに記述
}

/**
* @brief 終了処理
*/
void DeathState::Finalize()
{
	// ステート終了時の処理があればここに記述
}