#include "Model.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    float4 worldPos = mul(float4(input.Pos, 1.0f), matWorld);
    output.Pos = mul(worldPos, matView);
    output.Pos = mul(output.Pos, matProj);

    // Normal ‚Ì•ÏŠ·i³‹K‰»{s—ñ‚Ì 3x3 •”•ªj
    float3 worldNormal = mul(input.Normal, (float3x3) matWorld);
    output.Normal = normalize(worldNormal);

    output.Tex = input.Tex;
    
    
    return output;
}