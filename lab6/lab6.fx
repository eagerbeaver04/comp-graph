cbuffer ModelBuffer : register(b0)
{
    matrix m;
    matrix normal_m;
};

cbuffer VPBuffer : register(b1)
{
    matrix vp;
};

cbuffer LightBuffer : register(b2)
{
    struct Light {
        float4 Position;
        float4 Color;
        float4 Attenuation;
    } Lights[2];
};

cbuffer CameraBuffer : register(b3)
{
    float4 CameraPosition;
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(float4(input.pos, 1.0), m);
    output.worldPos = worldPos.xyz;
    output.pos = mul(worldPos, vp);
    output.normal = mul(input.normal, (float3x3)normal_m);
    output.tex = input.tex;
    return output;
}

Texture2D cubeTex : register(t0);
SamplerState samLinear : register(s0);

float4 PS(PS_INPUT input) : SV_Target
{
    input.normal = normalize(input.normal);

    float3 ambient = float3(0.1, 0.1, 0.1);

    float3 diffuse = float3(0, 0, 0);
    for (int i = 0; i < 2; i++)
    {
        float3 lightDir = Lights[i].Position.xyz - input.worldPos;
        float distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = 1.0 / (Lights[i].Attenuation.x +
                                  Lights[i].Attenuation.y * distance +
                                  Lights[i].Attenuation.z * distance * distance);

        float diff = max(dot(input.normal, lightDir), 0.0);
        diffuse += Lights[i].Color.rgb * diff * attenuation;
    }

    float3 texColor = cubeTex.Sample(samLinear, input.tex).rgb;
    float3 result = (ambient + diffuse) * texColor;
    return float4(result, 1.0);
}