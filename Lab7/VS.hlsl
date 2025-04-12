#define NUM_INSTANCES 11
#define NUM_TEX 3

cbuffer ModelBuffer : register(b0)
{
    matrix models[NUM_INSTANCES];
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
    uint   TexIndex : TEXCOORD3;
};

VS_OUTPUT main(VS_INPUT input, uint instanceID : SV_InstanceID)
{
    VS_OUTPUT output;
    float4 worldPos = mul(float4(input.Pos, 1.0), models[instanceID]);
    output.Pos = mul(worldPos, viewProj);
    output.WorldPos = worldPos.xyz;
    output.Normal = mul(input.Normal, (float3x3)models[instanceID]);
    output.TexCoord = input.TexCoord;
    output.TexIndex = instanceID % NUM_TEX;
    return output;
}

