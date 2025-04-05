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

VS_OUTPUT VS(FS_INPUT input)
{
    VS_OUTPUT output;
    output.Pos = input.Pos;
    output.TexCoord = input.Tex;
    return output;
}

Texture2D sceneTex : register(t0);
SamplerState samLinear : register(s0);

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 color = sceneTex.Sample(samLinear, input.TexCoord);

    float gray = dot(color.rgb, float3(0.300, 0.300, 0.300));
    return float4(gray, gray, gray, color.a);
}
