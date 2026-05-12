#pragma once
/**
* @file StateBase.h
* @brief プレイヤーや敵の行動を管理するステートマシンの基底クラスのヘッダーファイル
* @author 大嶋　琉也
* @date 2026/02/17
*/

class StateBase
{
public:
	// 初期化する
	virtual void Initialize() = 0;

	// 初期化後に呼ばれる
	virtual void PreUpdate() = 0;
	
	// 処理を実行する
	virtual void Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime) = 0;

	// 処理後に呼ばれる
	virtual void PostUpdate() = 0;

	// テクスチャを描画する
	virtual void Render() = 0;

	// 後始末をする
	virtual void Finalize() = 0;

	// 仮想デストラクタ
	virtual ~StateBase() = default;

	// 状態の名前を取得する
	virtual const wchar_t* GetName() const = 0;
};