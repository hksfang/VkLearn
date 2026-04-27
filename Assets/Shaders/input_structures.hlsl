struct SceneData {
    row_major float4x4 view;
    row_major float4x4 proj;
    row_major float4x4 viewProj;
    float4 ambientColor;
    float4 sunlightDirection; // w for sun power
    float4 sunlightColor;
    float4 camPos;
};

// Set 0: Bindless resources
[[vk::binding(0, 0)]] Texture2D GlobalTextures[];
[[vk::binding(1, 0)]] SamplerState GlobalSamplers[];
[[vk::binding(2, 0)]] SamplerComparisonState GlobalCompareSamplers[];
[[vk::binding(3, 0)]] TextureCube GlobalCubeMaps[];

struct Vertex {
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 color;
    float4 tangent;
};

vk::BufferPointer<Vertex> PtrAdd(vk::BufferPointer<Vertex> ptr, uint index) {
    return vk::BufferPointer<Vertex>((uint64_t)ptr + index * sizeof(Vertex));
}


struct InvData {
    row_major float4x4 invModelMatrix;
    row_major float4x4 invViewMatrix;
    row_major float4x4 invProjMatrix;
};

struct PushConstants {
    row_major float4x4 modelMatrix;
    vk::BufferPointer<InvData> invData;
    vk::BufferPointer<Vertex> vertexBuffer;
    uint64_t materialDataBuffer;
    vk::BufferPointer<SceneData> sceneDataBuffer;
};

[[vk::push_constant]]
PushConstants pc;
