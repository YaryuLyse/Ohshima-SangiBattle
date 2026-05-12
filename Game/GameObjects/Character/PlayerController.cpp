/**
* @file PlayerController.cpp
* @brief 敵のAI（意思決定）を管理するクラスの実装ファイル
* @brief プレイヤーとの距離や自身のステータスに応じた行動選択処理をまとめています
* @brief このクラスは、プレイヤーとの距離計算、行動パターンの抽選、タイマーによる状態遷移のトリガー、およびHP/スタミナに基づいた特殊な挙動を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "pch.h"
#include "PlayerController.h"

// ゲームオブジェクト
#include "Game/GameObjects/Character/Character.h"

// ステート
#include "Game/State/AttackState.h"
#include "Game/State/GuardBreakState.h"
#include "Game/State/GuardState.h"
#include "Game/State/IdleState.h"
#include "Game/State/MoveRightState.h"
#include "Game/State/MoveLeftState.h"
#include "Game/State/MoveForwardState.h"
#include "Game/State/MoveBackwardState.h"
#include "Game/State/DeathState.h"

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// 移動パラメータ
	constexpr float MOVE_SPEED = 0.1f;             // 移動速度（1フレームあたり）
}

/**
* @brief コンストラクタ。各タイマーと行動パターンの初期化を行う
* @param enemy : 制御対象の敵モデルポインタ
* @param player : ターゲットとなるプレイヤーモデルポインタ
*/
Lyse::PlayerController::PlayerController()
{
}

/**
* @brief デストラクタ
*/
Lyse::PlayerController::~PlayerController()
{
}

/**
* @brief 初期化処理。ターゲットの初期位置設定などを行う
* @param myBody : 制御対象のキャラクターポインタ
*/
void Lyse::PlayerController::Initialize(Character* myBody)
{
	UNREFERENCED_PARAMETER(myBody); // 今は使用していないが、将来的に初期化処理でmyBodyを利用する可能性があるため、警告回避のためにUNREFERENCED_PARAMETERマクロを使用しています。
	m_targetPos = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
}

/**
* @brief コントローラーの更新処理
* @param elapsedTime : 経過時間
*/
void Lyse::PlayerController::Update(Character* myBody, float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime); // 現状、経過時間はプレイヤーの更新に使用しないため、未使用パラメータとしてマクロで処理
	if (myBody->IsStunned()) return; // スタン状態なら入力を受け付けない
	if (myBody->GetCurrentState() == myBody->GetDeathState()) return; // 死亡状態なら入力を受け付けない
	// 入力の取得
	// キーボード入力更新
	auto kb = Keyboard::Get().GetState();
	m_keyboardTracker.Update(kb);
	if (m_keyboardTracker.pressed.Z)
	{
		myBody->ChangeState(myBody->GetAttackState());
	}
	else if (m_keyboardTracker.pressed.X)
	{
		myBody->ChangeState(myBody->GetGuardBreakState());
	}

	if (!(myBody->GetCurrentState() == myBody->GetAttackState() ||
		myBody->GetCurrentState() == myBody->GetGuardBreakState())) 
	{

		if (kb.C)
		{
			myBody->ChangeState(myBody->GetGuardState());
		}
		else if (kb.Right) 
		{
			myBody->ChangeState(myBody->GetMoveRightState());
		}
		else if (kb.Left) 
		{
			myBody->ChangeState(myBody->GetMoveLeftState());
		}
		else if (kb.Up)
		{
			myBody->ChangeState(myBody->GetMoveForwardState());
		}
		else if (kb.Down) 
		{
			myBody->ChangeState(myBody->GetMoveBackwardState());
		}
		else 
		{
			myBody->ChangeState(myBody->GetIdleState());

		}
	}

	//敵がいれば敵の方法を向く
	if (m_target) 
	{
		// atan2(xの差分, zの差分)
		float dx = m_target->GetPosition().x - myBody->GetPosition().x;
		float dz = m_target->GetPosition().z - myBody->GetPosition().z;
		float angle = atan2(dx, dz);

		myBody->SetRotationY(angle);
	}
}


