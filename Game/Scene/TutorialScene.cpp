/**
* @file TutorialScene.cpp
* @brief チュートリアル画面を管理するクラスの実装ファイル
* @brief 操作説明の表示やゲームルールの提示処理をまとめています
* @brief このクラスは、チュートリアル用リソースのロード、説明用テキストの描画、およびタイトル画面へ戻るための入力制御を管理します。
* @author 大嶋　琉也
* @date 2026/04/26
*/

#include "pch.h"
#include "TutorialScene.h"
#include "TitleScene.h"

#include "Game/State/GuardState.h"
#include "Game/State/AttackState.h"
#include "Game/State/GuardBreakState.h"
// ゲームオブジェクト
#include "Game/GameObjects/Character/Controller.h"
#include "Game/GameObjects/Collider/AttackCollider.h"
#include "Game/GameObjects/Collider/GuardBreakCollider.h"
#include "Game/GameObjects/Stage/FloorModel.h"
#include "Game/GameObjects/Stage/Stage.h"
#include "Game/GameObjects/Stage/Skydome.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/GameObjects/Camera/TPSCamera.h"
#include "Game/GameObjects/Character/PlayerController.h"
#include "Game/GameObjects/Character/EnemyController.h"
#include "Game/GameObjects/UI/Gauge.h"
#include "Game/GameObjects/UI/ButtonUI.h"
#include "Game/GameObjects/UI/TutorialUI.h"

// --- マネージャー・判定 ---
#include "Game/GameObjects/Collider/CollisionManager.h"
#include "Game/GameObjects/Collider/StageCollision.h"
#include "Game/GameObjects/Collider/StageBreakCollision.h"

// --- シェーダー ---
#include "Game/GameObjects/Shader/ShaderManager.h"
#include "Game/GameObjects/Shader/ShaderManagerParticle.h"
#include "Game/GameObjects/Shader/ShaderManagerEffect.h"

namespace
{
	// --- 定数定義 ---
	const wchar_t* MODEL_DIRECTORY = L"Resources/Models";

	// ステージ判定パラメータ
	const DirectX::SimpleMath::Vector3 STAGE_CENTER(0.0f, 0.5f, 0.0f);
	const float STAGE_RADIUS = 14.0f;

	// カメラパラメータ
	const float FOV_ANGLE = 45.0f;
	const float NEAR_PLANE = 0.1f;
	const float FAR_PLANE = 1000.0f;

	// 文字列バッファサイズ
	const int DEBUG_STR_LEN = 64;

	// チュートリアルステップ
	const int PATTERN_IDLE = 4;
	const int PATTERN_ATTACK = 2;
	const int PATTERN_GUARD = 0;
}

/**
* @brief コンストラクタ
*/

Lyse::TutorialScene::TutorialScene()
	:m_currentStep(TutorialStep::ATTACK)
	,m_isInputLocked(true)
	, m_elapsedTime(0.0f)
{
}

/**
* @brief デストラクタ
*/
Lyse::TutorialScene::~TutorialScene() = default;

