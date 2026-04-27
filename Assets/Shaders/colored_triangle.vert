struct VSOutput {
    float4 pos : SV_Position;
    [[vk::location(0)]] float3 color : COLOR0;
};

VSOutput main(uint VertexId : SV_VertexID) {
    VSOutput output;

    static const float3 positions[3] = {
        float3(1.0f, 1.0f, 0.0f),
        float3(-1.0f, 1.0f, 0.0f),
        float3(0.0f, -1.0f, 0.0f)
    };

    static const float3 colors[3] = {
        float3(1.0f, 0.0f, 0.0f),
        float3(0.0f, 1.0f, 0.0f),
        float3(0.0f, 0.0f, 1.0f)
    };

    output.pos = float4(positions[VertexId], 1.0f);
    output.color = colors[VertexId];

    return output;
}