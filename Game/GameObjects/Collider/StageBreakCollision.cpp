/**
* @file StageBreakCollision.cpp
* @brief ステージ破壊時の壁衝突判定を管理するクラスの実装ファイル
* @brief 壁が破壊された後に発生する進入不可領域の判定処理をまとめています
* @brief このクラスは、球体衝突判定を用いたオブジェクトの押し戻し補正や、壁に接触した際の速度ベクトル反射（弾ね返り）計算を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "StageBreakCollision.h"

#include <WICTextureLoader.h>
#include "Game/State/IdleState.h"
#include "Game/State/AttackState.h"
#include "Game/GameObjects/Character/Character.h"

using namespace DirectX;

namespace
{
	// 設定値
	const float DEFAULT_RADIUS = 7.5f;          // デフォルトの衝突半径
	const int   DEBUG_DRAW_DIVISIONS = 16;      // デバッグ描画の分割数
	const float REFLECTION_FACTOR = 2.0f;       // 反射係数（2.0 = 完全弾性）

	// 初期座標
	const DirectX::SimpleMath::Vector3 INIT_POSITION(0.0f, -0.5f, 5.0f);
}

/**
* @brief コンストラクタ。判定の初期状態や半径を設定する
*/
Lyse::StageBreakCollision::StageBreakCollision()
	: m_center(DirectX::SimpleMath::Vector3::Zero)
	, m_pos(DirectX::SimpleMath::Vector3::Zero)
	, m_radius(DEFAULT_RADIUS)
	, m_isCollision(false)
	, m_isActive(false)
	, m_player(nullptr)
	, m_enemy(nullptr)
{
}

/**
* @brief デストラクタ
*/
Lyse::StageBreakCollision::~StageBreakCollision()
{
}

/**
* @brief 初期化処理。メンバ変数の座標パラメータ等をデフォルト値に設定する
*/
void Lyse::StageBreakCollision::Initialize()
{
	m_position = INIT_POSITION;
	m_prePosition = DirectX::SimpleMath::Vector3::Zero;
	m_scale = DirectX::SimpleMath::Vector3::One;
	m_rotation = DirectX::SimpleMath::Vector3::Zero;
}

/**
* @brief 更新処理。毎フレームの衝突フラグのリセットなどを行う
*/
void Lyse::StageBreakCollision::Update()
{
	auto kb = DirectX::Keyboard::Get().GetState();

	// 毎フレームリセット
	m_isCollision = false;

	// アクティブな時だけクランプ処理
	if (!m_isActive) return;
}

/**
* @brief 描画処理。デバッグモード時、有効な衝突範囲をワイヤーフレーム球体として描画する
* @param batch : 頂点描画用バッチ
*/
void Lyse::StageBreakCollision::Render(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>& batch)
{
	UNREFERENCED_PARAMETER(batch); // デバッグ描画は後で実装予定
}

/**
* @brief 終了処
*/
void Lyse::StageBreakCollision::Finalize()
{
}

/**
* @brief 指定座標で衝突判定を有効化する。壁破壊時に呼び出される
* @param position : 有効化する中心座標
*/
void Lyse::StageBreakCollision::Activate(const DirectX::SimpleMath::Vector3& position)
{
	m_pos = position;
	m_isActive = true;
}

/**
* @brief 衝突判定を無効化する
*/
void Lyse::StageBreakCollision::Deactivate()
{
	m_isActive = false;
}



/**
* @brief 衝突時の応答処理。対象を範囲外へ押し戻し、進行方向とは逆の反射速度を与える
* @param other : 衝突相手のコリジョンオブジェクト
*/
void Lyse::StageBreakCollision::OnCollision(SphereCollider* other)
{
	UNREFERENCED_PARAMETER(other); // 今は衝突相手の情報を使用しないため、未使用パラメータとしてマクロで処理しています。
	
}