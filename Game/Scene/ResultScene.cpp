/**
* @file ResultScene1.cpp
* @brief リザルト画面を管理するクラスの実装ファイル
* @brief 戦闘終了後の結果表示やシーン遷移の処理をまとめています
* @brief このクラスは、リザルト用のテクスチャロード、勝敗テキストの表示、およびタイトル画面への遷移入力を管理します。
* @author 大嶋　琉也
* @date 2026/02/17
*/

#include "pch.h"
#include "ResultScene.h"
#include "GameScene.h"
#include "TitleScene.h"	

using namespace DirectX;

namespace
{
	// --- 定数定義 ---
	
	// 浮遊オフセット（振幅30px、速さは調整可）
	constexpr float FLOAT_AMPLITUDE = 30.0f;  // 上下の幅(px)
	constexpr float FLOAT_SPEED = 2.0f;   // 速さ
}


/**
* @brief リザルトシーンの初期化処理
*/
void ResultScene::Initialize()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	m_floatTimer = 0.0f; // 浮遊アニメーションのタイマー初期化
}

/**
* @brief リザルトシーンの更新処理
* @param elapsedTime : 経過時間
*/
void ResultScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	auto kb = GetUserResources()->GetKeyboardStateTracker();

	m_floatTimer += elapsedTime;

	if (kb->pressed.Z)
	{
		ChangeScene<TitleScene>();
	}
}

/**
* @brief リザルトシーンの描画処理
*/
void ResultScene::Render()
{
	auto* debugFont = GetUserResources()->GetDebugFont();
	debugFont->AddString(L"ResultScene", SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
	auto data = GetUserResources();

	float offsetY = FLOAT_AMPLITUDE * sinf(m_floatTimer * FLOAT_SPEED);

	// 画面中央に表示する位置（解像度に合わせて調整）
	SimpleMath::Vector2 center(640.0f, 360.0f + offsetY);
	
	m_spriteBatch->Begin();

	m_spriteBatch->Draw(
		m_resultTex.Get(),                      // テクスチャ
		DirectX::SimpleMath::Vector2(0.0f, 0.0f) // 描画位置
	);

	//勝っていたらWin負けていたらLoseと表示する
	if(data->GetIsWin()) 
	{
		m_spriteBatch->Draw(
			m_winTex.Get(),                      // テクスチャ
			center // 描画位置
		);
	}
	else 
	{
		m_spriteBatch->Draw(
			m_loseTex.Get(),                      // テクスチャ
			center // 描画位置
		);
	}
	m_spriteBatch->End();


}

/**
* @brief リザルトシーンの終了処理
*/
void ResultScene::Finalize()
{
}

/**
* @brief デバイスに依存するリソース（テクスチャ等）の作成処理
*/
void ResultScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();

	// 初期化
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// テクスチャ読み込み（WICTextureLoader使う場合）
	DirectX::CreateWICTextureFromFile(device, L"Resources/Texture/Back.png", nullptr, m_resultTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources/Texture/win.png", nullptr, m_winTex.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources/Texture/lose.png", nullptr, m_loseTex.GetAddressOf());
}

/**
* @brief ウィンドウサイズに依存するリソースの作成処理
*/
void ResultScene::CreateWindowSizeDependentResources()
{
}

/**
* @brief デバイスロスト時の解放処理
*/
void ResultScene::OnDeviceLost()
{
}