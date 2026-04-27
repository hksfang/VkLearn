#include "input_structures.hlsl"

typedef vk::BufferPointer<Vertex> VertexBufferPointer;

struct PushConstants {
    row_major float4x4 modelMatrix;
    VertexBufferPointer vertexBuffer;
};

[[vk::push_constant]]
PushConstants pc;

struct VSOutput {
    float4 Pos : SV_POSITION;
    [[vk::location(0)]] float3 outColor : COLOR0;
    [[vk::location(1)]] float2 outUV : TEXCOORD0;
    [[vk::location(2)]] float3 normal : NORMAL0;
    [[vk::location(3)]] float4 worldPos : POSITION0;
};

VSOutput main(uint VertexIndex : SV_VertexID) {
    VSOutput output;
    vk::BufferPointer<Vertex> currVertexPtr = PtrAdd(pc.vertexBuffer, VertexIndex);
    output.Pos = mul(float4(currVertexPtr.Get().position, 1.0f), SceneUniformData.viewProj);
    // output.outColor = currVertexPtr.Get().color.rgb;
    output.outColor = float3(1.0f, 0.0f, 0.0f);
    output.outUV.x = currVertexPtr.Get().uv_x;
    output.outUV.y = currVertexPtr.Get().uv_y;
    output.normal = mul(currVertexPtr.Get().normal, (float3x3)pc.modelMatrix);
    output.worldPos = mul(float4(currVertexPtr.Get().position, 1.0f), pc.modelMatrix);
    return output;
}