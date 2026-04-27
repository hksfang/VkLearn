#include "input_structures.hlsl"
#include "metallic_roughness.hlsl"

struct VSOut {
    float4 Pos : SV_POSITION;
    [[vk::location(0)]] float3 outColor : COLOR0;
    [[vk::location(1)]] float2 outUV : TEXCOORD0;
    [[vk::location(2)]] float3 normal : NORMAL0;
    [[vk::location(3)]] float4 worldPos : POSITION0;
    [[vk::location(4)]] float4 worldTangent : TANGENT;
};

VSOut main(uint VertexIndex : SV_VertexID) {
    vk::BufferPointer<PBRMaterialData> m = vk::BufferPointer<PBRMaterialData>(pc.materialDataBuffer);
    PBRMaterialData materialData = m.Get();
    SceneData sceneData = pc.sceneDataBuffer.Get();

    VSOut output;
    Vertex currVertexPtr = PtrAdd(pc.vertexBuffer, VertexIndex).Get();
    output.worldPos = mul(float4(currVertexPtr.position, 1.0f), pc.modelMatrix);
    output.outColor = currVertexPtr.color.rgb; //* MaterialData.colorFactors.rgb;
    output.outUV.x = currVertexPtr.uv_x;
    output.outUV.y = currVertexPtr.uv_y;
    output.normal = mul(currVertexPtr.normal, (float3x3)pc.modelMatrix);
    output.worldTangent = float4(mul(currVertexPtr.tangent.xyz, (float3x3)pc.modelMatrix), currVertexPtr.tangent.w);
    output.Pos = mul(output.worldPos, sceneData.viewProj);
    return output;
}