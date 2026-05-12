/**
* @file Character.cpp
* @brief 敵キャラクターの本体制御を管理するクラスの実装ファイル
* @brief 敵のステート管理、移動・回転計算、衝突判定、描画処理をまとめています
* @brief このクラスは、敵のHPやスタミナなどのステータス管理に加え、プレイヤーとの距離に応じた向きの制御や、攻撃を受けた際のダメージ計算・ノックバック処理を統括します。
* @author 大嶋　琉也
* @date 2026/02/25
*/

#include "pch.h"
#include "Character.h"
#include <PrimitiveBatch.h> 
#include <VertexTypes.h> 
#include <WICTextureLoader.h> 
#include "Game/GameObjects/Collider/AttackCollider.h"
#include "Game/GameObjects/Collider/GuardBreakCollider.h"
#include "Game/GameObjects/Collider/StageBreakCollision.h"
#include "Game/State/StateBase.h"
#include "Game/State/IdleState.h"
#include "Game/State/GuardState.h"
#include "Game/State/AttackState.h"
#include "Game/State/GuardBreakState.h"
#include "Game/State/MoveBackwardState.h"
#include "Game/State/MoveForwardState.h"
#include "Game/State/MoveRightState.h"
#include "Game/State/MoveLeftState.h"
#include "Game/State/DeathState.h"
#include "Game/Scene/GameScene.h"
#include "Game/Scene/TutorialScene.h"
#include "Game/GameObjects/Shader/ShaderManagerEffect.h"
#include "Game/GameObjects/Character/Controller.h"
#include "Game/GameObjects/Character/Sword.h"
#include "Game/GameObjects/Character/Shield.h"
#include "Game/GameObjects/Character/EnemyController.h"

using namespace DirectX;

// --- 定数定義（マジックナンバーの排除・統合） ---
namespace 
{
	// 描画関連
	constexpr int   ALPHA_REF_VALUE = 255;    // アルファテストの基準値
	constexpr float RENDER_OFFSET_Y = 0.5f;   // モデル描画時のY座標補正

	// 初期パラメータ
	constexpr float INIT_POS_X = -1.0f;
	constexpr float INIT_POS_Y = 0.0f;
	constexpr float INIT_POS_Z = -1.0f;
	constexpr int   INIT_HP = 100;
	constexpr float INIT_STAMINA = 100.0f;
	constexpr float INIT_RADIUS = 1.0f;   // 当たり判定半径

	// スタミナ・状態異常関連
	constexpr float STAMINA_MAX = 100.0f;
	constexpr float STAMINA_MIN = 0.0f;
	constexpr float STAMINA_RECOVERY_RATE = 10.0f;  // スタミナ毎秒回復量
	constexpr float GUARD_CRASH_DURATION = 2.0f;   // ガード崩し状態の継続時間
	constexpr float KNOCKBACK_DURATION = 0.1f;   // ノックバック時間
	constexpr float STUN_DURATION = 2.0f;   // スタン時間
	constexpr int   AI_PATTERN_STUN = 4;      // スタン時のAIパターンID

	// ダメージ値
	constexpr int   DAMAGE_SMALL = 5;
	constexpr int   DAMAGE_MEDIUM = 20;
	constexpr int   DAMAGE_LARGE = 30;

	// スタミナ減少値
	constexpr float STAMINA_DAMAGE_LOW = 10.0f;
	constexpr float STAMINA_DAMAGE_HIGH = 20.0f;
	constexpr float STAMINA_DAMAGE_CRIT = 1000.0f; // ガードブレイク確定級

	// ノックバック移動量
	constexpr float KNOCKBACK_DIST = -1.0f;   // 通常ノックバック（後退）
	constexpr float KNOCKBACK_REVERSE_DIST = 0.5f;    // 逆ノックバック（前進？）

	// 衝突判定用
	constexpr float COLLISION_EPSILON = 0.001f;  // ゼロ除算防止用の微小値
	constexpr float DEFAULT_PUSH_DIST = 1.0f;    // 距離が0だった場合の押し出し距離
	constexpr float DEFAULT_ALPHA = 1.0f;    // 色変更時のアルファ値

	constexpr float INVINCIBLE_DURATION = 1.0f; // 無敵時間（0.5秒など好みで調整)

