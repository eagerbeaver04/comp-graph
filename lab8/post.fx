Texture2D sceneTex : register(t0);
SamplerState samLinear : register(s0);

struct VSOutput {
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 PS(VSOutput input) : SV_Target{
    float4 color = sceneTex.Sample(samLinear, input.TexCoord);

    float gray = dot(color.rgb, float3(0.3, 0.3, 0.3));
    return float4(gray, gray, gray, color.a);
}

struct FSInput {
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

VSOutput VS(FSInput input) {
    VSOutput output;
    output.Pos = input.Pos;
    output.TexCoord = input.Tex;
    return output;
}