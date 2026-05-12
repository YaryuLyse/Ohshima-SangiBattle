/**
* @file EnemyController.cpp
* @brief 敵のAI（意思決定）を管理するクラスの実装ファイル
* @brief プレイヤーとの距離や自身のステータスに応じた行動選択処理をまとめています
* @brief このクラスは、プレイヤーとの距離計算、行動パターンの抽選、タイマーによる状態遷移のトリガー、およびHP/スタミナに基づいた特殊な挙動を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "pch.h"
#include "EnemyController.h"

// リソース
#include "Game/Common/UserResources.h"

// ゲームオブジェクト
#include "Game/GameObjects/Character/Character.h"

// ステート
#include "Game/State/MoveForwardState.h"
#include "Game/State/AttackState.h"
#include "Game/State/GuardBreakState.h"
#include "Game/State/MoveLeftState.h"
#include "Game/State/MoveBackwardState.h"
#include "Game/State/GuardState.h"
#include "Game/State/IdleState.h"
#include "Game/State/DeathState.h"



using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// 距離判定
	constexpr float DIST_ATTACK_NEAR = 2.5f;  // 近距離攻撃範囲
	constexpr float DIST_ATTACK_NORMAL = 5.0f;  // 通常攻撃範囲
	constexpr float DIST_ALERT = 15.0f; // 警戒範囲

	// 行動切り替え時間（秒）
	constexpr float TIME_LIMIT_GUARD = 2.0f;   // パターン0用
	constexpr float TIME_LIMIT_GB = 2.0f;  // パターン1用
	constexpr float TIME_LIMIT_ATTACK = 2.0f; // パターン3用
	constexpr float TIME_LIMIT_ATTACKS = 2.0f; // パターン3用

	// 特定行動トリガー
	constexpr int   HP_TRIGGER_VAL = 60;     // 特定行動をとるHP
	constexpr float STAMINA_BORDER = 60.0f;  // スタミナ閾値

	// 行動パターンID
	constexpr int PATTERN_GUARD = 0;
	constexpr int PATTERN_GB = 1;
	constexpr int PATTERN_ATTACK = 2;
	constexpr int PATTERN_IDLE = 3;
}

/**
* @brief コンストラクタ。各タイマーと行動パターンの初期化を行う
* @param enemy : 制御対象の敵モデルポインタ
* @param player : ターゲットとなるプレイヤーモデルポインタ
*/
Lyse::EnemyController::EnemyController()
	: m_actionPattern(0)
	, m_debugCount(0)
	, m_target(nullptr)
	, m_isTutorialMode(false)
	, m_tutorialPattern(PATTERN_IDLE)
    , m_limitTimePattern{
       TIME_LIMIT_GUARD,     // [0] PATTERN_GUARD
       TIME_LIMIT_GB,        // [1] PATTERN_GB
       TIME_LIMIT_ATTACK,    // [2] PATTERN_ATTACK
       TIME_LIMIT_ATTACKS,   // [3] PATTERN_IDLE
       0.0f                  // [4] PATTERN_STUN（タイムアウト不要）
    }
	, m_timerPattern{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
	, m_userResources(nullptr)
{
}

/**
* @brief デストラクタ
*/
Lyse::EnemyController::~EnemyController()
{
}

/**
* @brief 初期化処理。現状は特に行う処理はないが、将来的な拡張のためにメソッドを用意している
* @param myBody : 制御対象の敵モデルポインタ
*/
void Lyse::EnemyController::Initialize(Lyse::Character* myBody)
{
	UNREFERENCED_PARAMETER(myBody);  // 現状、初期化時に特に行う処理はないため、未使用パラメータとしてマクロで処理
}

/**
* @brief 敵AIの思考更新処理。距離に応じてステートを切り替え、一定時間で行動を再考する
* @param elapsedTime : 経過時間
*/
void Lyse::EnemyController::Update(Character* myBody, float elapsedTime)
{
    if (myBody->IsStunned()) return;
    if (myBody->GetCurrentState() == myBody->GetDeathState()) return;
    // チュートリアルモード中は固定パターンで動く
    if (m_isTutorialMode) 
    {
        if (m_target) 
        {
            float dx = m_target->GetPosition().x - myBody->GetPosition().x;
            float dz = m_target->GetPosition().z - myBody->GetPosition().z;
            float angle = atan2(dx, dz);
            myBody->SetRotationY(angle);

            DirectX::SimpleMath::Vector3 direction = m_target->GetPosition() - myBody->GetPosition();
            float distance = direction.Length();
            switch (m_tutorialPattern) 
            {
            case PATTERN_GUARD:
                myBody->ChangeState(myBody->GetGuardState());
                break;
            case PATTERN_ATTACK:
                if (distance < DIST_ATTACK_NEAR) 
                {
                    // 近距離：攻撃
                    myBody->ChangeState(myBody->GetAttackState());
                }
                else 
                {
                    // 離れていたら近づく
                    myBody->ChangeState(myBody->GetMoveForwardState());
                }
                break;
            case PATTERN_IDLE:
                myBody->ChangeState(myBody->GetIdleState());
                break;
            }
        }
        return; // 通常AIは動かさない
    }
    // 攻撃中はステート変更しない
    if (myBody->GetCurrentState() == myBody->GetAttackState() ||
        myBody->GetCurrentState() == myBody->GetGuardBreakState()) 
    {
        return; // ← 追加
    }
    if (!m_target) return;

    float dx = m_target->GetPosition().x - myBody->GetPosition().x;
    float dz = m_target->GetPosition().z - myBody->GetPosition().z;
    float angle = atan2(dx, dz);
    myBody->SetRotationY(angle);

    DirectX::SimpleMath::Vector3 direction = m_target->GetPosition() - myBody->GetPosition();
    float distance = direction.Length();

    // --- 状況に応じた割り込み行動 ---

    // HPが少ない時はガードを優先
    if (myBody->GetHP() <= HP_TRIGGER_VAL) 
    {
        if (myBody->GetStamina() > STAMINA_BORDER) 
        {
            myBody->ChangeState(myBody->GetGuardState());
            return;
        }
        else 
        {
            // スタミナも少ない時は少し距離を取る
            myBody->ChangeState(myBody->GetMoveBackwardState());
            return;
        }
    }

    // プレイヤーがガード中ならガードブレイクを狙う
    if (dynamic_cast<GuardState*>(m_target->GetCurrentState())) 
    {
        if (distance < DIST_ATTACK_NORMAL && myBody->GetStamina() > STAMINA_BORDER) 
        {
            myBody->ChangeState(myBody->GetGuardBreakState());
            m_timerPattern[1] += elapsedTime;
            // タイムオーバーチェック
            if (m_timerPattern[1] >= m_limitTimePattern[1])
            {
                DetermineNextAction(myBody);
            }
            return;
        }
    }

    // --- 通常の行動パターン ---
    switch (m_actionPattern) 
    {
    case PATTERN_GUARD:
        if (distance < DIST_ATTACK_NORMAL) 
        {
            myBody->ChangeState(myBody->GetGuardState());
            m_timerPattern[m_actionPattern] += elapsedTime;
        }
        else if (distance < DIST_ALERT) 
        {
            myBody->ChangeState(myBody->GetMoveForwardState());
        }
        break;

    case PATTERN_GB:
        if (distance < DIST_ATTACK_NORMAL) 
        {
            myBody->ChangeState(myBody->GetGuardBreakState());
            m_timerPattern[m_actionPattern] += elapsedTime;
        }
        else if (distance < DIST_ALERT) 
        {
            myBody->ChangeState(myBody->GetMoveForwardState());
        }
        break;

    case PATTERN_ATTACK:
        if (distance < DIST_ATTACK_NEAR) 
        {
            myBody->ChangeState(myBody->GetAttackState());
            m_timerPattern[m_actionPattern] += elapsedTime;
        }
        else if (distance < DIST_ALERT) 
        {
            myBody->ChangeState(myBody->GetMoveForwardState());
        }
        break;
    case PATTERN_IDLE:
        myBody->ChangeState(myBody->GetIdleState());
        break;

    default:
        m_actionPattern = 0;
        break;
    }

    // 行動切り替え判定（現在のパターンのタイマーだけ見る）
    bool isTimeOver = false;
    switch (m_actionPattern) 
    {
    case PATTERN_GUARD:   isTimeOver = (m_timerPattern[m_actionPattern] >= m_limitTimePattern[m_actionPattern]); break;
    case PATTERN_GB:      isTimeOver = (m_timerPattern[m_actionPattern] >= m_limitTimePattern[m_actionPattern]); break;
    case PATTERN_ATTACK:  isTimeOver = (m_timerPattern[m_actionPattern] >= m_limitTimePattern[m_actionPattern]); break;
    }

	// タイムオーバーなら次の行動を決定
    if (isTimeOver) 
    {
        DetermineNextAction(myBody);
    }
}

/**
* @brief 次の行動パターンの決定処理。乱数を使用して0~3のパターンを抽選する
*/
void Lyse::EnemyController::DetermineNextAction(Character* myBody)
{
	// HPが多い時は攻撃的、少ない時は守備的な重み付け
	if (myBody->GetHP() > HP_TRIGGER_VAL) 
    {
		// 攻撃的：攻撃系を多めに
		std::discrete_distribution<int> dist({
			1, // PATTERN_GUARD  （低め）
			2, // PATTERN_GB     （やや高め）
			3, // PATTERN_ATTACK （高め）
			3  // PATTERN_ATTACKS（高め）
			});
		m_actionPattern = dist(m_userResources->GetRandomEngine());
	}
	else 
    {
		// 守備的：ガード多め
		std::discrete_distribution<int> dist({
			4, // PATTERN_GUARD  （高め）
			1, // PATTERN_GB     （低め）
			2, // PATTERN_ATTACK （やや）
			1  // PATTERN_ATTACKS（低め）
			});
		m_actionPattern = dist(m_userResources->GetRandomEngine());
	}

	myBody->ChangeState(myBody->GetIdleState());
    ResetTimers();
}

void Lyse::EnemyController::ResetTimers()
{
    m_timerPattern.fill(0.0f);
}