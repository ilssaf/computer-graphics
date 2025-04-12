struct FS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

VS_OUTPUT main(FS_INPUT input)
{
    VS_OUTPUT output;
    output.Pos = input.Pos;
    output.TexCoord = input.Tex;
    return output;
}