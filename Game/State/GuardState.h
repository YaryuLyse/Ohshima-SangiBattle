#pragma once
/**
* @file GuardState.h
* @brief 敵の防御状態を管理するクラスのヘッダーファイル
* @brief 敵が防御アクションを行う際の処理をまとめています
* @brief このクラスは、敵が防御状態に入ったときのスタミナ管理や、ガードブレイク（クラッシュ）判定を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "StateBase.h"

namespace Lyse
{
	class Character;
}

class GuardState : public StateBase
{
public:
	// コンストラクタ
	GuardState(Lyse::Character* chara);
	// デストラクタ
	~GuardState() override = default;

	void Initialize() override;
	void PreUpdate() override;
	void Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime) override;
	void PostUpdate() override;
	void Render() override;
	void Finalize() override;

	const wchar_t* GetName() const override { return L"Guard"; }

private:
	// キャラクターの管理
	Lyse::Character* m_chara;       // 敵モデルへのポインタ

};