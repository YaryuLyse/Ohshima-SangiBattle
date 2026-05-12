/**
* @file GuardState.cpp
* @brief 敵の防御状態を管理するクラスの実装ファイル
* @brief 敵が防御アクションを行う際の処理をまとめています
* @brief このクラスは、敵が防御状態に入ったときのスタミナ管理や、ガードブレイク（クラッシュ）判定を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "GuardState.h"
#include "IdleState.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/State/IdleState.h"

// 定数定義（マジックナンバーの排除）
namespace 
{
	constexpr float STAMINA_DRAIN_RATE = 5.0f; // スタミナの減少速度
	constexpr float STAMINA_MIN = 0.0f; // スタミナの下限値
}

/**
* @brief コンストラクタ
* @param enemy : 敵モデルへのポインタ
*/
GuardState::GuardState(Lyse::Character* chara)
	: m_chara(chara)
{
}

/**
* @brief 初期化処理
*/
void GuardState::Initialize()
{
}

/**
* @brief 更新前処理
*/
void GuardState::PreUpdate()
{
	// 体制が崩れているか確認
	if (m_chara->GetIsStunned() == true)
	{
		// 待機ステートへ遷移
		m_chara->ChangeState(m_chara->GetIdleState());
	}
}

/**
* @brief  敵の防御の更新処理
* @param  keyboardStateTracker : キーボードの状態トラッカー
* @param  elapsedTime : 経過時間
*/
void GuardState::Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime)
{
	UNREFERENCED_PARAMETER(keyboardStateTracker); // キーボード入力はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	// 時間経過とともにスタミナを減少させる
	m_chara->SetStamina(m_chara->GetStamina() - STAMINA_DRAIN_RATE * elapsedTime);

	// スタミナが尽きた場合の処理
	if (m_chara->GetStamina() <= STAMINA_MIN) 
	{
		// スタミナを0に固定
		m_chara->SetStamina(STAMINA_MIN);

		// クラッシュ状態（ガードブレイク状態）にする
		m_chara->SetIsStunned(true);
	}
}

/**
* @brief 更新後処理
*/
void GuardState::PostUpdate()
{
}

/**
* @brief 描画処理
*/
void GuardState::Render()
{
}

/**
* @brief 終了処理
*/
void GuardState::Finalize()
{
}