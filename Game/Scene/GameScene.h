#pragma once
/**
* @file GameScene.h
* @brief メインゲームシーン（テストシーン）を管理するクラスのヘッダーファイル
* @brief ゲームのメインループ、オブジェクトの更新・描画、衝突判定の実行をまとめています
* @brief このクラスは、プレイヤー、敵、ステージ、カメラ、各マネージャーの生成と初期化を行い、ゲームプレイ中の相互作用やシーン遷移（勝利・敗北判定）を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/


#include <memory>
#include <array>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <GeometricPrimitive.h>

#include "LyseLib/SceneManager.h"
#include "Game/Common/UserResources.h"

namespace Lyse
{
	class AttackCollider;
	class GuardBreakCollider;
	class TPSCamera;
	class FloorModel;
	class Stage;
	class Stage;
	class Skydome;
	class Gauge;
	class Character;
	class ButtonUI;
	class PlayerController;
	class EnemyController;
	class Controller;
	class CollisionManager;
	class StageCollision;
	class StageBreakCollision;
	class ShaderManager;
	class ShaderManagerParticle;
	class ShaderManagerEffect;

	enum class GamePhase 
	{
		INTRO,  // イントロ（カメラ周回）
		GAME,   // ゲーム中
	};
	class GameScene : public Lyse::Scene<UserResources>
	{
	public:

		// コンストラクタ
		GameScene();
		// --- シーンライフサイクル ---
		~GameScene();

		// 初期化
		void Initialize() override;

		// 更新
		void Update(float elapsedTime) override;

		// 描画
		void Render() override;

		// 終了処理
		void Finalize() override;

		// --- リソース管理 ---

		// デバイス依存リソースの作成
		void CreateDeviceDependentResources() override;

		// ウィンドウサイズ依存リソースの作成
		void CreateWindowSizeDependentResources() override;

		// デバイスロスト時の処理
		void OnDeviceLost() override;

		// --- イベントコールバック ---

		// 攻撃ヒット時
		void IsAttackHit(bool isHit, DirectX::SimpleMath::Vector3 pos, bool isGuard);

		// ノックバック時
		void IsKnockBack(bool isKnock, DirectX::SimpleMath::Vector3 pos);

	private:
		// --- 描画用リソース ---
		std::unique_ptr<DirectX::BasicEffect> m_batchEffect;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_batchInputLayout;
		// --- カメラ・行列 ---
		std::unique_ptr<Lyse::TPSCamera> m_tpsCamera;
		DirectX::SimpleMath::Matrix m_proj;
		DirectX::SimpleMath::Matrix m_view;

		// --- キャラクター ---
		std::unique_ptr<Lyse::Character> m_player;
		std::unique_ptr<Lyse::Character> m_enemy;
		std::unique_ptr<Lyse::PlayerController>   m_playerController;
		std::unique_ptr<Lyse::EnemyController>    m_enemyController;

		// --- 攻撃オブジェクト ---
		std::unique_ptr<Lyse::AttackCollider>  m_playerAttack;
		std::unique_ptr<Lyse::AttackCollider>  m_enemyAttack;
		std::unique_ptr<Lyse::GuardBreakCollider> m_playerGuardBreak;
		std::unique_ptr<Lyse::GuardBreakCollider> m_enemyGuardBreak;

		// --- ステージ・環境 ---
		std::unique_ptr<Lyse::FloorModel>   m_floorModel;
		std::unique_ptr<Lyse::Stage>        m_stage;
		std::unique_ptr<Lyse::Skydome>      m_skydome;

		// ステージ当たり判定
		std::unique_ptr<Lyse::StageCollision> m_stageCollision;

		// --- UI・アイテム ---
		std::unique_ptr<DirectX::Model> m_diceModel;
		std::unique_ptr<DirectX::Model> m_swordModel;
		std::unique_ptr<DirectX::Model> m_shieldModel;
		std::unique_ptr<Lyse::Gauge>          m_gauge;
		std::unique_ptr<Lyse::ButtonUI>       m_buttonUI;


		// --- システム・マネージャー ---
		std::unique_ptr<Lyse::CollisionManager> m_collisionManager;
		std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
		std::unique_ptr<DirectX::GeometricPrimitive> m_debugSphere;

		// --- シェーダー ---
		std::unique_ptr<Lyse::ShaderManager>         m_shaderManager;
		std::unique_ptr<Lyse::ShaderManagerParticle> m_shaderManagerParticle;
		std::unique_ptr<Lyse::ShaderManagerEffect>   m_shaderManagerEffect;

		// --- その他 ---
		float m_elapsedTime;

		// --- ゲームフェーズ管理 ---
		GamePhase m_gamePhase;
		float m_introTimer = 0.0f;
		constexpr static float INTRO_DURATION = 3.0f; // 一周にかける時間

		// キーボード入力トラッカー
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

		// 入力ロックのフラグ
		bool m_isInputLocked = true;
	};
}