/**
* @brief チュートリアルシーンの初期化処理
*/
void Lyse::TutorialScene::Initialize()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();

	// エフェクトファクトリ
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(MODEL_DIRECTORY);

	// プレイヤーの生成
	m_playerController = std::make_unique<Lyse::PlayerController>();
	auto playerCtrlPtr = m_playerController.get(); // ← 先に生ポインタを取る
	m_player = std::make_unique<Lyse::Character>(device, *fx, std::move(m_playerController), Lyse::PLAYER);

	// 敵の生成
	m_enemyController = std::make_unique<Lyse::EnemyController>();
	auto enemyCtrlPtr = m_enemyController.get(); // ← 先に生ポインタを取る
	m_enemyCtrlPtr = enemyCtrlPtr;
	m_enemy = std::make_unique<Lyse::Character>(device, *fx, std::move(m_enemyController), Lyse::ENEMY);
	enemyCtrlPtr->SetUserResources(GetUserResources());

	m_player->Initialize();
	m_player->SetTutorialScene(this);
	m_enemy->Initialize();
	m_enemy->SetTutorialScene(this);

	// コントローラーに対象をセット
	playerCtrlPtr->SetTarget(m_enemy.get());
	enemyCtrlPtr->SetTarget(m_player.get());

	//それぞれ対象にセット
	m_player->SetOpponent(*m_enemy);
	m_enemy->SetOpponent(*m_player);

	// 初期位置を別々に設定
	m_player->SetPosition({ -3.0f, 0.0f, 0.0f });
	m_enemy->SetPosition({ 3.0f, 0.0f, 0.0f });

	m_playerAttack = std::make_unique<Lyse::AttackCollider>();
	m_enemyAttack = std::make_unique<Lyse::AttackCollider>();
	m_playerGuardBreak = std::make_unique<Lyse::GuardBreakCollider>();
	m_enemyGuardBreak = std::make_unique<Lyse::GuardBreakCollider>();

	// ステージ・環境生成
	m_stage = std::make_unique<Lyse::Stage>(device, *fx);
	m_skydome = std::make_unique<Lyse::Skydome>(device, *fx);
	m_stageCollision = std::make_unique<Lyse::StageCollision>(STAGE_CENTER, STAGE_RADIUS);

	m_collisionManager = std::make_unique<Lyse::CollisionManager>();

	// デバッグカメラ
	RECT rect = GetUserResources()->GetDeviceResources()->GetOutputSize();
	m_tpsCamera = std::make_unique<Lyse::TPSCamera>(rect.right, rect.bottom);

	// シェーダーマネージャー生成
	m_shaderManager = std::make_unique<Lyse::ShaderManager>();
	m_shaderManagerParticle = std::make_unique<Lyse::ShaderManagerParticle>();
	m_shaderManagerEffect = std::make_unique<Lyse::ShaderManagerEffect>();

	// 初期化 (Create -> Initialize に変更)
	m_shaderManager->Initialize(GetUserResources()->GetDeviceResources());
	m_shaderManagerParticle->Initialize(GetUserResources()->GetDeviceResources(),GetUserResources());
	m_shaderManagerEffect->Initialize(GetUserResources()->GetDeviceResources());

	// 依存関係の注入
	m_tpsCamera->SetTarget(*m_enemy);
	m_tpsCamera->SetPosition(*m_player);

	m_player->SetShaderManagerEffect(*m_shaderManagerEffect);
	m_enemy->SetShaderManagerEffect(*m_shaderManagerEffect);

	if (m_playerAttack != nullptr)
	{
		m_player->SetAttackCollider(*m_playerAttack);
	}
	if (m_enemyAttack != nullptr)
	{
		m_enemy->SetAttackCollider(*m_enemyAttack);
	}
	if (m_playerGuardBreak != nullptr)
	{
		m_player->SetGuardBreakCollider(*m_playerGuardBreak);
	}
	if (m_enemyGuardBreak != nullptr)
	{
		m_enemy->SetGuardBreakCollider(*m_enemyGuardBreak);
	}

	// 初期化実行
	m_stageCollision->SetStage(*m_stage);
	m_stageCollision->SetPlayer(*m_player);
	m_stageCollision->SetEnemy(*m_enemy);

	// シーン初期化時にセットしているか？
	for (auto& col : m_stage->GetBreakCollisions()) {
		col.SetPlayer(*m_player);
		col.SetEnemy(*m_enemy);
	}

	m_stage->Initialize(m_shaderManager.get());
	m_skydome->Initialize(m_shaderManager.get());
	m_tutorialUI->Initialize();

	m_elapsedTime = 0.0f;
	m_isInputLocked = true; // シーン開始時離すまでロック

	m_currentStep = TutorialStep::ATTACK;
	m_enemyCtrlPtr->SetTutorialMode(true);
	m_enemyCtrlPtr->SetTutorialState(PATTERN_IDLE); // 最初は待機
}