	constexpr float GUARD_STAMINA_RECOVERY_DELAY = 3.5f; // ガード解除後の回復待機時間
}

/**
* @brief コンストラクタ。リソースの生成と初期設定を行う
* @param device : DirectXデバイス
* @param fx : エフェクトファクトリ
* @param ctrl : プレイヤーコントローラーへのポインタ
*/
Lyse::Character::Character(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx, std::unique_ptr<Lyse::Controller> ctrl, CharacterTag tag)
	: m_position(DirectX::SimpleMath::Vector3::Zero)
	, m_basePosition(DirectX::SimpleMath::Vector3::Zero)
	, m_scale(DirectX::SimpleMath::Vector3::One)
	, m_rotation(DirectX::SimpleMath::Vector3::Zero)
	, m_isDead(false)
	, m_time(0.0f)
	, m_hp(INIT_HP)
	, m_stamina(INIT_STAMINA)
	, m_radius(INIT_RADIUS)
	, m_currentState(nullptr)
	, m_attackCollider(nullptr)
	, m_guardBreakCollider(nullptr)
	, m_shaderManagerEffect(nullptr)
	, m_controller(std::move(ctrl))
	, m_tag(tag)
	, m_deathRotZ(0.0f)
	, m_stunRotY(0.0f)
	, m_stunRotX(0.0f)
	, m_radY(0.0f)
{
	// --- 装備品の生成 ---
	m_sword = std::make_unique<Lyse::Sword>(device, fx);
	m_shield = std::make_unique<Lyse::Shield>(device, fx);

	// --- 描画エフェクトの設定 ---
	// アルファテスト（透過処理）有効化
	m_batchEffect = std::make_unique<AlphaTestEffect>(device);
	m_batchEffect->SetAlphaFunction(D3D11_COMPARISON_EQUAL);
	m_batchEffect->SetReferenceAlpha(ALPHA_REF_VALUE);

	// --- 入力レイアウトの生成 ---
	// 頂点シェーダーの情報を取得し、頂点フォーマットを定義する
	void const* shaderByteCode;
	size_t byteCodeLength;
	m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	device->CreateInputLayout(
		VertexPositionTexture::InputElements,
		VertexPositionTexture::InputElementCount,
		shaderByteCode,
		byteCodeLength,
		m_inputLayout.GetAddressOf()
	);

	// --- 共通ステートの作成 ---
	m_states = std::make_unique<CommonStates>(device);

	// --- 3Dモデルのロード ---
	// SDKMESHファイルを読み込む
	fx.SetSharing(false); // エフェクトの共有設定をオフにする
	m_charaModelP = DirectX::Model::CreateFromSDKMESH(
		device,
		L"Resources/Models/aka.sdkmesh",
		fx
	);
	m_charaModelE = DirectX::Model::CreateFromSDKMESH(
		device,
		L"Resources/Models/ao.sdkmesh",
		fx
	);

	// ロードしたテクスチャをエフェクトに適用
	m_batchEffect->SetTexture(m_texture.Get());
}

/**
* @brief デストラクタ
*/
Lyse::Character::~Character()
{
}

/**
* @brief 敵モデルの初期化処理
*/
void Lyse::Character::Initialize()
{
	// 座標・姿勢の初期化
	m_position = DirectX::SimpleMath::Vector3(INIT_POS_X, INIT_POS_Y, INIT_POS_Z);
	m_basePosition = DirectX::SimpleMath::Vector3::Zero;
	m_scale = DirectX::SimpleMath::Vector3::One;
	m_rotation = DirectX::SimpleMath::Vector3::Zero;

	// ステータスの初期化
	m_isDead = false;
	m_time = 0;
	m_hp = INIT_HP;
	m_stamina = INIT_STAMINA;
	m_radius = INIT_RADIUS;

	// --- ステート（状態）クラスの生成 ---
	// 各ステートにこのクラス自身(this)を渡し、Characterを操作できるようにする
	m_idleState = std::make_unique<IdleState>(this);
	m_guardState = std::make_unique<GuardState>(this);
	m_attackState = std::make_unique<AttackState>(this);
	m_guardBreakState = std::make_unique<GuardBreakState>(this);
	m_moveBackwardState = std::make_unique<MoveBackwardState>(this);
	m_moveForwardState = std::make_unique<MoveForwardState>(this);
	m_moveRightState = std::make_unique<MoveRightState>(this);
	m_moveLeftState = std::make_unique<MoveLeftState>(this);
	m_deathState = std::make_unique<DeathState>(this);

	// --- 装備品の初期化と紐づけ ---
	m_sword->SetCharacter(this);
	m_shield->SetCharacter(this);
	m_sword->Initialize();
	m_shield->Initialize();

	// 最初の状態を「前進」に設定
	m_currentState = m_moveForwardState.get();
}

