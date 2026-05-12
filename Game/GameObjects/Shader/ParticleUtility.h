#pragma once
/**
* @file ParticleUtil.h
* @brief パーティクル個々の挙動を管理するクラスのヘッダーファイル
* @brief 1粒子ごとの座標更新、補間計算（スケール・色）、寿命管理を行う処理をまとめています
* @brief このクラスは、パーティクルの物理演算（加速度・速度）、寿命に応じたサイズと色の線形補間、および生存フラグの管理を担います。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "Game/Common/StepTimer.h"
#include "Game/Common/DeviceResources.h"
#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>

namespace Lyse
{
	class ParticleUtility
	{
	public:
		// コンストラクタ
		ParticleUtility(
			float life,
			DirectX::SimpleMath::Vector3 position,
			DirectX::SimpleMath::Vector3 velocity,
			DirectX::SimpleMath::Vector3 acceleration,
			DirectX::SimpleMath::Vector3 startScale,
			DirectX::SimpleMath::Vector3 endScale,
			DirectX::SimpleMath::Color startColor,
			DirectX::SimpleMath::Color endColor,
			int type
		);

		// デストラクタ
		~ParticleUtility();

		// 更新
		bool Update(float elapsedTime);

		// --- ゲッター ---

		// 座標・移動関連
		const DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
		const DirectX::SimpleMath::Vector3 GetVelocity() const { return m_velocity; }
		const DirectX::SimpleMath::Vector3 GetAcceleration() const { return m_acceleration; }

		// スケール関連
		const DirectX::SimpleMath::Vector3 GetCurrentScale() const { return m_currentScale; }
		const DirectX::SimpleMath::Vector3 GetStartScale() const { return m_startScale; }
		const DirectX::SimpleMath::Vector3 GetEndScale() const { return m_endScale; }

		// ライフ関連
		float GetLife() const { return m_life; }
		float GetStartLife() const { return m_startLife; }

		// カラー関連
		const DirectX::SimpleMath::Color GetCurrentColor() const { return m_currentColor; }
		const DirectX::SimpleMath::Color GetStartColor() const { return m_startColor; }
		const DirectX::SimpleMath::Color GetEndColor() const { return m_endColor; }

		// その他
		int GetType() const { return m_type; }

	private:
		// --- 座標・移動 ---
		DirectX::SimpleMath::Vector3 m_position;     // 現在座標
		DirectX::SimpleMath::Vector3 m_velocity;     // 速度
		DirectX::SimpleMath::Vector3 m_acceleration; // 加速度

		// --- スケール ---
		DirectX::SimpleMath::Vector3 m_currentScale; // 現在サイズ
		DirectX::SimpleMath::Vector3 m_startScale;   // 開始サイズ
		DirectX::SimpleMath::Vector3 m_endScale;     // 終了サイズ

		// --- 生存時間 ---
		float m_life;        // 現在の残り時間
		float m_startLife;   // 初期生存時間

		// --- カラー ---
		DirectX::SimpleMath::Color m_currentColor;   // 現在色
		DirectX::SimpleMath::Color m_startColor;     // 開始色
		DirectX::SimpleMath::Color m_endColor;       // 終了色

		// --- 属性 ---
		int m_type;          // パーティクルの種類
	};
}