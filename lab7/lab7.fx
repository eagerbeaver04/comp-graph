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
    float4 diffuseColor = diffuseMap.Sample(samLinear, input.TexCoord);

    // Извлечение и преобразование нормалей из normal map
    float3 normalSample = normalMap.Sample(samLinear, input.TexCoord).rgb * 2.0 - 1.0;

    // Построение TBN матрицы
    float3 tangent = ComputeTangent(normalize(input.Normal));
    float3 bitangent = normalize(cross(normalize(input.Normal), tangent));
    float3x3 TBN = float3x3(tangent, bitangent, normalize(input.Normal));

    // Преобразование нормали в мировое пространство
    float3 perturbedNormal = normalize(mul(normalSample, TBN));

    // Ambient освещение
    float3 ambient = float3(0.1, 0.1, 0.1);

    // Расчёт освещения для каждого источника
    float3 totalDiffuse = float3(0, 0, 0);

    // Первый источник света
    float3 lightVector0 = light0Pos - input.WorldPos;
    float distance0 = length(lightVector0);
    float3 lightDir0 = normalize(lightVector0);
    float attenuation0 = 1.0 / (1.0 + 0.09 * distance0 + 0.032 * distance0 * distance0);
    float diff0 = saturate(dot(perturbedNormal, lightDir0));
    totalDiffuse += light0Color * diff0 * attenuation0;

    // Второй источник света
    float3 lightVector1 = light1Pos - input.WorldPos;
    float distance1 = length(lightVector1);
    float3 lightDir1 = normalize(lightVector1);
    float attenuation1 = 1.0 / (1.0 + 0.09 * distance1 + 0.032 * distance1 * distance1);
    float diff1 = saturate(dot(perturbedNormal, lightDir1));
    totalDiffuse += light1Color * diff1 * attenuation1;

    // Комбинирование компонентов освещения
    float3 finalColor = (ambient + totalDiffuse) * diffuseColor.rgb;

    return float4(finalColor, diffuseColor.a);
}