/**
* @brief 敵モデルの更新処理
* @param elapsedTime : 経過時間
*/
void Lyse::Character::Update(float elapsedTime)
{
	// 全体時間の更新
	m_time += elapsedTime;

	// Y座標を床の高さに強制リセット（簡易的な接地処理）
	m_position.y = INIT_POS_Y;

	// 入力情報の更新
	// キーボードの状態を取得し、Trackerを更新して押下判定などを可能にする
	if (m_controller) 
	{
		if (!(m_currentState == m_deathState.get() || m_opponent->GetCurrentState() == m_opponent->GetDeathState()))
		{
			m_controller->Update(this, elapsedTime);
		}
			
	}
	m_rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(m_radY);
	// --- 現在のステートの更新 ---
	// 移動や攻撃など、現在の状態に応じた処理を実行する
	
	m_currentState->Update(m_keyboardTracker, elapsedTime);

	// --- 装備品の更新 ---
	// 本体に合わせて剣と盾の位置・回転を更新
	m_sword->Update(elapsedTime, m_position, m_rotMat);
	m_shield->Update(elapsedTime, m_position, m_rotMat);

	/// --- スタン状態の管理 ---
	if (m_isStunned) 
	{
		m_stunTimer += elapsedTime;
		ChangeState(m_idleState.get());
		auto* enemyCtrl = dynamic_cast<Lyse::EnemyController*>(m_controller.get());
			// スタン中はY軸にくるくる回転
		m_stunRotY += DirectX::XMConvertToRadians(360.0f) * elapsedTime;

		// 敵だけAIパターンを固定
		if (enemyCtrl) 
		{
			enemyCtrl->SetActionPattern(AI_PATTERN_STUN);
		}
		// プレイヤーはPlayerController::UpdateのIsStunned()チェックで止まる

		if (m_stunTimer >= STUN_DURATION) 
		{
			m_isStunned = false;
			m_stunTimer = 0.0f;
			m_stunRotY = 0.0f;
			if (m_recoverStamina) 
			{
				m_stamina = STAMINA_MAX;
				m_recoverStamina = false;
			}
			if (enemyCtrl) 
			{
				enemyCtrl->SetActionPattern(AI_PATTERN_STUN);
				enemyCtrl->DetermineNextAction(this);
			}
		}
	}

	if (m_currentState == m_guardState.get())
	{
		// ガード中は回復しない＋ディレイタイマーをリセット
		m_guardRecoveryDelay = GUARD_STAMINA_RECOVERY_DELAY;
	}
	else if (m_isStunned)
	{
		// スタン中も回復しない
	}
	else
	{
		if (m_guardRecoveryDelay > 0.0f)
		{
			m_guardRecoveryDelay -= elapsedTime; // 待機中
		}
		else
		{
			m_stamina += STAMINA_RECOVERY_RATE * elapsedTime; // 待機後に回復
		}
	}

	// --- スタミナ管理 ---
	// スタミナ値が範囲外にならないよう制限（クランプ処理）
	if (m_stamina < STAMINA_MIN)
	{
		m_stamina = STAMINA_MIN;
	}
	if (m_stamina > STAMINA_MAX)
	{
		m_stamina = STAMINA_MAX;
	}
	// ガード中やスタン中はスタミナ回復しないようにする
	if (!(m_currentState == m_guardState.get() || m_isStunned == true)) 
	{
		m_stamina += STAMINA_RECOVERY_RATE * elapsedTime; // 毎秒回復量に経過時間を掛けて回復
	}

	if (m_invincibleTimer > 0.0f) 
	{
		m_invincibleTimer -= elapsedTime; // 経過時間分だけ減らす
	}



	// --- 死亡判定 ---
	if (m_hp <= 0) 
	{
		ChangeState(m_deathState.get()); // 倒れるステートへ
	}

	// --- ノックバック処理 ---
	if (m_isKnockBack == true) 
	{
		m_knockBackTimer += elapsedTime;

		// プレイヤー攻撃による後退処理
		KnockBack();

		if (m_pScene != nullptr) 
		{
			// シーンへノックバック発生を通知
			m_pScene->IsKnockBack(m_isKnockBack, m_position);
		}

		// 指定時間経過でノックバック終了
		if (m_knockBackTimer >= KNOCKBACK_DURATION) 
		{
			m_isKnockBack = false;
			m_knockBackTimer = 0.0f;
		}
	}

	// --- ノックバック（逆方向）処理 ---
	if (m_isKnockBackReverse == true) 
	{
		m_knockBackTimer += elapsedTime;

		KnockBackReverse();

		if (m_knockBackTimer >= KNOCKBACK_DURATION) 
		{
			m_isKnockBackReverse = false;
			m_knockBackTimer = 0.0f;
		}
	}

	

	// 攻撃ヒット判定フラグのリセット（毎フレーム初期化）
	m_isAttackHit = false;
}

