#pragma once
/**
* @file TitleScene.h
* @brief タイトル画面を管理するクラスのヘッダーファイル
* @brief ゲーム起動後のメニュー選択やシーン遷移の処理をまとめています
* @brief このクラスは、タイトルロゴやメニューUIの描画、キー入力による選択項目の切り替え、および各ゲームシーンへの遷移を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/
#include "LyseLib/SceneManager.h"
#include "Game/Common/UserResources.h"

class TitleScene : public Lyse::Scene<UserResources>
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
	// タイトルロゴの描画用スプライトバッチとテクスチャ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_titleTex1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_titleTex2;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_arrowTex;
	float m_posX = 0.0f;
	float m_time = 0.0f;
	int m_count = 0;
	// キーボード入力トラッカー
	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	// 入力ロックのフラグ
	bool m_isInputLocked = true;
};

