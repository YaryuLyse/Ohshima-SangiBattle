#pragma once
/**
* @file TutorialUI.h
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
	class TutorialUI
	{
	public:
		// コンストラクタ：デバイスやエフェクトファクトリを受け取って初期化
		TutorialUI(DirectX::SpriteBatch& spriteBatch,ID3D11Device1* device);

		// デストラクタ
		~TutorialUI();

		// --- メインループ ---

		// 初期化処理
		void Initialize();

		// 更新処理
		// elapsedTime: 経過時間
		// position: 基準となる位置
		// rot: 基準となる回転行列
		void Update(float elapsedTime, DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix rot);

		// 描画処理
		void Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, DebugFont* debugFont,int step);

		// 終了処理
		void Finalize();

		//プレイヤーをゲットする
		void SetPlayer(Lyse::Character& player) { m_player = &player; }


		// キーボード入力のトラッカー（必要に応じて公開）
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	private:
		// 頂点数（定数定義）
		static const int VERTEX_COUNT = 4;

		// 頂点データ配列
		static const DirectX::VertexPositionTexture VERTICES[VERTEX_COUNT];

		// --- トランスフォーム情報 ---
		DirectX::SimpleMath::Vector2 m_zPosition;    // 現在の位置
		DirectX::SimpleMath::Vector2 m_xPosition;    // 現在の位置
		DirectX::SimpleMath::Vector2 m_cPosition;    // 現在の位置
		DirectX::SimpleMath::Vector2 m_compPosition;    // 現在の位置

		DirectX::SimpleMath::Vector2 m_origin;

		// 描画用バッチ（参照保持）
		DirectX::SpriteBatch* m_spriteBatch;

		// テクスチャリソース
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_zTex;      // アイコン
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_xTex;      // アイコン
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cTex;      // アイコン
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_compTex;      // アイコン

		// 制御変数
		Character* m_player; // プレイヤーへの参照（攻撃状態の取得などに使用）

		int m_currentStep = 0; // チュートリアルの現在のステップ
	};
}