/**
* @brief 敵モデルの描画処理
* @param context : デバイスコンテキスト
* @param states : コモンステート
* @param view : ビュー行列
* @param proj : プロジェクション行列
* @param debugFont : デバッグフォント描画用クラス
*/
void Lyse::Character::Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, SimpleMath::Matrix view, SimpleMath::Matrix proj, DebugFont* debugFont)
{
	// 行列計算用の変数
	DirectX::SimpleMath::Matrix scale;
	DirectX::SimpleMath::Matrix trans;
	DirectX::SimpleMath::Matrix rot;

	// 1. 平行移動行列: 現在位置に加え、足元が埋まらないようY座標を補正
	trans = DirectX::SimpleMath::Matrix::CreateTranslation(m_position + DirectX::SimpleMath::Vector3(0.0f, RENDER_OFFSET_Y, 0.0f));

	// 2. 回転行列: 現在は固定値(0)だが、本来はm_rotMatを使用する想定
	DirectX::SimpleMath::Matrix rotZ = DirectX::SimpleMath::Matrix::CreateRotationZ(m_deathRotZ); // 単位行列
	if(m_isStunned) 
	{
		DirectX::SimpleMath::Matrix rotStunY =
			DirectX::SimpleMath::Matrix::CreateRotationY(m_stunRotY);
		DirectX::SimpleMath::Matrix rotStunX =
			DirectX::SimpleMath::Matrix::CreateRotationX(
				DirectX::XMConvertToRadians(20.0f)); // ← 前に傾ける角度
		rot = rotStunX * rotStunY;
	}
	else 
	{
		rot = rotZ * m_rotMat; //死亡時のみ回転
	}


	// 3. 拡大縮小行列: 等倍
	scale = DirectX::SimpleMath::Matrix::CreateScale(1.0f, 1.0f, 1.0f);

	// ワールド行列の合成 (拡大 -> 回転 -> 移動 の順序が基本)
	DirectX::SimpleMath::Matrix world = scale * rot * trans;

	// 生存している場合のみ描画
	if ((!IsDead())) 
	{
		// 装備品の描画
		m_sword->Render(context, states, view, proj, debugFont);
		m_shield->Render(context, states, view, proj, debugFont);

		// 本体の描画
		
		if (m_tag == PLAYER) 
		{
			m_charaModelP->Draw(context, *states, world, view, proj);
		}
		else if (m_tag == ENEMY) 
		{
			m_charaModelE->Draw(context, *states, world, view, proj);
		}
	}

}

/**
* @brief 終了処理
*/
void Lyse::Character::Finalize()
{
}

