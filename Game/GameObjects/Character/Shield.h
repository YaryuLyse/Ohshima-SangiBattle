#pragma once
/**
* @file Shield.h
* @brief 敵の盾を管理するクラスのヘッダーファイル
* @brief 敵が装備する盾のモデル描画やエフェクト制御をまとめています
* @brief このクラスは、敵の状態（ガード中か否か）に応じた盾の回転制御や、ガード時のシールドエフェクトの表示管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include "SimpleMath.h"
#include "Game/GameObjects/Collider/AttackCollider.h"

namespace Lyse
{
	class Character;
	class DebugFont;

	// 敵の盾（シールド）を管理・描画するクラス
	// 敵モデルに追従したり、独自のエフェクトを表示する役割を持ちます
	class Shield
	{
	public:
		// コンストラクタ：デバイスやエフェクトファクトリを受け取って初期化
		Shield(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx);

		// デストラクタ
		~Shield();

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

		// --- セッター関数 ---

		void SetPosition(DirectX::SimpleMath::Vector3 pos);

		// スケール設定（ベクトル指定）
		void SetScale(DirectX::SimpleMath::Vector3 scale);

		// スケール設定（一律指定）
		void SetScale(float scale);

		// 回転設定
		void SetRotation(DirectX::SimpleMath::Vector3 rot);

		// 敵モデルをセットする
		// 【修正】命名規則: 「動詞(Set) + 名詞(Enemy)」の順序に統一
		void SetCharacter(Character* chara) { m_character = chara; }

		// キーボード入力のトラッカー（必要に応じて公開）
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	private:
		// 頂点数（定数定義）
		static const int VERTEX_COUNT = 4;

		// 頂点データ配列
		static const DirectX::VertexPositionTexture VERTICES[VERTEX_COUNT];

		// --- トランスフォーム情報 ---
		DirectX::SimpleMath::Vector3 m_position;    // 現在の位置
		DirectX::SimpleMath::Vector3 m_prePosition; // 1フレーム前の位置
		DirectX::SimpleMath::Vector3 m_scale;       // スケール
		DirectX::SimpleMath::Vector3 m_rotation;    // 回転角度（オイラー角）

		// --- DirectX リソース ---

		// 入力レイアウト
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		// 共通ステートオブジェクトへのポインタ
		std::unique_ptr<DirectX::CommonStates> m_states;

		// テクスチャリソースビュー
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

		// アルファテストエフェクト（透過処理用）
		std::unique_ptr<DirectX::AlphaTestEffect> m_batchEffect;

		// プリミティブバッチ（形状描画用）
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_batch;

		// 3Dモデルデータ
		std::unique_ptr<DirectX::Model> m_shield;

		// --- 制御用変数 ---

		float m_rotate; // 回転制御用タイマーまたは角度

		// 位置オフセットベクトル
		// 【修正】命名規則: vector(型)ではなく offset(役割)に変更
		DirectX::SimpleMath::Vector3 m_offset;

		// 距離ベクトル
		DirectX::SimpleMath::Vector3 m_distance;

		// 敵の位置
		DirectX::SimpleMath::Vector3 m_enemyPos;

		// プレイヤーの位置
		DirectX::SimpleMath::Vector3 m_playerPos;

		// 回転行列
		DirectX::SimpleMath::Matrix m_rotMat;

		// 敵の回転行列
		DirectX::SimpleMath::Matrix m_enemyRot;

		// 敵モデルへのポインタ
		Character* m_character;

		// ドロップアニメーションのタイマー
		float m_dropTimer = 0.0f;

		// ドロップアニメーションのスケール制御
		float m_dropScale = 1.0f;

		float m_rotateZ;    // X軸回転（振る動作など）
	};
}