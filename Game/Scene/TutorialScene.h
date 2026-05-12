#pragma once
/**
* @file TutorialScene.h
* @brief チュートリアル画面を管理するクラスのヘッダーファイル
* @brief 操作説明の表示やゲームルールの提示処理をまとめています
* @brief このクラスは、チュートリアル用リソースのロード、説明用テキストの描画、およびタイトル画面へ戻るための入力制御を管理します。
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
	class TutorialUI;
	class PlayerController;
	class EnemyController;
	class Controller;
	class CollisionManager;
	class StageCollision;
	class StageBreakCollision;
	class ShaderManager;
	class ShaderManagerParticle;
	class ShaderManagerEffect;

	class TutorialScene : public Scene<UserResources>
	{
	public:
		enum class TutorialStep
		{
			ATTACK,       // 攻撃してみよう
			GUARD,        // ガードしてみよう
			GUARD_BREAK,  // ガードブレイクしてみよう
			COMPLETE      // チュートリアル完了
		};

		TutorialScene();
		 
		~TutorialScene();

		// 初期化
		void Initialize() override;

		// 更新
		void Update(float elapsedTime) override;

		// 描画
		void Render() override;

		// 終了処理
		void Finalize() override;

		// デバイスに依存するリソースを作成する関数
		void CreateDeviceDependentResources() override;

		// ウインドウサイズに依存するリソースを作成する関数
		void CreateWindowSizeDependentResources() override;

		// デバイスロストした時に呼び出される関数
		void OnDeviceLost() override;

		// チュートリアル用セッター
		void SetAttackHit(bool hit) { m_attackSuccess = hit; }
		void SetGuardSuccess(bool success) { m_guardSuccess = success; }
		void SetGuardBreakSuccess(bool success) { m_guardBreakSuccess = success; }
	private:
		std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TitleTex;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ArrowTex;
		float m_posX = 0.0f;
		float m_time = 0.0f;
		// キーボード入力トラッカー
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

		// 入力ロックのフラグ
		bool m_isInputLocked = true;

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
		std::unique_ptr<Lyse::TutorialUI>     m_tutorialUI;


		// --- システム・マネージャー ---
		std::unique_ptr<Lyse::CollisionManager> m_collisionManager;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
		std::unique_ptr<DirectX::GeometricPrimitive> m_debugSphere;

		// --- シェーダー ---
		std::unique_ptr<Lyse::ShaderManager>         m_shaderManager;
		std::unique_ptr<Lyse::ShaderManagerParticle> m_shaderManagerParticle;
		std::unique_ptr<Lyse::ShaderManagerEffect>   m_shaderManagerEffect;

		// --- その他 ---
		float m_elapsedTime;

		// --- チュートリアルステップ管理 ---
		TutorialStep m_currentStep;

		bool m_attackSuccess = false; // 攻撃がヒットしたかどうかのフラグ
		bool m_guardSuccess = false; // ガードが成功したかどうかのフラグ
		bool m_guardBreakSuccess = false; // ガードブレイクが成功したかどうかのフラグ

		// EnemyControllerへのポインタ（必要に応じて使用）
		Lyse::EnemyController* m_enemyCtrlPtr = nullptr;

	};
}

