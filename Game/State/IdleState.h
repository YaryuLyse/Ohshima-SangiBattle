#pragma once
/**
* @file IdleState.h
* @brief 敵の待機状態を管理するクラスのヘッダーファイル
* @brief 敵が待機（アイドル）中に実行される処理をまとめています
* @brief このクラスは、敵が何もしていない待機状態の挙動を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "StateBase.h"

namespace Lyse
{
	class Character;
}

class IdleState : public StateBase
{
public:
	// コンストラクタ
	IdleState(Lyse::Character* chara);
	// デストラクタ
	~IdleState() override = default;

	void Initialize() override;
	void PreUpdate() override;
	void Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime) override;
	void PostUpdate() override;
	void Render() override;
	void Finalize() override;

	const wchar_t* GetName() const override { return L"Idle"; }
private:
	// キャラクターの管理
	Lyse::Character* m_chara;       // モデルへのポインタ
};