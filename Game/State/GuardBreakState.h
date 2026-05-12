#pragma once
/**
* @file GuardBreakState.h
* @brief 敵のガード崩し攻撃状態を管理するクラスのヘッダーファイル
* @brief 敵がガード崩し攻撃を行う際の処理をまとめています
* @brief このクラスは、敵がガード崩し状態に入ったときのアニメーション制御や、攻撃判定の発生・消失を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "StateBase.h"

namespace Lyse
{
	class Character;
}

class GuardBreakState : public StateBase
{
public:
	// コンストラクタ
	GuardBreakState(Lyse::Character* chara);
	// デストラクタ
	~GuardBreakState() override = default;

	void Initialize() override;
	void PreUpdate() override;
	void Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime) override;
	void PostUpdate() override;
	void Render() override;
	void Finalize() override;

	const wchar_t* GetName() const override { return L"GuardBreak"; }


private:
	// 攻撃判定の管理
	Lyse::Character* m_chara;       // 敵モデルへのポインタ

	// 攻撃判定のオフセット位置
	DirectX::SimpleMath::Vector3 m_attackOffset{ 0.0f, 0.0f, 1.0f };

	// タイマー
	float m_timer{ 0.0f };
};