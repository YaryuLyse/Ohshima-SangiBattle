/**
* @file StageCollision.cpp
* @brief ステージ外周の衝突判定を管理するクラスの実装ファイル
* @brief 円形ステージの境界維持と、壁破壊のトリガー処理をまとめています
* @brief このクラスは、プレイヤーや敵がステージ外へ出ないように押し戻す処理に加え、衝突時の角度から破壊対象の壁を特定する管理を行います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "StageCollision.h"

#include <WICTextureLoader.h>
#include "Game/State/IdleState.h"
#include "Game/State/AttackState.h"
#include "Game/GameObjects/Character/Character.h"
#include "Game/GameObjects/Stage/Stage.h"

using namespace DirectX;

namespace
{
	// 設定値
	const float MODEL_SCALE = 3.0f;
	const float REFLECTION_FACTOR = 2.0f; // 反射係数

	// 壁破壊計算用
	const float WALL_COUNT = 10.0f;       // 壁の総枚数

	// 初期座標（Initializeで使用）
	const DirectX::SimpleMath::Vector3 INIT_POSITION(0.0f, -0.5f, 5.0f);
}

/**
* @brief コンストラクタ。ステージの中心座標と半径を保持する
* @param center : ステージの中心座標
* @param radius : ステージの半径
*/
Lyse::StageCollision::StageCollision(const DirectX::SimpleMath::Vector3& center, float radius)
	: m_center(center)
	, m_radius(radius)
	, m_isCollision(false)
	, m_stage(nullptr)
	, m_player(nullptr)
	, m_enemy(nullptr)
{
}

/**
* @brief デストラクタ
*/
Lyse::StageCollision::~StageCollision()
{
}

/**
* @brief 初期化処理。メンバ変数の座標パラメータ等をデフォルト値に設定する
*/
void Lyse::StageCollision::Initialize()
{
	m_position = INIT_POSITION;
	m_prePosition = DirectX::SimpleMath::Vector3::Zero;
	m_scale = DirectX::SimpleMath::Vector3::One;
	m_rotation = DirectX::SimpleMath::Vector3::Zero;
}

/**
* @brief 更新処理。毎フレームの衝突フラグのリセットなどを行う
*/
void Lyse::StageCollision::Update()
{
	auto kb = DirectX::Keyboard::Get().GetState();
	m_isCollision = false;

	// プレイヤーの位置を毎フレームクランプ
	if (m_player) 
	{
		ClampToStage(m_player);
	}
	if (m_enemy) 
	{
		ClampToStage(m_enemy);
	}
}

/**
* @brief 描画処理。必要に応じて衝突判定用のモデルを描画する
* @param context : デバイスコンテキスト
* @param states : 共通ステート
* @param view : ビュー行列
* @param proj : プロジェクション行列
*/
void Lyse::StageCollision::Render(ID3D11DeviceContext1* context, DirectX::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	DirectX::SimpleMath::Matrix scaleMat;
	DirectX::SimpleMath::Matrix transMat;
	DirectX::SimpleMath::Matrix rotMat;

	transMat = DirectX::SimpleMath::Matrix::CreateTranslation(m_center);
	rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(0.0f);
	scaleMat = DirectX::SimpleMath::Matrix::CreateScale(MODEL_SCALE);

	DirectX::SimpleMath::Matrix world = transMat * rotMat * scaleMat;

	m_isCollision = false;
}

/**
* @brief 終了処理
*/
void Lyse::StageCollision::Finalize()
{
}

/**
* @brief 衝突時の応答処理。外周制限を超えたオブジェクトの押し戻し、反射、および壁の破壊判定を行う
* @param other : 衝突相手のコリジョンオブジェクト
*/
void Lyse::StageCollision::OnCollision(SphereCollider* other)
{
	UNREFERENCED_PARAMETER(other); // 今は衝突相手の情報を直接使用しないため、未使用パラメータとしてマクロで処理
}

void Lyse::StageCollision::ClampToStage(Lyse::Character* chara)
{
	DirectX::SimpleMath::Vector3 pos = chara->GetPosition();
	DirectX::SimpleMath::Vector3 diff = pos - m_center;
	float dist = diff.Length();

	if (dist > m_radius - 0.2f)
	{
		// ① 壁破壊判定を先に行う
		if (m_stage != nullptr && chara->GetIsKnockBack())
		{
			float angle = atan2f(diff.z, diff.x) + DirectX::XM_PIDIV2;
			if (angle < 0.0f) angle += DirectX::XM_2PI;
			float unitAngle = DirectX::XM_2PI / WALL_COUNT;
			int wallIndex = static_cast<int>(angle / unitAngle);

			if (chara == m_enemy)  m_enemy->SetIsKnockBackReverse(true);
			if (chara == m_player) m_player->SetIsKnockBackReverse(true);
			m_stage->BreakWall(wallIndex);
		}

		// ② その後で押し戻す
		DirectX::SimpleMath::Vector3 dir = diff / dist;
		chara->SetPosition(m_center + dir * (m_radius - 0.2f));
		chara->SetIsKnockBack(false);
	}
}

