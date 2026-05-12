#pragma once
/**
* @file Gauge.h
* @brief HPやスタミナ等のゲージ描画を管理するクラスのヘッダーファイル
* @brief プレイヤーおよび敵のステータスを可視化するためのUI表示処理をまとめています
* @brief このクラスは、各ステータステクスチャの読み込み、現在値に基づいた描画範囲（SourceRect）の計算、およびスプライトバッチを用いたUI表示を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "LyseLib/SceneManager.h"
#include "Game/Common/UserResources.h"

#include <memory>
#include <SimpleMath.h>

namespace Lyse
{
	class Character;

	class Gauge
	{
	public:
		// コンストラクタ / デストラクタ
		Gauge(DirectX::SpriteBatch& spriteBatch, ID3D11Device1* device);
		~Gauge();

		// --- メインループ ---
		void Initialize();
		void Update(float elapsedTime);
		void Render();
		void Finalize();

		// --- 参照設定 ---
		void SetPlayer(Lyse::Character& player) { m_player = &player; }
		void SetEnemy(Lyse::Character& enemy) { m_enemy = &enemy; }

	private:
		// 描画用バッチ（参照保持）
		DirectX::SpriteBatch* m_spriteBatch;

		// テクスチャリソース
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_hpTex;      // HPゲージ（緑など）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_redTex;     // 背景/ダメージゲージ（赤）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_staminaTex; // スタミナゲージ（黄など）

		// 制御変数
		float m_posX = 0.0f; // 未使用
		float m_time = 0.0f; // 未使用

		// 外部参照
		Lyse::Character* m_player;
		Lyse::Character* m_enemy;
	};
}