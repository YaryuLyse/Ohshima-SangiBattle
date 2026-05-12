/**
* @file TutorialUI.cpp
* @brief 敵の盾を管理するクラスの実装ファイル
* @brief 敵が装備する盾のモデル描画やエフェクト制御をまとめています
* @brief このクラスは、敵の状態（ガード中か否か）に応じた盾の回転制御や、ガード時のシールドエフェクトの表示管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "TutorialUI.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/State/GuardState.h"
#include "Game/State/GuardBreakState.h"
#include "Game/State/AttackState.h"
#include "Game/State/DeathState.h"
#include "LyseLib/DebugFont.h"

// DirectXTK
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>

using namespace DirectX;

namespace
{
	// --- 定数定義 ---
	// テクスチャファイルパス
	const wchar_t* PATH_TEX_Z = L"Resources/Texture/Zkey.png";
	const wchar_t* PATH_TEX_C = L"Resources/Texture/Ckey.png";
	const wchar_t* PATH_TEX_X = L"Resources/Texture/Xkey.png";
	const wchar_t* PATH_TEX_COMP = L"Resources/Texture/comp.png";
	

	// 描画スケール
	constexpr float DRAW_SCALE = 3.0f;
	constexpr float DRAW_SCALE_ARROW = 2.0f;

	//チュートリアルステップ	
	constexpr int ATTACK = 0;	// 攻撃してみよう
	constexpr int GUARD = 1;	 // ガードしてみよう
	constexpr int GUARD_BREAK = 2; // ガードブレイクしてみよう
	constexpr int COMPLETE = 3; // チュートリアル完了

}



/**
* @brief コンストラクタ。デバイスの初期化やモデルのロードを行う
* @param spriteBatch : スプライトバッチ（エフェクト描画用）
* @param device : DirectXデバイス
*/
Lyse::TutorialUI::TutorialUI(DirectX::SpriteBatch& spriteBatch, ID3D11Device1* device)
	: m_spriteBatch(&spriteBatch)
	, m_currentStep(0)
	, m_zPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_cPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_xPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_compPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_player(nullptr)
	, m_origin(DirectX::SimpleMath::Vector2::Zero)
	, m_zTex(nullptr)
	, m_cTex(nullptr)
	, m_xTex(nullptr)
	, m_compTex(nullptr)
{
	// テクスチャ読み込み
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_Z, nullptr, m_zTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_C, nullptr, m_cTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_X, nullptr, m_xTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_COMP, nullptr, m_compTex.GetAddressOf());

}

/**
* @brief デストラクタ
*/
Lyse::TutorialUI::~TutorialUI()
{
}

/**
* @brief 盾の初期化処理
*/
void Lyse::TutorialUI::Initialize()
{
	m_zPosition = DirectX::SimpleMath::Vector2(300.0f, 0.0f);
	m_cPosition = DirectX::SimpleMath::Vector2(300.0f, 0.0f);
	m_xPosition = DirectX::SimpleMath::Vector2(300.0f, 0.0f);
	m_compPosition = DirectX::SimpleMath::Vector2(300.0f, 0.0f);

	m_origin = DirectX::SimpleMath::Vector2(
		50.0f / 2.0f,  // X座標の真ん中
		50.0f / 2.0f  // Y座標の真ん中
	);
}

/**
* @brief 盾の更新処理（親モデルとの同期）
* @param elapsedTime : 経過時間
* @param position : 親モデルの座標
* @param rot : 親モデルの回転行列
*/
void Lyse::TutorialUI::Update(float elapsedTime, DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix rot)
{
	UNREFERENCED_PARAMETER(elapsedTime); // 現状、経過時間はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(position);    // 現状、親モデルの座標はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(rot);         // 現状、親モデルの回転はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
}

/**
* @brief 盾およびガードエフェクトの描画処理
* @param context : デバイスコンテキスト
* @param states : コモンステート
* @param view : ビュー行列
* @param proj : プロジェクション行列
* @param debugFont : デバッグフォント描画用クラス
*/
void Lyse::TutorialUI::Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, DebugFont* debugFont, int step)
{
	UNREFERENCED_PARAMETER(debugFont); // 現状、デバッグフォントはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(context);   // 現状、デバイスコンテキストはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(states);    // 現状、コモンステートはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(view);      // 現状、ビュー行列はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(proj);      // 現状、プロジェクション行列はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	m_currentStep = step; // チュートリアルの現在のステップを更新

	// 座標が 640, 600 になっているか確認
	std::wstring debugPos = L"Pos: " + std::to_wstring(m_zPosition.y) + L"\n";
	OutputDebugString(debugPos.c_str());
	// 背景用（全幅）
	RECT srcRectFull = { 0, 0, (LONG)32, (LONG)32 };

	// ステップに応じたテクスチャを描画
	m_spriteBatch->Begin();

	if (m_currentStep == ATTACK)
	{
		m_spriteBatch->Draw(m_zTex.Get(), m_zPosition, nullptr, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, 1.0f);
	}
	else if (m_currentStep == GUARD)
	{
		m_spriteBatch->Draw(m_cTex.Get(), m_cPosition, nullptr, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, 1.0f);
	}
	else if (m_currentStep == GUARD_BREAK)
	{
		m_spriteBatch->Draw(m_xTex.Get(), m_xPosition, nullptr, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, 1.0f);
	}
	else if (m_currentStep == COMPLETE)
	{
		m_spriteBatch->Draw(m_compTex.Get(), m_compPosition, nullptr, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, 1.0f);
	}

	m_spriteBatch->End();
}

/**
* @brief 終了処理
*/
void Lyse::TutorialUI::Finalize()
{
}

