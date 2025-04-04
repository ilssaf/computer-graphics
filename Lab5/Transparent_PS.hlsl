cbuffer TransparentBuffer : register(b0)
{
    float4 g_Color;
};

float4 main() : SV_TARGET
{
    return g_Color;
}