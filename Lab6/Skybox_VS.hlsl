cbuffer VPBuffer : register(b0)
{
    matrix vp;
};

struct VS_INPUT
{
    float3 position : POSITION;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 texcoord : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    float4 pos = float4(input.position, 1.0);
    output.position = mul(pos, vp);
    output.texcoord = input.position;
    return output;
}
