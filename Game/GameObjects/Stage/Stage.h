#pragma once
/**
* @file Stage.h
* @brief ステージ（フィールド）の構成要素やギミックを管理するクラスのヘッダーファイル
* @brief 壁のセグメント管理や破壊時の演出処理をまとめています
* @brief このクラスは、複数の壁モデルのロード、破壊フラグに応じたモデルの切り替え、および破壊時の発光（フラッシュ）タイマーや衝突判定の遅延発生を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <SimpleMath.h>
#include <Effects.h>
#include <Model.h>
#include <CommonStates.h>
#include <vector>
#include <array>
#include <memory>
#include <wrl/client.h>
#include "Game/GameObjects/Collider/StageBreakCollision.h"

namespace Lyse
{
	class ShaderManager;

	class Stage
	{
	public:
		// 定数定義
		static const int WALL_COUNT = 10;

	private:
		/// <summary>
		/// 壁の各セグメント情報
		/// </summary>
		struct WallSegment
		{
			std::unique_ptr<DirectX::Model> m_normalModel;		 // 通常モデル
			std::unique_ptr<DirectX::Model> m_brokenModel;		 // 壊れたモデル
			bool  m_isBroken = false;			 // 壊れているかどうか
			float m_flashTimer = 0.0f;			 // 発光タイマー
			float m_collisionSpawnTimer= 0.0f; // 衝突判定発生までのタイマー
			bool  m_isColliding = 0.0f;		 // 衝突発生待ちフラグ
		};

	public:
		// コンストラクタ / デストラクタ
		Stage(ID3D11Device1* device, DirectX::DX11::EffectFactory& fx);
		~Stage();

		// --- 初期化・終了 ---
		void Initialize(ShaderManager* shaderManager);
		void Finalize();

		// --- 更新・描画 ---
		void Update(float elapsedTime);
		void Render(ID3D11DeviceContext1* context, DirectX::CommonStates* states, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

		// --- アクション ---
		void BreakWall(int index); // 指定したインデックスの壁を壊す

		// --- アクセサ ---
		const WallSegment& GetWall(int index) const
		{
			return m_walls[index];
		}

		// 衝突判定配列への参照を取得
		std::array<Lyse::StageBreakCollision, WALL_COUNT>& GetBreakCollisions()
		{
			return m_stageBreakCollisions;
		}

	private:
		// 定数データ
		static const DirectX::VertexPositionTexture VERTICES[4];

		// --- 変換パラメータ ---
		DirectX::SimpleMath::Vector3 m_position;
		DirectX::SimpleMath::Vector3 m_prePosition;
		DirectX::SimpleMath::Vector3 m_scale;
		DirectX::SimpleMath::Vector3 m_rotation;

		DirectX::SimpleMath::Vector3 m_attackVec;

		// --- DirectX リソース ---
		// 入力レイアウト
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		// 共通ステート
		std::unique_ptr<DirectX::CommonStates> m_states;

		// テクスチャ
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

		// エフェクト（プリミティブ用）
		std::unique_ptr<DirectX::AlphaTestEffect> m_batchEffect;

		// プリミティブバッチ
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_batch;

		// --- モデルリソース ---
		std::unique_ptr<DirectX::Model> m_modelStage; // ステージ全体（現在は未使用の可能性あり）

		// 壁セグメント管理
		std::vector<WallSegment> m_walls;

		// 壁の破壊判定オブジェクト
		std::array<Lyse::StageBreakCollision, WALL_COUNT> m_stageBreakCollisions;

		// 壁の配置座標リスト
		std::vector<DirectX::SimpleMath::Vector3> m_wallPositions
		{
			{  4.6f, 0.5f, -12.6f },   //左斜め前
			{  11.5f, 0.5f,  -8.0f },
			{  14.0f, 0.5f,   0.4f },
			{  11.2f, 0.5f,   8.3f },
			{   5.2f, 0.5f,  13.0f },
			{  -4.3f, 0.5f,  13.2f },
			{ -11.2f, 0.5f,   8.3f },
			{ -14.0f, 0.5f,  -0.1f },
			{ -11.3f, 0.5f,  -8.3f },
			{  -5.4f, 0.5f, -12.8f }
		};

		// --- 外部参照 ---
		ShaderManager* m_shaderManager;

	};
}