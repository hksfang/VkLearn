struct FSIn {
    [[vk::location(0)]] float3 normal : NORMAL0;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
    [[vk::location(2)]] float4 worldPos : POSITION0;
};

struct FSOut {
    [[vk::location(0)]] float4 gPosition : SV_Target0;
    [[vk::location(1)]] float4 gNormal : SV_Target1;
    [[vk::location(2)]] float2 gUV : SV_Target2;
};

FSOut main(FSIn input) {
    FSOut output;
    output.gPosition = input.worldPos;
    output.gNormal = float4(normalize(input.normal), 1.0);
    output.gUV = input.uv;
    return output;
}
