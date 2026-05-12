/**
* @file Sword.h
* @brief 敵の武器（剣）を管理するクラスのヘッダーファイル
* @brief 敵が装備する剣のモデル描画や攻撃時のアニメーション制御をまとめています
* @brief このクラスは、敵の身体の回転に同期した位置計算や、攻撃ステートに応じたスイング動作（回転）の更新を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#pragma once
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include "SimpleMath.h"

namespace Lyse
{
	class Character;
	class DebugFont;

	class Sword
	{
	public:
		// コンストラクタ
		Sword(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx);
		// デストラクタ
		~Sword();

		void Initialize();
		void Update(float elapsedTime, DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix rot);
		void Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, DebugFont* debugFont);
		void Finalize();

		// --- セッター関数 ---

		void SetPosition(DirectX::SimpleMath::Vector3 pos);
		void SetScale(DirectX::SimpleMath::Vector3 scale);
		void SetScale(float scale);
		void SetRotation(DirectX::SimpleMath::Vector3 rot);

		// 敵モデルをセットする
		// 【修正】命名規則: 「動詞(Set) + 名詞(Enemy)」の順序に統一
		void SetCharacter(Character* chara) { m_character = chara; }

		// キーボード入力のトラッカー
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	private:
		// 頂点数（定数定義）
		static const int VERTEX_COUNT = 4;

		// 頂点データ配列
		static const DirectX::VertexPositionTexture VERTICES[VERTEX_COUNT];

		// --- トランスフォーム情報 ---
		DirectX::SimpleMath::Vector3 m_position;    // 位置
		DirectX::SimpleMath::Vector3 m_prePosition; // 以前の位置
		DirectX::SimpleMath::Vector3 m_scale;       // スケール
		DirectX::SimpleMath::Vector3 m_rotation;    // 回転（オイラー角）

		// --- DirectX リソース ---

		// 入力レイアウト
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		// 共通ステートオブジェクトへのポインタ
		std::unique_ptr<DirectX::CommonStates> m_states;

		// テクスチャリソースビュー
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

		// アルファテストエフェクト
		std::unique_ptr<DirectX::AlphaTestEffect> m_batchEffect;

		// プリミティブバッチ
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_batch;

		// 3Dモデル
		std::unique_ptr<DirectX::Model> m_sword;

		// --- 制御用変数 ---

		float m_rotate;     // 全体的な回転制御
		float m_rotateX;    // X軸回転（振る動作など）
		float m_rotateZ;    // X軸回転（振る動作など）

		// 位置オフセット
		// 【修正】命名規則: vector(型)ではなく offset(役割)に変更
		DirectX::SimpleMath::Vector3 m_offset;

		// 距離ベクトル
		DirectX::SimpleMath::Vector3 m_distance;

		// 敵の位置
		DirectX::SimpleMath::Vector3 m_charaPos;

		// プレイヤーの位置
		DirectX::SimpleMath::Vector3 m_playerPos;

		// 回転行列
		DirectX::SimpleMath::Matrix m_rotMat;

		// 敵の回転行列
		DirectX::SimpleMath::Matrix m_charaRot;

		//攻撃のタイマー
		float m_attackTimer;
		
		//攻撃のタイマー２
		float m_attacksTimer;

		//ガードブレイクのタイマー
		float m_guardBreakTimer;

		//突き出しのオフセット
		DirectX::SimpleMath::Vector3 m_thrustOffset;

		//突き出しの距離
		float m_thrustDistance;

		// 敵モデルへのポインタ
		Character* m_character;

		// ドロップアニメーションのタイマー
		float m_dropTimer = 0.0f;

		// ドロップアニメーションのスケール制御
		float m_dropScale = 1.0f;
	};
}