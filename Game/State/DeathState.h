#pragma once
/**
* @file DeathState.h
* @brief 敵の攻撃状態を管理するクラスのヘッダーファイル
* @brief 敵が攻撃アニメーションを行う際の処理をまとめています
* @brief このクラスは、敵が攻撃状態に入ったときのアニメーション制御や、攻撃判定の発生・消失を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "StateBase.h"

namespace Lyse
{
	class Character;
}


class DeathState : public StateBase
{
public:
	// コンストラクタ
	DeathState(Lyse::Character* chara);
	// デストラクタ
	~DeathState() override = default;

	// ステートマシンの基本メソッド群
	void Initialize() override;
	void PreUpdate() override;
	void Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime) override;
	void PostUpdate() override;
	void Render() override;
	void Finalize() override;

	// デバッグ用：ステート名を返す
	const wchar_t* GetName() const override { return L"Death"; }

private:
	// キャラクターの管理
	Lyse::Character* m_chara;       // 敵モデルへのポインタ

	// 攻撃判定を出す位置（敵の中心からのオフセット座標）
	DirectX::SimpleMath::Vector3 m_DeathOffset{ 0.0f, 0.0f, 1.0f };

	// ステートの経過時間を計測するタイマー
	float m_timer{ 0.0f };
};