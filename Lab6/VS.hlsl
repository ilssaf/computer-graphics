cbuffer ModelBuffer : register(b0)
{
    matrix model;
};

cbuffer VPBuffer : register(b1)
{
    matrix viewProj;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 worldPos = mul(float4(input.Pos, 1.0), model);
    output.Pos = mul(worldPos, viewProj);
    output.WorldPos = worldPos.xyz;
    output.Normal = mul(input.Normal, (float3x3) model);
    output.TexCoord = input.TexCoord;
    return output;
}