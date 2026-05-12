#pragma once
/**
* @file ResultScene.h
* @brief リザルト画面を管理するクラスのヘッダーファイル
* @brief 戦闘終了後の結果表示やシーン遷移の処理をまとめています
* @brief このクラスは、リザルト用のテクスチャロード、勝敗テキストの表示、およびタイトル画面への遷移入力を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "LyseLib/SceneManager.h"
#include "Game/Common/UserResources.h"

class ResultScene : public Lyse::Scene<UserResources>
{
public:

	// 初期化
	void Initialize() override;

	// 更新
	void Update(float elapsedTime) override;

	// 描画
	void Render() override;

	// 終了処理
	void Finalize() override;

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources() override;

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources() override;

	// デバイスロストした時に呼び出される関数
	void OnDeviceLost() override;
private:

	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_resultTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_winTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_loseTex;

	// 浮遊アニメーション用
	float m_floatTimer = 0.0f;
};

