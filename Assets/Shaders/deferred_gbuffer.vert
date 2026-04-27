#include "input_structures.hlsl"

struct VSOut {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 outNormal : NORMAL0;
    [[vk::location(1)]] float2 outUV : TEXCOORD0;
    [[vk::location(2)]] float4 outWorldPos : POSITION0;
};

struct PushConstants {
    row_major float4x4 modelMatrix;
    vk::BufferPointer<Vertex> vertexBuffer;
};

[[vk::push_constant]] PushConstants pc;

VSOut main(uint vID : SV_VertexID) {
    vk::BufferPointer<Vertex> vPtr = PtrAdd(pc.vertexBuffer, vID);
    VSOut output;
    float4 worldPos = mul(float4(vPtr.Get().position, 1.0f), pc.modelMatrix);
    output.position = mul(worldPos, SceneUniformData.viewProj);
    output.outNormal = mul(vPtr.Get().normal, (float3x3)pc.modelMatrix);
    output.outUV = float2(vPtr.Get().uv_x, vPtr.Get().uv_y);
    output.outWorldPos = worldPos;
    return output;
}
