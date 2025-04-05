cbuffer CullingParams : register(b0)
{
    uint ShapeCount;
    uint3 _Padding;
    float4 BoundingBoxMin[20];
    float4 BoundingBoxMax[20];
};

cbuffer FrustumData : register(b1)
{
    float4x4 ViewProjection;
    float4 FrustumPlanes[6];
};

RWStructuredBuffer<uint> DrawIndirectArgs : register(u0);
RWStructuredBuffer<uint> VisibleObjects : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
    const uint objectIndex = ThreadID.x;

    if (objectIndex >= ShapeCount)
        return;

    const float3 minBounds = BoundingBoxMin[objectIndex].xyz;
    const float3 maxBounds = BoundingBoxMax[objectIndex].xyz;

    const float3 boxCorners[8] = {
        float3(minBounds.x, minBounds.y, minBounds.z),
        float3(maxBounds.x, minBounds.y, minBounds.z),
        float3(minBounds.x, maxBounds.y, minBounds.z),
        float3(minBounds.x, minBounds.y, maxBounds.z),
        float3(maxBounds.x, maxBounds.y, minBounds.z),
        float3(maxBounds.x, minBounds.y, maxBounds.z),
        float3(minBounds.x, maxBounds.y, maxBounds.z),
        float3(maxBounds.x, maxBounds.y, maxBounds.z)
    };

    bool visible = true;

    [unroll]
    for (int planeIdx = 0; planeIdx < 6; ++planeIdx)
    {
        int outsidePoints = 0;

        [unroll]
        for (int cornerIdx = 0; cornerIdx < 8; ++cornerIdx)
            if (dot(FrustumPlanes[planeIdx].xyz, boxCorners[cornerIdx]) + FrustumPlanes[planeIdx].w < 0)
                ++outsidePoints;
        if (outsidePoints == 8)
        {
            visible = false;
            break;
        }
    }
    if (visible)
    {
        uint outputPos;
        InterlockedAdd(DrawIndirectArgs[1], 1, outputPos);
        VisibleObjects[outputPos] = objectIndex;
    }
}