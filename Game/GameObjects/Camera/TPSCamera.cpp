/**
* @file TPSCamera.cpp
* @brief 三人称視点カメラの挙動を管理するクラスの実装ファイル
* @brief プレイヤーと敵の位置関係に基づいたカメラの座標計算や注視制御をまとめています
* @brief このクラスは、対象となるモデルの座標から適切なオフセットを算出し、LookAt行列（ビュー行列）を生成することで、常に戦闘状況を把握しやすい視点を提供します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "TPSCamera.h"
#include "Mouse.h"
#include "Game/GameObjects/Character/Character.h"

using namespace DirectX;

// 定数定義（マジックナンバーの排除）
namespace 
{
	const float DEFAULT_CAMERA_DISTANCE = 5.0f;
	// ステージ中心を見ながら一周する
	constexpr float INTRO_SPEED = DirectX::XM_2PI / 3.0f; // 3秒で一周
	constexpr float CAMERA_HEIGHT = 12.0f;
	constexpr float CAMERA_RADIUS = 20.0f;
}

/**
* @brief コンストラクタ。ウィンドウサイズの設定とマウス状態のリセットを行う
* @param windowWidth : ウィンドウの横幅
* @param windowHeight : ウィンドウの高さ
*/
Lyse::TPSCamera::TPSCamera(int windowWidth, int windowHeight)
	:m_yAngle(0.0f), m_yTmp(0.0f), m_xAngle(0.0f), m_xTmp(0.0f)
{
	//ウィンドウの初期値を設定
	SetWindowSize(windowWidth, windowHeight);
	//マウスのホイール値をリセット
	Mouse::Get().ResetScrollWheelValue();
}

/**
* @brief カメラの更新処理。プレイヤーと敵の位置からビュー行列を算出し、視点を確定させる
*/
void Lyse::TPSCamera::Update()
{
	auto mouse = Mouse::Get().GetState();
	// ビュー行列を算出する
	SimpleMath::Matrix rotY = SimpleMath::Matrix::CreateRotationY(m_yTmp);
	SimpleMath::Matrix rotX = SimpleMath::Matrix::CreateRotationX(m_xTmp);

	SimpleMath::Vector3 target(0.0f, 0.0f, 0.0f);

	if (m_enemy != nullptr)
	{
		m_targetPos = m_enemy->GetPosition();
	}
	if (m_player != nullptr)
	{
		m_pos = m_player->GetPosition();
	}
	target = m_targetPos;

	DirectX::SimpleMath::Vector3 off = DirectX::SimpleMath::Vector3(0.0f, 2.0f, -10.0f);
	float rad = atan2(m_targetPos.x - m_pos.x, m_targetPos.z - m_pos.z);
	DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(rad);
	off = DirectX::SimpleMath::Vector3::Transform(off, rotMat);

	// 初期のカメラ位置（例：背後の方向ベクトル）
	DirectX::SimpleMath::Vector3 offset = DirectX::SimpleMath::Vector3(m_pos.x + off.x, 3.0f + off.y, m_pos.z + off.z);  // 斜め後ろ上

	//// カメラ回転行列
	DirectX::SimpleMath::Matrix rt = DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(0.0f, 0.0f, 0.0f);

	// オフセットをカメラの回転で変換
	offset = DirectX::SimpleMath::Vector3::TransformNormal(offset, rt);

	// 一定距離にスケーリング
	//offset *= DEFAULT_CAMERA_DISTANCE;

	// カメラ位置 = ターゲット位置 + オフセット
	DirectX::SimpleMath::Vector3 eye = offset;

	// 上方向ベクトル（ワールドY軸）
	DirectX::SimpleMath::Vector3 up = DirectX::SimpleMath::Vector3::TransformNormal(DirectX::SimpleMath::Vector3::Up, rt);

	m_eye = eye;
	m_target = target;
	m_up = up;

	m_view = SimpleMath::Matrix::CreateLookAt(eye, target, up);
}

/**
* @brief ステージ開始時のイントロ演出用の更新処理。カメラを円形に動かしてステージ全体を見せる
* @param elapsedTime : 経過時間
*/
void Lyse::TPSCamera::UpdateIntro(float elapsedTime)
{
	m_introAngle += INTRO_SPEED * elapsedTime;
	m_introAngle = std::min(m_introAngle, DirectX::XM_2PI); // 一周で止まる

	// 円形にカメラを動かす
	float eyeX = sinf(m_introAngle) * CAMERA_RADIUS;
	float eyeZ = cosf(m_introAngle) * CAMERA_RADIUS;

	m_eye = DirectX::SimpleMath::Vector3(eyeX, CAMERA_HEIGHT, eyeZ);
	m_target = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f); // ステージ中心
	m_up = DirectX::SimpleMath::Vector3::Up;

	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(m_eye, m_target, m_up);
}

/**
* @brief 生成されたビュー行列を取得する
* @return DirectX::SimpleMath::Matrix : ビュー行列
*/
DirectX::SimpleMath::Matrix Lyse::TPSCamera::GetCameraMatrix()
{
	return m_view;
}

/**
* @brief 現在のカメラ（視点）座標を取得する
* @return DirectX::SimpleMath::Vector3 : カメラの座標
*/
DirectX::SimpleMath::Vector3 Lyse::TPSCamera::GetEyePosition()
{
	return m_eye;
}

/**
* @brief 現在のカメラの注視点座標を取得する
* @return DirectX::SimpleMath::Vector3 : 注視点の座標
*/
DirectX::SimpleMath::Vector3 Lyse::TPSCamera::GetTargetPosition()
{
	return m_target;
}

/**
* @brief カメラの上方向ベクトルを取得する
* @return DirectX::SimpleMath::Vector3 : 上方向ベクトル
*/
DirectX::SimpleMath::Vector3 Lyse::TPSCamera::GetUpVector()
{
	return m_up;
}

/**
* @brief ウィンドウサイズを設定する
* @param windowWidth : ウィンドウの横幅
* @param windowHeght : ウィンドウの高さ
*/
void Lyse::TPSCamera::SetWindowSize(int windowWidth, int windowHeght)
{
	UNREFERENCED_PARAMETER(windowWidth);
	UNREFERENCED_PARAMETER(windowHeght);
}

/**
* @brief ウィンドウサイズを取得する
* @param windowWidth : [out] 横幅の格納先
* @param windowHeght : [out] 高さの格納先
*/
void Lyse::TPSCamera::GetWindowSize(int& windowWidth, int& windowHeght)
{
	UNREFERENCED_PARAMETER(windowWidth);
	UNREFERENCED_PARAMETER(windowHeght);
}

/**
* @brief 追従対象となる敵モデルを設定する
* @param enemy : 敵モデルの参照
*/
void Lyse::TPSCamera::SetTarget(Character& target)
{
	m_enemy = &target;
}

/**
* @brief 基準点となるプレイヤーモデルを設定する
* @param player : プレイヤーモデルの参照
*/
void Lyse::TPSCamera::SetPosition(Character& player)
{
	m_player = &player;
}