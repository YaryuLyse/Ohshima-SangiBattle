#include "Particle.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
Texture2D tex3 : register(t2);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	
	// --- 計算ロジック ---
    // input.Type が 0.5 以上 (つまり 1 か 2) なら 1.0 になる係数
    float isType1Or2 = step(0.5f, input.Type);

    // input.Type が 1.5 以上 (つまり 2) なら 1.0 になる係数
    float isType2 = step(1.5f, input.Type);

    // 1段階目: 0ならtex0、1以上ならtex1 を選ぶ
    // lerp(A, B, 0) -> A
    // lerp(A, B, 1) -> B
    float4 finalColor = lerp(tex.Sample(samLinear, input.Tex), tex2.Sample(samLinear, input.Tex), isType1Or2);

    // 2段階目: さっきの結果と tex2 を混ぜる
    // IDが2の場合だけ isType2 が 1 になるので tex2 に置き換わる
    finalColor = lerp(finalColor, tex3.Sample(samLinear, input.Tex), isType2);

    return finalColor;
	
}