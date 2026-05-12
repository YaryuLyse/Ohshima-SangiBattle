/**
* @file CollisionDraw.h
* @brief コリジョンのデバッグ描画を管理するクラスのヘッダーファイル
* @brief 当たり判定の視認化を目的とした描画処理をまとめています
* @brief このクラスは、球体などのコリジョン形状をワイヤーフレームとして描画し、開発時の判定確認をサポートします。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#pragma once

#include "Game/Common/UserResources.h"

#include <vector>
#include "Game/GameObjects/Collider/SphereCollider.h"
#include "SimpleMath.h"
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <SimpleMath.h>
#include <DirectXColors.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace Lyse {
    class CollisionDraw
    {
    public:

        /**
        * @brief 球体コリジョンの描画処理
        * @param batch : プリミティブバッチの参照
        * @param center : 球体の中心座標
        * @param radius : 球体の半径
        * @param color : 描画色
        * @param segments : 円を構成する分割数（精度）
        */
        void DrawSphereCollider(PrimitiveBatch<VertexPositionColor>& batch,
            const Vector3& center,
            float radius,
            FXMVECTOR color,
            int segments = 16)
        {
            // XY平面
            for (int i = 0; i < segments; ++i)
            {
                float t1 = XM_2PI * i / segments;
                float t2 = XM_2PI * (i + 1) / segments;
                batch.DrawLine(
                    VertexPositionColor(center + Vector3(radius * cosf(t1), radius * sinf(t1), 0), color),
                    VertexPositionColor(center + Vector3(radius * cosf(t2), radius * sinf(t2), 0), color)
                );
            }
            // XZ平面
            for (int i = 0; i < segments; ++i)
            {
                float t1 = XM_2PI * i / segments;
                float t2 = XM_2PI * (i + 1) / segments;
                batch.DrawLine(
                    VertexPositionColor(center + Vector3(radius * cosf(t1), 0, radius * sinf(t1)), color),
                    VertexPositionColor(center + Vector3(radius * cosf(t2), 0, radius * sinf(t2)), color)
                );
            }
            // YZ平面
            for (int i = 0; i < segments; ++i)
            {
                float t1 = XM_2PI * i / segments;
                float t2 = XM_2PI * (i + 1) / segments;
                batch.DrawLine(
                    VertexPositionColor(center + Vector3(0, radius * cosf(t1), radius * sinf(t1)), color),
                    VertexPositionColor(center + Vector3(0, radius * cosf(t2), radius * sinf(t2)), color)
                );
            }
        }
    };
}