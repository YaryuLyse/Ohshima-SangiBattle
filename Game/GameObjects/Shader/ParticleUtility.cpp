/**
* @file ParticleUtil.cpp
* @brief パーティクル個々の挙動を管理するクラスの実装ファイル
* @brief 1粒子ごとの座標更新、補間計算（スケール・色）、寿命管理を行う処理をまとめています
* @brief このクラスは、パーティクルの物理演算（加速度・速度）、寿命に応じたサイズと色の線形補間、および生存フラグの管理を担います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "ParticleUtility.h"

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	constexpr float LIFE_DEAD_THRESHOLD = 0.0f; // 死亡判定の閾値
	constexpr float LERP_BASE_ONE = 1.0f; // 補間計算用ベース値
}

/**
* @brief コンストラクタ。パーティクルの初期状態（寿命、物理量、色、スケール）を設定する
* @param life : 初期寿命
* @param position : 初期座標
* @param velocity : 初期速度
* @param acceleration : 加速度
* @param startScale : 開始時のスケール
* @param endScale : 終了時のスケール
* @param startColor : 開始時の色
* @param endColor : 終了時の色
* @param type : パーティクルの種類
*/
Lyse::ParticleUtility::ParticleUtility(
	float life,
	DirectX::SimpleMath::Vector3 position,
	DirectX::SimpleMath::Vector3 velocity,
	DirectX::SimpleMath::Vector3 acceleration,
	DirectX::SimpleMath::Vector3 startScale,
	DirectX::SimpleMath::Vector3 endScale,
	DirectX::SimpleMath::Color startColor,
	DirectX::SimpleMath::Color endColor,
	int type)
	: m_life(life)
	, m_startLife(life)
	, m_position(position)
	, m_velocity(velocity)
	, m_acceleration(acceleration)
	, m_currentScale(startScale)
	, m_startScale(startScale)
	, m_endScale(endScale)
	, m_currentColor(startColor)
	, m_startColor(startColor)
	, m_endColor(endColor)
	, m_type(type)
{
}

/**
* @brief デストラクタ
*/
Lyse::ParticleUtility::~ParticleUtility()
{
}

/**
* @brief パーティクルの更新処理。寿命の減少、物理挙動の計算、スケールと色の補間を行う
* @param elapsedTime : 経過時間
* @return bool : trueなら生存、falseなら寿命切れ
*/
bool Lyse::ParticleUtility::Update(float elapsedTime)
{
	// 進行割合の計算 (0.0f -> 1.0f)
	// (1.0 - 残り寿命 / 全寿命) なので、時間が経つほど 1 に近づく
	float t = LERP_BASE_ONE - (m_life / m_startLife);

	// スケールの線形補間更新
	m_currentScale = SimpleMath::Vector3::Lerp(m_startScale, m_endScale, t);

	// 色の線形補間更新
	m_currentColor = SimpleMath::Color::Lerp(m_startColor, m_endColor, t);

	// 物理挙動更新
	// 速度 ＝ 加速度 * 時間
	m_velocity += m_acceleration * elapsedTime;

	// 座標 ＝ 速度 * 時間
	m_position += m_velocity * elapsedTime;

	// ライフ減少
	m_life -= elapsedTime;

	// 死亡判定
	if (m_life < LIFE_DEAD_THRESHOLD)
	{
		return false;
	}

	return true;
}