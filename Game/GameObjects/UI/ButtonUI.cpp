/**
* @file ButtonUI.cpp
* @brief 敵の盾を管理するクラスの実装ファイル
* @brief 敵が装備する盾のモデル描画やエフェクト制御をまとめています
* @brief このクラスは、敵の状態（ガード中か否か）に応じた盾の回転制御や、ガード時のシールドエフェクトの表示管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "ButtonUI.h"
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
	const wchar_t* PATH_TEX_Z1 = L"Resources/Texture/zTex1.png";
	const wchar_t* PATH_TEX_Z2 = L"Resources/Texture/zTex2.png";
	const wchar_t* PATH_TEX_X1 = L"Resources/Texture/xTex1.png";
	const wchar_t* PATH_TEX_X2 = L"Resources/Texture/xTex2.png";
	const wchar_t* PATH_TEX_C1 = L"Resources/Texture/cTex1.png";
	const wchar_t* PATH_TEX_C2 = L"Resources/Texture/cTex2.png";
	const wchar_t* PATH_TEX_STUN = L"Resources/Texture/stun.png";
	const wchar_t* PATH_TEX_ARROW = L"Resources/Texture/arrow.png";

	// 描画スケール
	constexpr float DRAW_SCALE = 3.0f;
	constexpr float DRAW_SCALE_ARROW = 2.0f;

	//角度
	constexpr float LEFT_DOWN_ANGLE = 215.0f;
	constexpr float RIGHT_ANGLE = 90.0f;
	constexpr float LEFT_UP_ANGLE = 320.0f;
}


/**
* @brief コンストラクタ。デバイスの初期化やモデルのロードを行う
* @param spriteBatch : スプライトバッチ（エフェクト描画用）
* @param device : DirectXデバイス
*/
Lyse::ButtonUI::ButtonUI(DirectX::SpriteBatch& spriteBatch, ID3D11Device1* device)
	: m_spriteBatch(&spriteBatch)
	, m_player(nullptr)
	, m_zPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_xPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_cPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_arrowPosition1(DirectX::SimpleMath::Vector2::Zero)
	, m_arrowPosition2(DirectX::SimpleMath::Vector2::Zero)
	, m_arrowPosition3(DirectX::SimpleMath::Vector2::Zero)
	, m_attackPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_guardPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_guardBreakPosition(DirectX::SimpleMath::Vector2::Zero)
	, m_origin(DirectX::SimpleMath::Vector2::Zero)
	, m_angleLeftDown(0.0f)
	, m_angleLeftUp(0.0f)
	, m_angleRight(0.0f)
{
	// テクスチャ読み込み
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_Z1, nullptr, m_attackTex1.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_Z2, nullptr, m_attackTex2.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_X1, nullptr, m_guardBreakTex1.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_X2, nullptr, m_guardBreakTex2.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_C1, nullptr, m_guardTex1.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_C2, nullptr, m_guardTex2.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_STUN, nullptr, m_stunTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_ARROW, nullptr, m_arrowTex.GetAddressOf());
}

/**
* @brief デストラクタ
*/
Lyse::ButtonUI::~ButtonUI()
{
}

/**
* @brief 盾の初期化処理
*/
void Lyse::ButtonUI::Initialize()
{

	m_zPosition = DirectX::SimpleMath::Vector2(950.0f, 600.0f);
	m_xPosition = DirectX::SimpleMath::Vector2(1050.0f, 600.0f);
	m_cPosition = DirectX::SimpleMath::Vector2(1150.0f, 600.0f);

	m_arrowPosition1 = DirectX::SimpleMath::Vector2(70.0f, 550.0f);
	m_arrowPosition2 = DirectX::SimpleMath::Vector2(150.0f, 650.0f);
	m_arrowPosition3 = DirectX::SimpleMath::Vector2(230.0f, 550.0f);
	m_attackPosition = DirectX::SimpleMath::Vector2(10.0f, 600.0f);
	m_guardPosition = DirectX::SimpleMath::Vector2(100.0f, 400.0f);
	m_guardBreakPosition = DirectX::SimpleMath::Vector2(200.0f, 600.0f);

	m_origin = DirectX::SimpleMath::Vector2(
		50.0f / 2.0f,  // X座標の真ん中
		50.0f / 2.0f  // Y座標の真ん中
	);
	m_angleLeftDown = XMConvertToRadians(LEFT_DOWN_ANGLE);
	m_angleLeftUp = XMConvertToRadians(LEFT_UP_ANGLE);
	m_angleRight = XMConvertToRadians(RIGHT_ANGLE);
}

/**
* @brief 盾の更新処理（親モデルとの同期）
* @param elapsedTime : 経過時間
* @param position : 親モデルの座標
* @param rot : 親モデルの回転行列
*/
void Lyse::ButtonUI::Update(float elapsedTime, DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix rot)
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
void Lyse::ButtonUI::Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, DebugFont* debugFont)
{
	UNREFERENCED_PARAMETER(debugFont); // 現状、デバッグフォントはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(proj);   // 現状、プロジェクション行列はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(view); // 現状、ビュー行列はこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(states); // 現状、コモンステートはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(context); // 現状、デバイスコンテキストはこのクラスでは使用しないため、未使用パラメータとしてマクロで処理
	// 背景用（全幅）
	RECT srcRectFull = { 0, 0, (LONG)32, (LONG)32 };
	m_spriteBatch->Begin();
	if (m_player->GetCurrentState() == m_player->GetAttackState())
	{
		m_spriteBatch->Draw(m_attackTex2.Get(), m_zPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	}
	else
	{
		m_spriteBatch->Draw(m_attackTex1.Get(), m_zPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	}
	if (m_player->GetCurrentState() == m_player->GetGuardBreakState())
	{
		m_spriteBatch->Draw(m_guardBreakTex2.Get(), m_xPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	}
	else
	{
		m_spriteBatch->Draw(m_guardBreakTex1.Get(), m_xPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	}
	if (m_player->GetCurrentState() == m_player->GetGuardState())
	{
		m_spriteBatch->Draw(m_guardTex2.Get(), m_cPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	}
	else
	{
		m_spriteBatch->Draw(m_guardTex1.Get(), m_cPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	}

	if (m_player->GetIsStunned() == true)
	{
		m_spriteBatch->Draw(m_stunTex.Get(), m_zPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
		m_spriteBatch->Draw(m_stunTex.Get(), m_xPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
		m_spriteBatch->Draw(m_stunTex.Get(), m_cPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	}

	m_spriteBatch->Draw(m_arrowTex.Get(), m_arrowPosition1, nullptr, Colors::White, m_angleLeftDown, m_origin, DRAW_SCALE_ARROW);
	m_spriteBatch->Draw(m_arrowTex.Get(), m_arrowPosition2, nullptr, Colors::White, m_angleRight, m_origin, DRAW_SCALE_ARROW);
	m_spriteBatch->Draw(m_arrowTex.Get(), m_arrowPosition3, nullptr, Colors::White, m_angleLeftUp, m_origin, DRAW_SCALE_ARROW);
	m_spriteBatch->Draw(m_attackTex1.Get(), m_attackPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	m_spriteBatch->Draw(m_guardTex1.Get(), m_guardPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	m_spriteBatch->Draw(m_guardBreakTex1.Get(), m_guardBreakPosition, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);

	m_spriteBatch->End();
}

/**
* @brief 終了処理
*/
void Lyse::ButtonUI::Finalize()
{
}

	
