#include "Model.hlsli"
Texture2D ModelTex : register(t1);
SamplerState ModelSampler : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
    return ModelTex.Sample(ModelSampler, input.Tex);
}