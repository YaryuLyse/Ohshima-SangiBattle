/**
* @file TitleScene.cpp
* @brief タイトル画面を管理するクラスの実装ファイル
* @brief ゲーム起動後のメニュー選択やシーン遷移の処理をまとめています
* @brief このクラスは、タイトルロゴやメニューUIの描画、キー入力による選択項目の切り替え、および各ゲームシーンへの遷移を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "TitleScene.h"
#include "Game/Scene/GameScene.h"
#include "TutorialScene.h"

using namespace DirectX;

/**
* @brief タイトルシーンの初期化処理。リソースの作成などを行います
*/
void TitleScene::Initialize()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

/**
* @brief タイトルシーンの更新処理。項目の選択状態や矢印のアニメーション、シーン遷移判定を行います
* @param elapsedTime : 経過時間
*/
void TitleScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);
	auto kb = Keyboard::Get().GetState();
	m_keyboardTracker.Update(kb);
	// 現在のキーボード（またはパッド）の状態を取得
	bool isAttackButtonPressed = kb.Z;

	// ロック状態の処理
	if (m_isInputLocked) 
	{
		if (!isAttackButtonPressed) 
		{
			// ボタンが離されたのを確認したらロック解除！
			m_isInputLocked = false;
		}
		return; // ロック中はここで処理を終わらせて、攻撃させない
	}

	if (m_keyboardTracker.pressed.Right)
	{
		m_count = 1;
	}
	if (m_keyboardTracker.pressed.Left)
	{
		m_count = 0;
	}
	if (m_keyboardTracker.IsKeyPressed(Keyboard::Z))
	{
		if (m_count == 0) 
		{
			ChangeScene<Lyse::GameScene>();
			
		}
		else if(m_count == 1)
		{
			ChangeScene<Lyse::TutorialScene>();
		}
	}
	m_time += elapsedTime;
	if (m_time < 1.0f) 
	{
		m_posX += 0.2f;
	}
	else if (m_time < 2.0f) 
	{
		m_posX -= 0.2f;
	}
	else 
	{
		m_time = 0;
	}
}

/**
* @brief タイトルシーンの描画処理。背景や選択状態に応じたUI、動く矢印を描画します
*/
void TitleScene::Render()
{
	//auto* debugFont = GetUserResources()->GetDebugFont();
	//debugFont->AddString(L"TitleScene", SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
	//debugFont->AddString(L"Press Enter to Gameplay", SimpleMath::Vector2(0.0f, debugFont->GetFontHeight() * 2));
	m_spriteBatch->Begin();
	if (m_count == 0) 
	{
		m_spriteBatch->Draw(
			m_titleTex1.Get(),                      // テクスチャ
			DirectX::SimpleMath::Vector2(0.0f, 0.0f) // 描画位置
		);
	}
	else 
	{
		m_spriteBatch->Draw(
			m_titleTex2.Get(),                      // テクスチャ
			DirectX::SimpleMath::Vector2(0.0f, 0.0f) // 描画位置
		);
	}
	m_spriteBatch->Draw(
		m_arrowTex.Get(),                      // テクスチャ
		DirectX::SimpleMath::Vector2(150.0f + m_posX, 410.0f) // 描画位置
	);
	m_spriteBatch->End();

}

/**
* @brief タイトルシーンの終了処理
*/
void TitleScene::Finalize()
{
}

/**
* @brief デバイスに依存するリソース（スプライトバッチ、タイトル用テクスチャ等）の作成処理
*/
void TitleScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();

	// 初期化
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// テクスチャ読み込み（WICTextureLoader使う場合）
	DirectX::CreateWICTextureFromFile(device, L"Resources/Texture/Title_sangi1.png", nullptr, m_titleTex1.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources/Texture/Title_sangi2.png", nullptr, m_titleTex2.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources/Texture/yagirusi.png", nullptr, m_arrowTex.GetAddressOf());
}

/**
* @brief ウィンドウサイズに依存するリソースの作成処理
*/
void TitleScene::CreateWindowSizeDependentResources()
{
}

/**
* @brief グラフィックスデバイスが失われた際の解放処理
*/
void TitleScene::OnDeviceLost()
{
}