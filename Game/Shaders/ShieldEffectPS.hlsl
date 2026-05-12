#include "Effect.hlsli"
Texture2D ModelTex : register(t1);
SamplerState ModelSampler : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
   // 1. ベクトルの準備
    // 法線と、カメラへの向き(ViewDir)を正規化
    float3 N = input.Normal;
    float3 V = input.ViewDir;

    // 2. フレネル計算（輪郭抽出）
    // dot(N, V) は「向きがどれだけ合っているか」
    // 正面＝1.0, フチ＝0.0
    float dotNV = dot(N, V);

    // 1.0 から引くことで、「フチ＝1.0（白）, 正面＝0.0（黒）」に反転させる
    // saturate はマイナスになるのを防ぐ安全装置
    float rim = 1.0 - saturate(dotNV);

    // 3. 光の強さと鋭さを調整
    // powの数字(3.0)を大きくすると、線が細く鋭くなる
    float fresnel = pow(rim, 2.0);

    // ★追加演出：時間(time.x)を使って、強さをゆらゆらさせる
    // sin波で 0.8倍 ～ 1.2倍 の間で明るさを揺らす
    float pulse = 1.0 + sin(time.x * 2.0) * 0.2;
    
    // 4. 最終カラー決定
    // 色 × フレネル強度 × 点滅
    float3 finalRGB = color.rgb * fresnel * pulse;

    // アルファは1.0でOK（Additive合成なら黒＝透明になるため）
    return float4(finalRGB, 1.0);
}