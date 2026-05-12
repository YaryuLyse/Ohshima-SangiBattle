#pragma once
/**
* @file EnemyController.h
* @brief 敵のAI（意思決定）を管理するクラスのヘッダーファイル
* @brief プレイヤーとの距離や自身のステータスに応じた行動選択処理をまとめています
* @brief このクラスは、プレイヤーとの距離計算、行動パターンの抽選、タイマーによる状態遷移のトリガー、およびHP/スタミナに基づいた特殊な挙動を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "Game/GameObjects/Character/Controller.h"
#include <array>

class UserResources;
namespace Lyse
{

	class EnemyController : public Lyse::Controller
	{
	public:
		// 行動パターン数
		static constexpr int PATTERN_COUNT = 5; // 0~3 + スタン(4)

		// コンストラクタ / デストラクタ
		EnemyController();
		~EnemyController();

		//初期化処理
		void Initialize(Lyse::Character* myBody) override;

		// 更新処理
		void Update(Character* myBody, float elapsedTime) override;

		// 行動パターンのランダム設定
		void DetermineNextAction(Character* myBody); // 旧: Random

		// ゲッター / セッター
		void SetTarget(Lyse::Character* target) { m_target = target; }
		void SetActionPattern(int count) { m_actionPattern = count; }
		void SetUserResources(UserResources* userResources) { m_userResources = userResources; }
		// チュートリアル用モード
		void SetTutorialMode(bool isTutorial) { m_isTutorialMode = isTutorial; }
		void SetTutorialState(int pattern) { m_tutorialPattern = pattern; }
		int GetActionPattern() const { return m_actionPattern; }

	private:

		void ResetTimers();   // 全タイマーリセット用ヘルパー

	private:
		//ユーザーリソース管理
		UserResources* m_userResources;

		// 参照ポインタ
		Lyse::Character* m_target = nullptr; // 向く対象（敵）

		// AI制御変数
		int   m_actionPattern; // 旧: m_switchCount (0:Guard, 1:GuardBreak, 2:Attacks, 3:Attack, 4:Idle)
		int   m_debugCount;    // 旧: m_count (詳細不明だがロジック維持のため保持)

		std::array<float, PATTERN_COUNT> m_timerPattern;      // 経過タイマー
		std::array<float, PATTERN_COUNT> m_limitTimePattern;  // 閾値

		bool m_isTutorialMode = false;
		int m_tutorialPattern;

	};
}