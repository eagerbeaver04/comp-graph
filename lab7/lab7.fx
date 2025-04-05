cbuffer ModelBuffer : register(b0)
{
    matrix m;
};

cbuffer VPBuffer : register(b1)
{
    matrix vp;
};

cbuffer LightBuffer : register(b3)
{
    struct Light {
        float4 Position;
        float4 Color;
        float4 Attenuation;
    } Lights[2];
};

cbuffer CameraBuffer : register(b4)
{
    float4 CameraPosition;
};

cbuffer InstanceBuffer : register(b2)
{
    struct InstanceData {
        matrix model;
        int textureIndex;
        float3 padding;
    } instances[100];
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 tex : TEXCOORD;
    uint instanceID : SV_InstanceID;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3x3 tbn : TBN;
    int TexIndex : TEXINDEX;
};

// Массив текстур вместо отдельных переменных
Texture2D textures[3] : register(t0);
Texture2D normalMap : register(t3);
SamplerState samLinear : register(s0);

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    matrix model = instances[input.instanceID].model;
    output.TexIndex = instances[input.instanceID].textureIndex;

    float4 worldPos = mul(float4(input.pos, 1.0), model);
    output.worldPos = worldPos.xyz;
    output.pos = mul(worldPos, vp);
    output.tex = input.tex;

    float3 T = normalize(mul(input.tangent, (float3x3)model));
    float3 N = normalize(mul(input.normal, (float3x3)model));
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T);

    output.tbn = float3x3(T, B, N);

    return output;
}

float3 GetTextureColor(int index, float2 uv)
{
    switch (index)
    {
    case 0: return textures[0].Sample(samLinear, uv).rgb;
    case 1: return textures[1].Sample(samLinear, uv).rgb;
    case 2: return textures[2].Sample(samLinear, uv).rgb;
    default: return float3(1, 1, 1);
    }
}

float4 PS(PS_INPUT input) : SV_Target
{
    float3 normalMapSample = normalMap.Sample(samLinear, input.tex).rgb;
    normalMapSample = normalize(normalMapSample * 2.0 - 1.0);

    float3 normal = mul(normalMapSample, input.tbn);
    normal = normalize(normal);

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

        float diff = saturate(dot(normal, lightDir));
        diffuse += Lights[i].Color.rgb * diff * attenuation;
    }

    // Использование массива текстур вместо switch
    float3 texColor = GetTextureColor(input.TexIndex, input.tex);

    float3 result = (ambient + diffuse) * texColor;
    return float4(result, 1.0);
}