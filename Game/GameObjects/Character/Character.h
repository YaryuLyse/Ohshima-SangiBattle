/**
* @file Character.h
* @brief 敵キャラクターの本体制御を管理するクラスのヘッダーファイル
* @brief 敵のステート管理、移動・回転計算、衝突判定、描画処理をまとめています
* @brief このクラスは、敵のHPやスタミナなどのステータス管理に加え、プレイヤーとの距離に応じた向きの制御や、攻撃を受けた際のダメージ計算・ノックバック処理を統括します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#pragma once

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <memory>
#include "SimpleMath.h"

#include "Game/GameObjects/Collider/SphereCollider.h"

// --- 前方宣言 (Forward Declarations) ---
// 循環参照を防ぐため、クラス名のみを先に宣言
class IdleState;
class GuardState;
class AttackState;
class GuardBreakState;
class MoveBackwardState;
class MoveForwardState;
class MoveRightState;
class MoveLeftState;
class AvoidState;
class DeathState;
class TutorialScene;
class StateBase;

namespace Lyse
{
    class GameScene;
	class Controller;
	class AttackCollider;
	class GuardBreakCollider;
	class ShaderManagerEffect;
    class Shield;
    class Sword;

    enum CharacterTag
    {
        PLAYER,
		ENEMY
    };

    /// <summary>
    /// 敵キャラクタークラス
    /// 衝突判定(SphereCollider)を継承し、ステートパターンによる行動制御を持つ
    /// </summary>
    class Character : public Lyse::SphereCollider
    {
    public:
        // --- コンストラクタ / デストラクタ ---
        Character(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx, std::unique_ptr<Lyse::Controller> ctrl, CharacterTag tag);
        ~Character();

        // --- 基本動作メソッド ---

        /// <summary>
        /// 初期化処理
        /// </summary>
        void Initialize();

        /// <summary>
        /// 更新処理
        /// </summary>
        /// <param name="elapsedTime">経過時間</param>
        void Update(float elapsedTime);

        /// <summary>
        /// 描画処理
        /// </summary>
        void Render(ID3D11DeviceContext1* context, DirectX::DX11::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, DebugFont* debugFont);

        /// <summary>
        /// 終了処理
        /// </summary>
        void Finalize();

        // --- セッター (Setters) ---

        void SetBasePosition(DirectX::SimpleMath::Vector3 basePos);
        void SetPosition(const DirectX::SimpleMath::Vector3& pos) override;

        // スケール設定（ベクトル指定）
        void SetScale(DirectX::SimpleMath::Vector3 scale);
        // スケール設定（均等指定）
        void SetScale(float scale);

        void SetRotation(DirectX::SimpleMath::Vector3 rot);
        void SetOpponent(Lyse::Character& opponent);
        void SetAttackCollider(Lyse::AttackCollider& attack);
        void SetGuardBreakCollider(Lyse::GuardBreakCollider& attack);
     
        /// <summary>
        /// スタミナを設定
        /// </summary>
        void SetStamina(float stamina) { m_stamina = stamina; }

        /// <summary>
        /// 移動可能フラグの設定
        /// </summary>
        void SetIsPlay(bool isPlay) { m_isPlay = isPlay; }

        /// <summary>
        /// 逆方向ノックバックフラグの設定
        /// </summary>
        void SetIsKnockBackReverse(bool isKnockBackReverse) { m_isKnockBackReverse = isKnockBackReverse; }

        /// <summary>
        /// シェーダーマネージャーの設定
        /// </summary>
        void SetShaderManagerEffect(ShaderManagerEffect& shaderManagerEffect) { m_shaderManagerEffect = &shaderManagerEffect; }

        /// <summary>
        /// 速度ベクトルの設定
        /// </summary>
        void SetVelocity(const DirectX::SimpleMath::Vector3& velocity);

        /// <summary>
        /// 所属シーンの設定
        /// </summary>
        void SetScene(GameScene* scene) { m_pScene = scene; }

		/// <summary>
		/// 所属シーンの設定（チュートリアル用）
		/// </summary>
        void SetTutorialScene(TutorialScene* scene) { m_pTutorialScene = scene; }

		/// <summary>
		/// スタン状態の設定
		/// </summary>
		void SetIsStunned(bool isStunned) { m_isStunned = isStunned; }

        // --- ゲッター (Getters) ---
        DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; };
        DirectX::SimpleMath::Matrix GetRotMat() const { return m_rotMat; }

        /// <summary>
        /// 衝突半径の取得
        /// </summary>
        float GetRadius() const override;

        int GetHP() const { return m_hp; }

        float GetStamina() const { return m_stamina; }

        bool GetIsPlay() { return m_isPlay; }

        /// <summary>
        /// ガード成功フラグの取得
        /// </summary>
        bool GetGuardSuccess() const { return m_isGuardSuccess; }

        /// <summary>
        /// 攻撃ヒットフラグの取得
        /// 取得と同時にフラグをリセットする場合があるため非const
        /// </summary>
        bool GetAttackHit();

        bool GetIsKnockBack() const { return m_isKnockBack; }

        bool GetIsKnockBackReverse() const { return m_isKnockBackReverse; }

        ShaderManagerEffect* GetShaderManagerEffect() const { return m_shaderManagerEffect; }

        /// <summary>
        /// 速度ベクトルの取得
        /// </summary>
        DirectX::SimpleMath::Vector3 GetVelocity();

        // --- 状態制御 / アクション ---

        /// <summary>
        /// 死亡しているか
        /// </summary>
        bool IsDead();

        /// <summary>
        /// 死亡フラグを立てる
        /// </summary>
        void IsDeath();

        /// <summary>
        /// 衝突時のコールバック
        /// </summary>
        virtual void OnCollision(SphereCollider* other) override;

        /// <summary>
        /// モデルの色を変更（ダメージ演出等）
        /// </summary>
        void ColorChange(DirectX::SimpleMath::Vector4 color);

        // ノックバック処理（後退）
        void KnockBack();
        // ノックバック処理（逆方向）
        void KnockBackReverse();

		bool IsStunned() const { return m_isStunned; }

        // --- ステート管理 ---

        /// <summary>
        /// ステートを変更する
        /// </summary>
        /// <param name="currentState">遷移先のステートポインタ</param>
        void ChangeState(StateBase* currentState);

        /// <summary>
        /// 現在のステートを取得
        /// </summary>
        StateBase* GetCurrentState() const { return m_currentState; }

        // 各ステートへのアクセサ
        GuardState* GetGuardState() { return m_guardState.get(); }
        IdleState* GetIdleState() { return m_idleState.get(); }
        AttackState* GetAttackState() { return m_attackState.get(); }
        GuardBreakState* GetGuardBreakState() { return m_guardBreakState.get(); }
        MoveBackwardState* GetMoveBackwardState() { return m_moveBackwardState.get(); }
        MoveForwardState* GetMoveForwardState() { return m_moveForwardState.get(); }
        MoveRightState* GetMoveRightState() { return m_moveRightState.get(); }
        MoveLeftState* GetMoveLeftState() { return m_moveLeftState.get(); }
		DeathState* GetDeathState() { return m_deathState.get(); }

        // 攻撃判定オブジェクトの取得
        AttackCollider* GetAttack() { return m_attackCollider; }
        GuardBreakCollider* GetGuardBreakCollider() { return m_guardBreakCollider; }

        //スタンしているかを取得
		bool GetIsStunned() const { return m_isStunned; }

		// キャラクターの角度をセット
        void SetRotationY(float radY) { m_radY = radY; }

		//倒れたときの回転角度を取得
        void SetDeathRotZ(float rotZ) { m_deathRotZ = rotZ; }

        //ノックバックするかどうかをセット
		void SetIsKnockBack(bool isKnockBack) { m_isKnockBack = isKnockBack; }

    private:
		/// --- 内部処理メソッド ---
        void OnHitByGuardBreak();
		void OnHitByAttack();
    private:
        // --- 定数 ---
        static constexpr int VERTEX_COUNT = 4; // 頂点数（必要であれば使用）
        const static DirectX::VertexPositionTexture VERTICES[VERTEX_COUNT];

        // --- トランスフォーム ---
        DirectX::SimpleMath::Vector3 m_position;
        DirectX::SimpleMath::Vector3 m_basePosition;
        DirectX::SimpleMath::Vector3 m_scale;
        DirectX::SimpleMath::Vector3 m_rotation;
        DirectX::SimpleMath::Matrix  m_rotMat;
        DirectX::SimpleMath::Vector3 m_distance;
        DirectX::SimpleMath::Vector3 m_prePosition;
        DirectX::SimpleMath::Vector3 m_vector = DirectX::SimpleMath::Vector3::Zero;

        // --- DirectXリソース ---
        Microsoft::WRL::ComPtr<ID3D11InputLayout>        m_inputLayout;
        std::unique_ptr<DirectX::CommonStates>           m_states;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
        std::unique_ptr<DirectX::AlphaTestEffect>        m_batchEffect;
        std::unique_ptr<DirectX::Model>                  m_charaModelP;
        std::unique_ptr<DirectX::Model>                  m_charaModelE;

        // --- 装備品 ---
        std::unique_ptr<Sword>  m_sword;
        std::unique_ptr<Shield> m_shield;

        // --- ステートパターン ---
        std::unique_ptr<IdleState>         m_idleState;
        std::unique_ptr<GuardState>        m_guardState;
        std::unique_ptr<AttackState>       m_attackState;
        std::unique_ptr<GuardBreakState>   m_guardBreakState;
        std::unique_ptr<MoveBackwardState> m_moveBackwardState;
        std::unique_ptr<MoveForwardState>  m_moveForwardState;
        std::unique_ptr<MoveRightState>    m_moveRightState;
        std::unique_ptr<MoveLeftState>     m_moveLeftState;
		std::unique_ptr<DeathState>        m_deathState;

        StateBase* m_currentState; // 現在のステートへのポインタ（所有権は持たない）

        // --- 関連オブジェクトへの参照 ---
        DirectX::SimpleMath::Vector3 m_playerPos;
        Lyse::Character* m_opponent = nullptr;
        AttackCollider* m_attackCollider;
		GuardBreakCollider* m_guardBreakCollider;
        Lyse::GameScene* m_pScene = nullptr;
		TutorialScene* m_pTutorialScene = nullptr;
        ShaderManagerEffect* m_shaderManagerEffect;

        // --- 入力管理 ---
        DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

        // --- ゲームパラメータ ---
        float m_time;
        int   m_hp;
        float m_stamina;
        float m_radius;         // 当たり判定半径
        bool  m_isDead;
        bool  m_isPlay = true;  // 行動可能フラグ

        // --- 状態異常・アクション管理 ---
        bool  m_isStunned = false;   // スタン中フラグ（ガードクラッシュ含む）
        float m_stunTimer = 0.0f;
        bool  m_recoverStamina = false; // 復帰時スタミナ全回復するか

        bool  m_isKnockBack = false;
        bool  m_isKnockBackReverse = false;
        float m_knockBackTimer = 0.0f;

        bool  m_isGuardSuccess = false; // ガード成功フラグ
        bool  m_isAttackHit = false;    // 攻撃ヒットフラグ
		float m_radY;              // Y軸回転角度
		float m_deathRotZ = 0.0f; //倒れたときのZ軸回転角度
        float m_stunRotY = 0.0f;
        float m_stunRotX = 0.0f;

        float m_invincibleTimer = 0.0f; // 無敵残り時間タイマー

        float m_guardRecoveryDelay = 0.0f; // ガード解除後の回復待機タイマー

		CharacterTag m_tag; // キャラクターのタグ（プレイヤー or 敵）

        std::unique_ptr<Controller> m_controller; // コントローラーへのポインタ（所有権は持たない）
    };
}
