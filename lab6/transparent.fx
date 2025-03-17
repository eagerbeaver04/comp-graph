cbuffer ModelBuffer : register(b0)
{
    matrix m;
};

cbuffer VPBuffer : register(b1)
{
    matrix vp;
};

cbuffer ColorBuffer : register(b2)
{
    float4 color;
};

cbuffer LightBuffer : register(b3)
{
    struct Light {
        float4 position;
        float4 color;
        float4 attenuation;
    } Lights[2];
};

cbuffer CameraBuffer : register(b4)
{
    float4 CameraPosition;
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(float4(input.pos, 1.0), m);
    output.worldPos = worldPos.xyz;
    output.pos = mul(worldPos, vp);
    output.normal = mul(input.normal, (float3x3)m);
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    input.normal = normalize(input.normal);

    float3 ambient = float3(0.1, 0.1, 0.1);

    float3 diffuse = float3(0, 0, 0);
    for (int i = 0; i < 2; i++)
    {
        float3 lightDir = Lights[i].position.xyz - input.worldPos;
        float distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = 1.0 / (Lights[i].attenuation.x +
                                  Lights[i].attenuation.y * distance +
                                  Lights[i].attenuation.z * distance * distance);

        float diff = saturate(dot(input.normal, lightDir));
        diffuse += Lights[i].color.rgb * diff * attenuation;
    }

    float3 result = (ambient + diffuse) * color.rgb;
    return float4(result, color.a);
}