/**
* @brief 衝突判定時の挙動処理
* @param other : 衝突相手のオブジェクト
*/
void Lyse::Character::OnCollision(SphereCollider* other)
{
	// --- プレイヤー本体との衝突 ---
	if (dynamic_cast<Character*>(other)) 
	{
		// 自分自身との衝突は無視
		if (other == this) return;

		// 攻撃オブジェクトは除外（Attackは別で処理）
		if (dynamic_cast<AttackCollider*>(other)) return;

		DirectX::SimpleMath::Vector3 diff = m_position - other->GetPosition();
		float distance = diff.Length();
		float hitRadius = m_radius + other->GetRadius();

		if (distance < hitRadius) 
		{
			if (distance <= COLLISION_EPSILON) 
			{
				diff = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f);
				distance = DEFAULT_PUSH_DIST;
			}
			DirectX::SimpleMath::Vector3 direction = diff / distance;
			float pushLength = hitRadius - distance;
			m_position += direction * pushLength; // 0.5fで両者が均等に押し出される
		}
	}

	// --- 攻撃との衝突（先にチェック）---
	if (auto* attack = dynamic_cast<AttackCollider*>(other)) 
	{
		if (attack == m_attackCollider) return; // 自分の攻撃は無視
		if (attack->GetAttackFlag()) return;  // すでにヒット済みなら無視
		if (m_invincibleTimer > 0.0f) return; // 無敵時間中はヒットしない

		attack->SetAttackFlag(true); // ヒットしたのでフラグを立てる
		m_invincibleTimer = INVINCIBLE_DURATION; // 無敵時間をリセット
		m_isAttackHit = true; // シーンへの通知用フラグも立てる

		OnHitByAttack();
		
		if(m_pScene !=nullptr)
		m_pScene->IsAttackHit(m_isAttackHit, m_position, m_isGuardSuccess);
	}
	if (auto* guardBreak = dynamic_cast<GuardBreakCollider*>(other)) 
	{
		if (guardBreak == m_guardBreakCollider) return; // 自分のガードブレイクは無視
		if (guardBreak->GetGuardBreakFlag()) return;  // すでにヒット済みなら無視
		if (m_invincibleTimer > 0.0f) return; // 無敵時間中はヒットしない

		guardBreak->SetGuardBreakFlag(true); // ヒットしたのでフラグを立てる
		m_invincibleTimer = INVINCIBLE_DURATION; // 無敵時間をリセット
		m_isAttackHit = true; // シーンへの通知用フラグも立てる

		OnHitByGuardBreak();

		if (m_pScene != nullptr)
			m_pScene->IsAttackHit(m_isAttackHit, m_position, m_isGuardSuccess);
	}

	// --- ステージの障害物（壁・瓦礫など）との衝突 ---
	if (dynamic_cast<StageBreakCollision*>(other))
	{
	}
}

/**
* @brief モデルの色変更処理
* @param color : 設定するRGBAカラー
*/
void Lyse::Character::ColorChange(DirectX::SimpleMath::Vector4 color)
{
	auto model = m_charaModelP.get();

	for (auto& mesh : model->meshes)
	{
		for (auto& part : mesh->meshParts)
		{
			auto effect = dynamic_cast<DirectX::BasicEffect*>(part->effect.get());
			if (effect)
			{
				effect->SetDiffuseColor(color);
				effect->SetAlpha(DEFAULT_ALPHA);
			}
		}
	}
}

/**
* @brief 後退方向へのノックバック移動処理
*/
void Lyse::Character::KnockBack()
{
	m_vector = DirectX::SimpleMath::Vector3(0.0f, 0.0f, KNOCKBACK_DIST);
	m_vector = DirectX::SimpleMath::Vector3::Transform(m_vector, m_rotMat);
	m_position += m_vector;
}

/**
* @brief 前進方向へのノックバック移動処理
*/
void Lyse::Character::KnockBackReverse()
{
	m_vector = DirectX::SimpleMath::Vector3(0.0f, 0.0f, KNOCKBACK_REVERSE_DIST);
	m_vector = DirectX::SimpleMath::Vector3::Transform(m_vector, m_rotMat);
	m_position += m_vector;
}

/**
* @brief 攻撃ヒットフラグの取得処理
* @return bool : ヒットしたかどうか
*/
bool Lyse::Character::GetAttackHit()
{
	return m_isAttackHit;
}



