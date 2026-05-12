//--------------------------------------------------------------------------------------
// File: UserResources.h
//
// シーンへ渡すユーザーリソースクラス（サンプル）
//
// Date: 2025.2.28
//--------------------------------------------------------------------------------------
#pragma once

#include "StepTimer.h"
#include "DeviceResources.h"
#include "LyseLib/DebugFont.h"
#include <random>

// 各シーンに渡す共通リソースを記述してください
class UserResources
{
private:

	// ステップタイマー
	DX::StepTimer* m_timer;

	// デバイスリソース
	DX::DeviceResources* m_deviceResources;

	// キーボードステートトラッカー
	DirectX::Keyboard::KeyboardStateTracker* m_keyboardTracker;

	// マウスステートトラッカー
	DirectX::Mouse::ButtonStateTracker* m_mouseTracker;

	// デバッグ用文字列表示へのポインタ
	Lyse::DebugFont* m_debugFont;

	// 共通ステート
	DirectX::CommonStates* m_states;

	// 勝敗フラグ
	bool isWin;

	// ランダムエンジン
	std::mt19937 m_randomEngine;

public:

	// コンストラクタ
	UserResources()
		: m_timer(nullptr)
		, m_deviceResources(nullptr)
		, m_keyboardTracker(nullptr)
		, m_mouseTracker(nullptr)
		, m_debugFont(nullptr)
		, m_states(nullptr)
		, isWin(false)
		, m_randomEngine(std::random_device{}())
	{
	}

	// ------------------------------------------ //
	// ステップタイマー	
	// ------------------------------------------ //

	// ステップタイマーを設定する関数
	void SetStepTimerStates(DX::StepTimer* timer) { m_timer = timer; }

	// ステップタイマーを取得する関数
	DX::StepTimer* GetStepTimer() { return m_timer; }

	// ------------------------------------------ //
	// デバイスリソース	
	// ------------------------------------------ //

	// デバイスリソースを設定する関数
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }

	// デバイスリソースを取得する関数
	DX::DeviceResources* GetDeviceResources() { return m_deviceResources; }

	// ------------------------------------------ //
	// キーボードステートトラッカー	
	// ------------------------------------------ //

	// キーボードステートトラッカーを設定する関数
	void SetKeyboardStateTracker(DirectX::Keyboard::KeyboardStateTracker* tracker) { m_keyboardTracker = tracker; }

	// キーボードステートトラッカーを取得する関数
	DirectX::Keyboard::KeyboardStateTracker* GetKeyboardStateTracker() { return m_keyboardTracker; }

	// ------------------------------------------ //
	// マウスステートトラッカー	
	// ------------------------------------------ //

	// マウスステートトラッカーを設定する関数
	void SetMouseStateTracker(DirectX::Mouse::ButtonStateTracker* tracker) { m_mouseTracker = tracker; }

	// マウスステートトラッカーを取得する関数
	DirectX::Mouse::ButtonStateTracker* GetMouseStateTracker() { return m_mouseTracker; }

	// ------------------------------------------ //
	// 共通ステート	
	// ------------------------------------------ //

	// 共通ステートを設定する関数
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

	// 共通ステートを取得する関数
	DirectX::CommonStates* GetCommonStates() { return m_states; }

	// ------------------------------------------ //
	// デバッグフォント	
	// ------------------------------------------ //

	// デバッグフォントを設定する関数
	void SetDebugFont(Lyse::DebugFont* debugFont) { m_debugFont = debugFont; }

	// デバッグフォントを取得する関数
	Lyse::DebugFont* GetDebugFont() { return m_debugFont; }

	// ------------------------------------------ //
	// 勝敗フラグ
	// ------------------------------------------ //
	
	// 勝敗フラグを設定する関数
	void SetIsWin(bool win) { isWin = win; }
	
	// 勝敗フラグを取得する関数
	bool GetIsWin() { return isWin; }

	// ------------------------------------------ //
	// ランダム関数
	// ------------------------------------------ //

	// 好きな範囲の「小数」をランダムで取得する
	float GetRandomFloat(float min, float max)
	{
		std::uniform_real_distribution<float> dist(min, max);
		return dist(m_randomEngine);
	}

	// 好きな範囲の「整数」をランダムで取得する
	int GetRandomInt(int min, int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(m_randomEngine);
	}

	// ランダムエンジンを直接取得する関数（必要に応じて公開）
	std::mt19937& GetRandomEngine() { return m_randomEngine; }

};

