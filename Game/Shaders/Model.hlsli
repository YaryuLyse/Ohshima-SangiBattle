cbuffer ConstBuffer : register(b1)
{
    matrix matWorld;
    matrix matView;
    matrix matProj;
    float4 flashColor;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD;
    
};