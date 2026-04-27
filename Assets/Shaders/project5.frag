#include "input_structures.hlsl"
#include "blinn_phong.hlsl"

[[vk::binding(0, 1)]] Texture2D<float4> texture;
[[vk::binding(1, 1)]] SamplerState textureSampler;
[[vk::binding(4, 1)]] Texture2D<float4> normalMap;

struct FSIn {
    float4 pos : SV_POSITION;
    [[vk::location(0)]] float4 worldPos : TEXCOORD0;
	[[vk::location(1)]] float2 uv : TEXCOORD2;
};

float4 main(FSIn input) : SV_Target
{
    float3 diffuseColor = float3(0.3, 0.3, 0.3);
    float3 specularColor = float3(1.0, 1.0, 1.0);
    float3 worldPos = input.worldPos.xyz;

    float3 N = normalize(normalMap.Sample(textureSampler, input.uv).rgb);
    float3 L = normalize(SceneUniformData.lightPos.xyz - worldPos.xyz);
    float3 V = normalize((float3)SceneUniformData.camPos - worldPos);

    float3 result = AmbientLight(diffuseColor);
    result += Diffuse(N, L, diffuseColor);
    result += Specular(L, V, N, specularColor);

    return float4(result, 1);
}
