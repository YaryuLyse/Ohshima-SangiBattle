#pragma once
/**
* @file Controller.h
* @brief コントローラーを管理するクラスのヘッダーファイル
* @author 大嶋　琉也
* @date 2026/02/25
*/

#include "pch.h"
#include "Game/Common/UserResources.h"
#include "SimpleMath.h"

namespace Lyse
{
	// --- 前方宣言 (Forward Declarations) ---
	class Character;
	class Controller
	{
	public:
		// デストラクタ
		virtual ~Controller() = default;

		// --- 純粋仮想関数（実装必須） ---

		// 初期化処理（必要に応じてオーバーライド）
		virtual void Initialize(Character* myBody) = 0;

		// 毎フレーム呼ばれる思考・入力処理

		virtual void Update(Character* myBody, float elapsedTime) = 0;

	};
}