cbuffer ConstBuffer : register(b1)
{
    matrix matWorld;
    matrix matView;
    matrix matProj;
    float4 color;
    float4 cameraPos;
    float4 time;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 ViewDir : TEXCOORD;
    
};