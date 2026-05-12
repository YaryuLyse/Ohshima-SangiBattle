/**
* @file MoveForwardState.cpp
* @brief 敵の前進状態を管理するクラスの実装ファイル
* @brief 敵が前進移動を行う際の処理をまとめています
* @brief このクラスは、敵が前進状態に入ったときの移動計算や座標更新を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "MoveForwardState.h"
#include "IdleState.h"
#include "Game/GameObjects/Character/Character.h"

// 定数定義（マジックナンバーの排除）
namespace 
{
	constexpr float FORWARD_MOVE_Z = 0.1f; // 後退時のZ軸移動量
}

/**
* @brief コンストラクタ
* @param enemy : 敵モデルへのポインタ
*/
MoveForwardState::MoveForwardState(Lyse::Character* chara)
	: m_chara(chara)
{
}

/**
* @brief 初期化処理
*/
void MoveForwardState::Initialize()
{
}

/**
* @brief 更新前処理
*/
void MoveForwardState::PreUpdate()
{
}

/**
* @brief 敵の前進移動の更新処理
* @param keyboardStateTracker : キーボードの状態トラッカー
* @param elapsedTime : 経過時間
*/
void MoveForwardState::Update(const DirectX::Keyboard::KeyboardStateTracker& keyboardStateTracker, float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime); // 経過時間はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	UNREFERENCED_PARAMETER(keyboardStateTracker); // キーボード入力はこのステートでは使用しないため、未使用パラメータとしてマクロで処理
	// ベクトルの初期化
	m_moveOffset = DirectX::SimpleMath::Vector3::Zero;

	// 後退方向（Z軸マイナス）への移動量を設定
	m_moveOffset = DirectX::SimpleMath::Vector3(0.0f, 0.0f, FORWARD_MOVE_Z);

	// 敵の回転に合わせて移動ベクトルを変換
	m_moveOffset = DirectX::SimpleMath::Vector3::Transform(m_moveOffset, m_chara->GetRotMat());

	// 現在位置に移動量を加算して更新
	m_chara->SetPosition(m_chara->GetPosition() + m_moveOffset);
}

/**
* @brief 更新後処理
*/
void MoveForwardState::PostUpdate()
{
}

/**
* @brief 描画処理
*/
void MoveForwardState::Render()
{
}

/**
* @brief 終了処理
*/
void MoveForwardState::Finalize()
{
}