#include "input_structures.hlsl"

struct VSOutput {
    float4 Pos : SV_POSITION;
    [[vk::location(0)]] float4 worldPos : TEXCOORD0;
    [[vk::location(2)]] float2 UV : TEXCOORD2;
};

VSOutput main(uint VertexID : SV_VertexID) {
    VSOutput output;
    static const float2 positions[4] = {
            float2(-0.5, -0.5),
            float2( 0.5, -0.5),
            float2(-0.5,  0.5),
            float2( 0.5,  0.5),
    };
    float4 pos = float4(positions[VertexID] * 160 , 0.0, 1.0);
    output.Pos = mul(pos, SceneUniformData.viewProj);
    output.worldPos = pos;
    output.UV = positions[VertexID] + float2(0.5, 0.5);
    // output.Pos = pos;
    return output;
}