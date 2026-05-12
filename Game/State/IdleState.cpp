/**
* @file IdleState.cpp
* @brief 敵の待機状態を管理するクラスの実装ファイル
* @brief 敵が待機（アイドル）中に実行される処理をまとめています
* @brief このクラスは、敵が何もしていない待機状態の挙動を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "IdleState.h"
#include "IdleState.h"
#include "Game/GameObjects/Character/Character.h"

/**
* @brief コンストラクタ
* @param enemy : 敵モデルへのポインタ
*/
IdleState::IdleState(Lyse::Character* chara)
	: m_chara(chara)
{
}

/**
* @brief 初期化処理
*/
void IdleState::Initialize()
{
}

/**
* @brief 更新前処理
*/
void IdleState::PreUpdate()
{
}

/**
* @brief  敵の待機の更新処理
* @param  keyboardStateTracker : キーボードの状態トラッカー
* @param  elapsedTime : 経過時間
*/
void IdleState::Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime); // 経過時間はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(keyboardStateTracker); // キーボード入力はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	// 待機ステートへ遷移
	m_chara->ChangeState(m_chara->GetIdleState());
}

/**
* @brief 更新後処理
*/
void IdleState::PostUpdate()
{
}

/**
* @brief 描画処理
*/
void IdleState::Render()
{
}

/**
* @brief 終了処理
*/
void IdleState::Finalize()
{
}