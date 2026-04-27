#include "input_structures.hlsl"

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 uvw: TEXCOORD0;
};

VSOutput main(uint VertexID : SV_VertexID) {
    VSOutput output;
    float2 texCoord = float2((VertexID << 1) & 2, VertexID & 2);
    float2 ndcPos = texCoord * 2.0f - 1.0f;
    float4 target = mul(float4(ndcPos.x, ndcPos.y, 1.0f, 1.0f), pc.invData.Get().invProjMatrix);
    float3 rayDir = mul(float4(target.xyz / target.w, 0.0f), pc.invData.Get().invViewMatrix).xyz;

    output.uvw = rayDir;
    output.pos = float4(ndcPos, 0.0f, 1.0f);
    return output;
}