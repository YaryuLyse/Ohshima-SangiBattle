#include "Model.hlsli"
Texture2D ModelTex : register(t0);
SamplerState ModelSampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    // 法線のY成分（上下成分）の絶対値を取る
    // 1.0に近いほど「床（天井）」、0.0に近いほど「壁」
    //float upComponent = abs(input.Normal.y);

    // 色の定義
   // float4 wallColor = float4(1.0f, 1.0f, 1.0f, 1.0f); // 壁：白
    //float4 floorColor = float4(0.0f, 0.0f, 0.0f, 1.0f); // 床：黒

    // 判定ロジック
    // Y成分が 0.8 以上なら「床」、それ以外は「壁」とする
    // step(閾値, 値) は、値 >= 閾値 なら 1、そうでなければ 0 を返します
    //float isFloor = step(0.8f, upComponent);

    // lerpで混ぜる（isFloorが1ならfloorColor、0ならwallColorになる）
    ///float4 finalColor = lerp(wallColor, floorColor, isFloor);

    // もしテクスチャも反映したい場合（壁だけテクスチャなど）はこのように書けます：
    
    float4 texColor = ModelTex.Sample(ModelSampler, input.Tex);
    
    texColor.rgb += flashColor.rgb;
    //float4 finalColor = lerp(texColor, floorColor, isFloor); 
    

    return texColor;
}