#pragma once
/**
* @file ButtonUI.h
* @brief 敵の盾を管理するクラスのヘッダーファイル
* @brief 敵が装備する盾のモデル描画やエフェクト制御をまとめています
* @brief このクラスは、敵の状態（ガード中か否か）に応じた盾の回転制御や、ガード時のシールドエフェクトの表示管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include "SimpleMath.h"
#include <SpriteBatch.h>
#include <DirectXColors.h>


namespace Lyse
{
	class Character;
	class DebugFont;

	// 敵の盾（シールド）を管理・描画するクラス
	// 敵モデルに追従したり、独自のエフェクトを表示する役割を持ちます
	class ButtonUI
	{
	public:
		// コンストラクタ：デバイスやエフェクトファクトリを受け取って初期化
		ButtonUI(DirectX::SpriteBatch& spriteBatch,ID3D11Device1* device);

		// デストラクタ
		~ButtonUI();

		// --- メインループ ---

		// 初期化処理
		void Initialize();

		// 更新処理
		// elapsedTime: 経過時間
		// position: 基準となる位置
		// rot: 基準となる回転行列
		void Update(float elapsedTime, DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix rot);

		// 描画処理
		void Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, DebugFont* debugFont);

		// 終了処理
		void Finalize();

		//プレイヤーをセットする
		void SetPlayer(Lyse::Character& player) { m_player = &player; }

		// キーボード入力のトラッカー（必要に応じて公開）
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	private:

		// --- トランスフォーム情報 ---
		DirectX::SimpleMath::Vector2 m_zPosition;    // 現在の位置
		DirectX::SimpleMath::Vector2 m_xPosition;    // 現在の位置
		DirectX::SimpleMath::Vector2 m_cPosition;    // 現在の位置

		DirectX::SimpleMath::Vector2 m_attackPosition;
		DirectX::SimpleMath::Vector2 m_guardPosition;
		DirectX::SimpleMath::Vector2 m_guardBreakPosition;
		DirectX::SimpleMath::Vector2 m_arrowPosition1;
		DirectX::SimpleMath::Vector2 m_arrowPosition2;
		DirectX::SimpleMath::Vector2 m_arrowPosition3;

		DirectX::SimpleMath::Vector2 m_origin;

		// 描画用バッチ（参照保持）
		DirectX::SpriteBatch* m_spriteBatch;

		// テクスチャリソース
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_attackTex1;      // 攻撃アイコン（反応なし）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_attackTex2;      // 攻撃アイコン（反応あり）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_attacksTex1;     // 攻撃アイコン（反応なし）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_attacksTex2;     // 攻撃アイコン（反応あり）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_guardBreakTex1;     // ガードブレイクアイコン（反応なし）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_guardBreakTex2;     // ガードブレイクアイコン（反応あり）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_guardTex1;     // ガードアイコン（反応なし）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_guardTex2;     // ガードアイコン（反応あり）
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_stunTex;     // スタンアイコン
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_arrowTex;     // 矢印アイコン

		// 135度をラジアンに変換する
		float m_angleLeftDown;
		float m_angleRight;
		float m_angleLeftUp;
		// 制御変数
		Character* m_player; // プレイヤーへの参照（攻撃状態の取得などに使用）
	};
}