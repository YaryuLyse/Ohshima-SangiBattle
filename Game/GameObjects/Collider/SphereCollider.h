#pragma once
/**
* @file SphereCollider.h
* @brief 当たり判定を管理するクラスのヘッダーファイル
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "Game/Common/UserResources.h"
#include "SimpleMath.h"

namespace Lyse
{
	class SphereCollider
	{
	public:
		// デストラクタ
		virtual ~SphereCollider() = default;

		// --- 純粋仮想関数（実装必須） ---

		// 中心座標の取得
		virtual DirectX::SimpleMath::Vector3 GetPosition() const = 0;

		// 衝突半径の取得（球体/円形判定用）
		virtual float GetRadius() const = 0;

		// 衝突時のコールバック処理
		virtual void OnCollision(SphereCollider* other) = 0;


		// --- 仮想関数（必要に応じてオーバーライド） ---

		// 座標の設定
		virtual void SetPosition(const DirectX::SimpleMath::Vector3& /*position*/) {}
	};
}