#pragma once
/**
* @file TPSCamera.h
* @brief 三人称視点カメラの挙動を管理するクラスのヘッダーファイル
* @brief プレイヤーと敵の位置関係に基づいたカメラの座標計算や注視制御をまとめています
* @brief このクラスは、対象となるモデルの座標から適切なオフセットを算出し、LookAt行列（ビュー行列）を生成することで、常に戦闘状況を把握しやすい視点を提供します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include "SimpleMath.h"

namespace Lyse
{
    class Character;

    class TPSCamera
    {
    public:
        TPSCamera(int windowWidth, int windowHeight);
        //でバッグカメラの更新
        void Update();
        //デバッグカメラビュー行列の取得関数
        DirectX::SimpleMath::Matrix GetCameraMatrix();
        //デバッグカメラの位置の取得関数
        DirectX::SimpleMath::Vector3 GetEyePosition();

        //デバッグカメラの注視点の取得関数
        DirectX::SimpleMath::Vector3 GetTargetPosition();

		DirectX::SimpleMath::Vector3 GetUpVector();

        //画面サイズの設定関数
        void SetWindowSize(int windowWidth, int windowHeght);
        //画面サイズの取得関数
        void GetWindowSize(int &windowWidth, int &windowHeght);

        void SetTarget(Character& target);
        void SetPosition(Character& player);
        void UpdateIntro(float elapsedTime); // イントロ用更新
        bool IsIntroDone() const { return m_introAngle >= DirectX::XM_2PI; }
    private:
        // 横回転
        float m_yAngle, m_yTmp;

        // 縦回転
        float m_xAngle, m_xTmp;
        // カメラの距離
        static const float DEFAULT_CAMERA_DISTANCE;
        //注視点
        DirectX::SimpleMath::Vector3 m_targetPos;
        DirectX::SimpleMath::Vector3 m_pos;

		Character* m_enemy = nullptr;
		Character* m_player = nullptr;

        // 視点
        DirectX::SimpleMath::Vector3 m_eye;

        // 注視点
        DirectX::SimpleMath::Vector3 m_target;

		// 上方向ベクトル
		DirectX::SimpleMath::Vector3 m_up = DirectX::SimpleMath::Vector3(0,0,0);

        // 生成されたビュー行列
        DirectX::SimpleMath::Matrix m_view;

        // マウストラッカー
        DirectX::Mouse::ButtonStateTracker m_tracker;
        float m_introAngle = 0.0f; // 周回角度（0→2π）
    };
}
