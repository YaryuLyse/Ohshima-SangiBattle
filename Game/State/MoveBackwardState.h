#pragma once
/**
* @file MoveBackwardState.h
* @brief 敵の後退状態を管理するクラスのヘッダーファイル
* @brief 敵が後退移動を行う際の処理をまとめています
* @brief このクラスは、敵が後退状態に入ったときの移動計算や座標更新を管理します。
* @author 大嶋　琉也
* @date 2026/02/20
*/

#include "StateBase.h"

namespace Lyse
{
	class Character;
}

class MoveBackwardState : public StateBase
{
public:
	// コンストラクタ
	MoveBackwardState(Lyse::Character* chara);
	// デストラクタ
	~MoveBackwardState() override = default;

	void Initialize() override;
	void PreUpdate() override;
	void Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime) override;
	void PostUpdate() override;
	void Render() override;
	void Finalize() override;

	const wchar_t* GetName() const override { return L"MoveBackward"; }
private:
	// キャラクターの管理
	Lyse::Character* m_chara;       // モデルへのポインタ

	// 移動用オフセットベクトル
	DirectX::SimpleMath::Vector3 m_moveOffset{ 0.0f, 0.0f, 0.0f };
};