/**
* @brief チュートリアルシーンの更新処理。入力によるシーン遷移や、UIのアニメーション用タイマー更新を行います
* @param elapsedTime : 経過時間
*/
void Lyse::TutorialScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	auto kb = DirectX::Keyboard::Get().GetState();
	m_keyboardTracker.Update(kb);
	// 現在のキーボード（またはパッド）の状態を取得
	// （DirectInputやGetKeyboardStateなど）
	bool isAttackButtonPressed = kb.Z;

	// ロック状態の処理
	if (m_isInputLocked) 
	{
		if (!isAttackButtonPressed) 
		{
			// ボタンが離されたのを確認したらロック解除！
			m_isInputLocked = false;
		}
		return; // ロック中はここで処理を終わらせて、攻撃させない
	}

	// 各更新
	m_tpsCamera->Update();
	m_player->Update(elapsedTime);
	m_enemy->Update(elapsedTime);

	m_shaderManagerParticle->Update(elapsedTime);
	m_skydome->SetPlayerPosition(m_player->GetPosition());

	// ビルボード設定
	m_shaderManagerParticle->CreateBillboard(
		m_tpsCamera->GetTargetPosition(),
		m_tpsCamera->GetEyePosition(),
		m_tpsCamera->GetUpVector()
	);

	m_stageCollision->Update();
	m_stage->Update(elapsedTime);

	// --- 衝突判定の登録と実行 ---
	m_collisionManager->Clear();

	// 壊れる壁の登録
	auto& breakCollisions = m_stage->GetBreakCollisions();
	for (auto& col : breakCollisions)
	{
		if (col.IsActive())
		{
			// StageBreakObjects -> AddStageBreakObject
			m_collisionManager->AddStageBreakObject(col);
		}
	}

	// GameScene::Update に追加
	// ラムダ関数：1キャラクター分の処理
	auto clampCharacter = [&](Lyse::Character* chara) 
	{
		DirectX::SimpleMath::Vector3 totalPush =
		DirectX::SimpleMath::Vector3::Zero; // 押し出しの合計
		bool isHit = false; // どこかに当たったか

		// 全壊れ壁をループ
		for (auto& col : m_stage->GetBreakCollisions()) 
		{
			if (!col.IsActive()) continue; // 非アクティブはスキップ

			DirectX::SimpleMath::Vector3 pos = chara->GetPosition();

			// キャラと壊れ壁の距離を計算
			DirectX::SimpleMath::Vector3 diff = pos - col.GetPosition();
			float dist = diff.Length();

			// 壊れ壁の判定範囲内に入っているか
			if (dist < col.GetRadius() + 0.2f)
			{
				// ゼロ除算防止
				if (dist <= 0.001f) 
				{
					diff = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f);
					dist = 1.0f;
				}
				// 押し出す方向（壁の中心からキャラへの方向）
				DirectX::SimpleMath::Vector3 dir = diff / dist;

				// めり込んでいる量
				float over = (col.GetRadius() + 0.2f) - dist;

				// 押し出しベクトルを合計に加える
				totalPush += dir * over;
				isHit = true;
			}
		}

		// 全壁の押し出しを合算して1回だけ適用
		if (isHit) 
		{
			chara->SetPosition(chara->GetPosition() + totalPush);
			chara->SetIsKnockBack(false);
		}
	};

	// プレイヤーと敵に適用
	clampCharacter(m_player.get());
	clampCharacter(m_enemy.get());

	// オブジェクト登録 (Objects -> AddCollisionObject)
	m_collisionManager->AddCollisionObject(*m_player);
	m_collisionManager->AddCollisionObject(*m_enemy);

	// 攻撃オブジェクト登録 (AttackObjects -> Add...AttackObject)
	// ※攻撃を受ける側(Target)の登録が必要な場合は適宜 AddCollisionObject 等で行われている前提とします
	m_collisionManager->AddPlayerAttackObject(*m_playerAttack);
	m_collisionManager->AddPlayerAttackObject(*m_enemy);
	m_collisionManager->AddPlayerAttackObject(*m_playerGuardBreak);
	m_collisionManager->AddEnemyAttackObject(*m_enemyAttack);
	m_collisionManager->AddEnemyAttackObject(*m_player);
	m_collisionManager->AddEnemyAttackObject(*m_enemyGuardBreak);

	// ステージ登録 (StageObjects -> AddStageObject)
	m_collisionManager->AddStageObject(*m_stageCollision);

	// プレイヤー系登録 (PlayerObjects -> AddPlayerObject)
	m_collisionManager->AddPlayerObject(*m_player);
	m_collisionManager->AddPlayerObject(*m_enemy);

	// 判定実行
	m_collisionManager->CheckAllCollisions();
	m_collisionManager->CheckAttackToEnemy();
	m_collisionManager->CheckAttackToPlayer();
	m_collisionManager->CheckPlayerToStage();
	m_collisionManager->CheckPlayerToStageBreak();
	switch (m_currentStep) 
	{

	case TutorialStep::ATTACK:
		//敵はアイドルに固定
		m_enemyCtrlPtr->SetTutorialState(PATTERN_IDLE);
		// プレイヤーが攻撃したら次へ
		if(m_enemy->GetAttackHit() && m_enemy->GetCurrentState() != m_enemy->GetGuardState() && m_player->GetCurrentState() == m_player->GetAttackState())
		{
			m_currentStep = TutorialStep::GUARD;
		}
		break;
	case TutorialStep::GUARD:
		//敵はアタックに固定
		m_enemyCtrlPtr ->SetTutorialState(PATTERN_ATTACK);
		// プレイヤーがガードしたら次へ
		if (m_player->GetCurrentState() == m_player->GetGuardState() && m_player->GetAttackHit() && m_enemy->GetCurrentState() != m_enemy->GetGuardBreakState()) 
		{
			m_currentStep = TutorialStep::GUARD_BREAK;
		}
		break;
	case TutorialStep::GUARD_BREAK:
		//敵はガードに固定
		m_enemyCtrlPtr->SetTutorialState(PATTERN_GUARD);
		// ガードブレイクしたら次へ
		if (m_player->GetCurrentState() == m_player->GetGuardBreakState() && m_enemy->GetAttackHit() && m_enemy->GetCurrentState() == m_enemy->GetGuardState()) 
		{
			m_currentStep = TutorialStep::COMPLETE;
		}
		break;
	case TutorialStep::COMPLETE:
		// ゲームシーンへ遷移
		if(m_keyboardTracker.pressed.Z)
		{
			ChangeScene<TitleScene>();
		}
		break;
	}
}