/**
* @brief ステート（状態）の遷移処理
* @param currentState : 遷移先のステートインスタンス
*/
void Lyse::Character::ChangeState(StateBase* currentState)
{
	if (m_currentState == currentState) 
	{
		return;
	}
	m_currentState->PostUpdate();
	m_currentState = currentState;
	m_currentState->PreUpdate();
}

void Lyse::Character::OnHitByGuardBreak()
{
	if (m_currentState == m_guardState.get())
	{
		m_hp -= DAMAGE_MEDIUM;
		m_stamina -= STAMINA_DAMAGE_CRIT;
		m_recoverStamina = true;
		m_isKnockBack = true;
		m_isStunned = false;
		m_isGuardSuccess = false;
	}
	else
	{
		m_hp -= DAMAGE_SMALL;
		m_opponent->SetIsStunned(true);
		m_isGuardSuccess = true;
	}
}

void Lyse::Character::OnHitByAttack()
{
	if (m_currentState == m_guardState.get())
	{
		m_stamina -= STAMINA_DAMAGE_LOW;
		m_opponent->SetIsStunned(true);
		m_isGuardSuccess = true;

		if (m_stamina < STAMINA_MIN)
		{
			m_stamina = STAMINA_MIN;
			m_isStunned = true;
			m_recoverStamina = true;
			m_isGuardSuccess = false;
		}
	}
	else
	{
		m_hp -= DAMAGE_MEDIUM;
		m_isKnockBack = true;
		m_isStunned = false;
		m_isGuardSuccess = false;
	}
}

/**
* @brief 基準位置の設定
* @param basePos : 設定する座標
*/
void Lyse::Character::SetBasePosition(DirectX::SimpleMath::Vector3 basePos)
{
	m_basePosition = basePos;
}

/**
* @brief 現在位置の設定
* @param pos : 設定する座標
*/
void Lyse::Character::SetPosition(const DirectX::SimpleMath::Vector3& pos)
{
	m_position = pos;
}

/**
* @brief モデルのスケール設定
* @param scale : 設定するスケール値(Vector3)
*/
void Lyse::Character::SetScale(DirectX::SimpleMath::Vector3 scale)
{
	m_scale = scale;
}

/**
* @brief モデルの均等スケール設定
* @param scale : 設定する倍率
*/
void Lyse::Character::SetScale(float scale)
{
	m_scale.x = scale;
	m_scale.y = scale;
	m_scale.z = scale;
}

/**
* @brief モデルの回転角設定
* @param rot : 各軸の回転角(Vector3)
*/
void Lyse::Character::SetRotation(DirectX::SimpleMath::Vector3 rot)
{
	m_rotation = rot;
}

/**
* @brief ターゲットとなるプレイヤーモデルの設定
* @param player : プレイヤーモデルへの参照
*/
void Lyse::Character::SetOpponent(Lyse::Character& opponent)
{
	m_opponent = &opponent;
}

/**
* @brief 自身のガードブレイク判定オブジェクトの設定
* @param guardBreak : ガードブレイク判定クラスへの参照
*/
void Lyse::Character::SetGuardBreakCollider(Lyse::GuardBreakCollider& attack)
{
	m_guardBreakCollider = &attack;
}


/**
* @brief 移動ベクトルの設定
* @param velocity : 設定するベクトル
*/
void Lyse::Character::SetVelocity(const DirectX::SimpleMath::Vector3& velocity)
{
	m_vector = velocity;
}

/**
* @brief 自身の攻撃判定オブジェクトの設定
* @param enemyAttack : 攻撃判定クラスへの参照
*/
void Lyse::Character::SetAttackCollider(Lyse::AttackCollider& attack)
{
	m_attackCollider = &attack;
}

/**
* @brief 現在の移動ベクトルの取得
* @return Vector3 : 移動ベクトル
*/
DirectX::SimpleMath::Vector3 Lyse::Character::GetVelocity()
{
	return m_vector;
}

/**
* @brief 死亡状態の確認
* @return bool : 死亡していればtrue
*/
bool Lyse::Character::IsDead()
{
	return m_isDead;
}

/**
* @brief 死亡フラグを強制的に立てる
*/
void Lyse::Character::IsDeath()
{
	m_isDead = true;
}

/**
* @brief 衝突判定用の半径を取得
* @return float : 半径
*/
float Lyse::Character::GetRadius() const
{
	return 1.0f;
}