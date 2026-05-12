#pragma once
/**
* @file PlayerController.h
* @brief プレイヤーの入力を管理するクラスのヘッダーファイル
* @author 大嶋　琉也
* @date 2026/02/25
*/
#include "Game/GameObjects/Character/Controller.h"

namespace Lyse
{
	class PlayerController : public Lyse::Controller
	{
	public:
		// コンストラクタ / デストラクタ
		PlayerController();
		~PlayerController();

		void Initialize(Character* myBody) override;
		// 更新処理
		void Update(Character* myBody, float elapsedTime) override;

		// ターゲットのセット
		void SetTarget(Lyse::Character* target) { m_target = target; }
	
	private:
		// キーボード入力トラッカー
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

		// ターゲット位置
		DirectX::SimpleMath::Vector3 m_targetPos;

		// ターゲットキャラクターへのポインタ
		Lyse::Character* m_target = nullptr;
	};
}