/**
* @brief チュートリアルシーンの描画処理。デバッグフォントを用いた操作ガイド等を表示します
*/
void Lyse::TutorialScene::Render()
{
	auto* debugFont = GetUserResources()->GetDebugFont();
	debugFont->AddString(L"TutorialScene", DirectX::SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
	debugFont->AddString(L"Win", DirectX::SimpleMath::Vector2(0.0f, debugFont->GetFontHeight() * 2));
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	auto states = GetUserResources()->GetCommonStates();
	m_view = m_tpsCamera->GetCameraMatrix();
	// 各モデル描画
	m_stage->Render(context, states, m_view, m_proj);
	m_skydome->Render(context, states, m_view, m_proj);
	m_floorModel->Render(context, states, m_view, m_proj);

	m_player->Render(context, states, m_view, m_proj, debugFont);
	m_enemy->Render(context, states, m_view, m_proj, debugFont);

	// UI・エフェクト
	m_shaderManagerParticle->Render(m_view, m_proj);
	m_shaderManagerEffect->Render(m_view, m_proj);

	context->GSSetShader(nullptr, nullptr, 0); // ジオメトリシェーダーを空にする

	// チュートリアルの現在のステップに応じたUIを描画
	switch (m_currentStep) 
	{
	case TutorialStep::ATTACK:
		m_tutorialUI->Render(context, states, m_view, m_proj, debugFont, 0);
		break;
	case TutorialStep::GUARD:
		m_tutorialUI->Render(context, states, m_view, m_proj, debugFont, 1);
		break;
	case TutorialStep::GUARD_BREAK:
		m_tutorialUI->Render(context, states, m_view, m_proj, debugFont, 2);
		break;
	case TutorialStep::COMPLETE:
		m_tutorialUI->Render(context, states, m_view, m_proj, debugFont, 3);
		break;
	}
}

/**
* @brief チュートリアルシーンの終了処理
*/
void Lyse::TutorialScene::Finalize()
{
}

/**
* @brief デバイスに依存するリソース（スプライトバッチ、説明用テクスチャ等）の作成処理
*/
void Lyse::TutorialScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();

	m_debugSphere = DirectX::GeometricPrimitive::CreateSphere(context, 1.0f);

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	m_tutorialUI = std::make_unique<Lyse::TutorialUI>(*m_spriteBatch, device);

	// 床
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(MODEL_DIRECTORY);
	m_floorModel = std::make_unique<Lyse::FloorModel>(device, *fx);
}

/**
* @brief ウィンドウサイズに依存するリソースの作成処理
*/
void Lyse::TutorialScene::CreateWindowSizeDependentResources()
{
	RECT rect = GetUserResources()->GetDeviceResources()->GetOutputSize();

	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(FOV_ANGLE),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		NEAR_PLANE,
		FAR_PLANE
	);
}

/**
* @brief グラフィックスデバイスが失われた際の解放処理
*/
void Lyse::TutorialScene::OnDeviceLost()
{
}