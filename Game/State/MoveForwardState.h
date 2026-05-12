#pragma once
/**
* @file MoveForwardState.h
* @brief 敵の前進状態を管理するクラスのヘッダーファイル
* @brief 敵が前進移動を行う際の処理をまとめています
* @brief このクラスは、敵が前進状態に入ったときの移動計算や座標更新を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "StateBase.h"

namespace Lyse
{
	class Character;
}

class MoveForwardState : public StateBase
{
public:
	// コンストラクタ
	MoveForwardState(Lyse::Character* chara);
	// デストラクタ
	~MoveForwardState() override = default;

	void Initialize() override;
	void PreUpdate() override;
	void Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime) override;
	void PostUpdate() override;
	void Render() override;
	void Finalize() override;

	const wchar_t* GetName() const override { return L"MoveForward"; }

private:
	// キャラクターの管理
	Lyse::Character* m_chara;       // 敵モデルへのポインタ

	// 移動用オフセットベクトル
	DirectX::SimpleMath::Vector3 m_moveOffset{ 0.0f, 0.0f, 0.0f };
};