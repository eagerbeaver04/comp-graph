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

VS_OUTPUT VS(VS_INPUT input, uint instanceID : SV_InstanceID)
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

Texture2D diffuseMap : register(t0);
Texture2D normalMap  : register(t1);
SamplerState samLinear : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 light0Pos;
    float4 light0Color;
    float4 light1Pos;
    float4 light1Color;
};

float3 ComputeTangent(float3 n)
{
    return (abs(n.y) > 0.99) ? float3(1, 0, 0) : normalize(cross(float3(0, 1, 0), n));
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 diffuseColor = diffuseMap.Sample(samLinear, input.TexCoord);

    float3 normalSample = normalMap.Sample(samLinear, input.TexCoord).rgb * 2.0 - 1.0;

    float3 tangent = ComputeTangent(normalize(input.Normal));
    float3 bitangent = normalize(cross(normalize(input.Normal), tangent));
    float3x3 TBN = float3x3(tangent, bitangent, normalize(input.Normal));

    float3 perturbedNormal = normalize(mul(normalSample, TBN));

    float3 ambient = float3(0.1, 0.1, 0.1);

    float3 totalDiffuse = float3(0, 0, 0);

    float3 lightVector0 = light0Pos.xyz - input.WorldPos;
    float distance0 = length(lightVector0);
    float3 lightDir0 = normalize(lightVector0);
    float attenuation0 = 1.0 / (1.0 + 0.09 * distance0 + 0.032 * distance0 * distance0);
    float diff0 = saturate(dot(perturbedNormal, lightDir0));
    totalDiffuse += light0Color.rgb * diff0 * attenuation0;

    float3 lightVector1 = light1Pos.xyz - input.WorldPos;
    float distance1 = length(lightVector1);
    float3 lightDir1 = normalize(lightVector1);
    float attenuation1 = 1.0 / (1.0 + 0.09 * distance1 + 0.032 * distance1 * distance1);
    float diff1 = saturate(dot(perturbedNormal, lightDir1));
    totalDiffuse += light1Color.rgb * diff1 * attenuation1;

    float3 finalColor = (ambient + totalDiffuse) * diffuseColor.rgb;

    return float4(finalColor, diffuseColor.a);
}