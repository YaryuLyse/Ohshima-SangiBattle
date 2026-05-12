/**
* @file Gauge.cpp
* @brief HPやスタミナ等のゲージ描画を管理するクラスの実装ファイル
* @brief プレイヤーおよび敵のステータスを可視化するためのUI表示処理をまとめています
* @brief このクラスは、各ステータステクスチャの読み込み、現在値に基づいた描画範囲（SourceRect）の計算、およびスプライトバッチを用いたUI表示を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "Gauge.h"
#include <WICTextureLoader.h>
#include "Game/GameObjects/Character/Character.h"

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	// テクスチャファイルパス
	const wchar_t* PATH_TEX_HP = L"Resources/Texture/HP.png";
	const wchar_t* PATH_TEX_RED = L"Resources/Texture/Red.png";
	const wchar_t* PATH_TEX_STAMINA = L"Resources/Texture/Stamina.png";

	// ゲージの基本サイズ（テクスチャの元サイズと想定）
	constexpr float GAUGE_WIDTH_BASE = 1280.0f;
	constexpr float GAUGE_HEIGHT_BASE = 240.0f;

	// パラメータ最大値
	constexpr float MAX_STATUS_VALUE = 100.0f;

	// 描画スケール
	constexpr float DRAW_SCALE = 0.2f;

	// プレイヤー用表示座標
	const DirectX::SimpleMath::Vector2 POS_PLAYER_HP(50.0f, 100.0f);
	const DirectX::SimpleMath::Vector2 POS_PLAYER_STAMINA(50.0f, 150.0f);

	// 敵用表示座標
	const DirectX::SimpleMath::Vector2 POS_ENEMY_HP(1000.0f, 100.0f);
	const DirectX::SimpleMath::Vector2 POS_ENEMY_STAMINA(1000.0f, 150.0f);
}

/**
* @brief コンストラクタ。スプライトバッチの紐付けとテクスチャリソースの読み込みを行う
* @param spriteBatch : 描画に使用するSpriteBatch
* @param device : DirectXデバイス
*/
Lyse::Gauge::Gauge(SpriteBatch& spriteBatch, ID3D11Device1* device)
	: m_spriteBatch(&spriteBatch)
	, m_player(nullptr)
	, m_enemy(nullptr)
{
	// テクスチャ読み込み
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_HP, nullptr, m_hpTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_RED, nullptr, m_redTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, PATH_TEX_STAMINA, nullptr, m_staminaTex.GetAddressOf());
}

/**
* @brief デストラクタ

*/
Lyse::Gauge::~Gauge()
{
}

/**
* @brief ゲージの初期化処理
*/
void Lyse::Gauge::Initialize()
{
}

/**
* @brief ゲージの更新処理
* @param elapsedTime : 経過時間
*/
void Lyse::Gauge::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime); // 現状、経過時間はゲージの更新に使用しないため、未使用パラメータとしてマクロで処理
}

/**
* @brief ゲージの描画処理。各モデルのステータス割合に応じてテクスチャの描画範囲を切り分ける
*/
void Lyse::Gauge::Render()
{
	// 安全策：ポインタが無効なら描画しない
	if (!m_player || !m_enemy) return;

	m_spriteBatch->Begin();

	// --- 割合計算 (0.0f - 1.0f) ---
	float playerHpRate = m_player->GetHP() / MAX_STATUS_VALUE;
	float enemyHpRate = m_enemy->GetHP() / MAX_STATUS_VALUE;
	float playerStaRate = m_player->GetStamina() / MAX_STATUS_VALUE;
	float enemyStaRate = m_enemy->GetStamina() / MAX_STATUS_VALUE;

	// --- 描画矩形(SourceRect)の計算 ---
	// 幅を現在のパラメータに応じて可変にする

	// 背景用（全幅）
	RECT srcRectFull = { 0, 0, (LONG)GAUGE_WIDTH_BASE, (LONG)GAUGE_HEIGHT_BASE };
	// プレイヤーHP
	RECT srcRectPlayerHp = { 0, 0, (LONG)(GAUGE_WIDTH_BASE * playerHpRate), (LONG)GAUGE_HEIGHT_BASE };
	// 敵HP
	RECT srcRectEnemyHp = { 0, 0, (LONG)(GAUGE_WIDTH_BASE * enemyHpRate),  (LONG)GAUGE_HEIGHT_BASE };
	// プレイヤーStamina
	RECT srcRectPlayerSta = { 0, 0, (LONG)(GAUGE_WIDTH_BASE * playerStaRate),(LONG)GAUGE_HEIGHT_BASE };
	// 敵Stamina
	RECT srcRectEnemySta = { 0, 0, (LONG)(GAUGE_WIDTH_BASE * enemyStaRate), (LONG)GAUGE_HEIGHT_BASE };

	// --- プレイヤーゲージ描画 ---

	// HP (背景赤 -> 手前HP)
	m_spriteBatch->Draw(m_redTex.Get(), POS_PLAYER_HP, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	m_spriteBatch->Draw(m_hpTex.Get(), POS_PLAYER_HP, &srcRectPlayerHp, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);

	// スタミナ (背景赤 -> 手前スタミナ)
	m_spriteBatch->Draw(m_redTex.Get(), POS_PLAYER_STAMINA, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	m_spriteBatch->Draw(m_staminaTex.Get(), POS_PLAYER_STAMINA, &srcRectPlayerSta, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);


	// --- 敵ゲージ描画 ---

	// HP (背景赤 -> 手前HP)
	m_spriteBatch->Draw(m_redTex.Get(), POS_ENEMY_HP, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	m_spriteBatch->Draw(m_hpTex.Get(), POS_ENEMY_HP, &srcRectEnemyHp, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);

	// スタミナ (背景赤 -> 手前スタミナ)
	m_spriteBatch->Draw(m_redTex.Get(), POS_ENEMY_STAMINA, &srcRectFull, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);
	m_spriteBatch->Draw(m_staminaTex.Get(), POS_ENEMY_STAMINA, &srcRectEnemySta, Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero, DRAW_SCALE);

	m_spriteBatch->End();
}

/**
* @brief 終了処理
*/
void Lyse::Gauge::Finalize()
{
}