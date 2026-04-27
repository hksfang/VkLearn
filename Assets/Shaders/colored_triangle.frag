#include "input_structures.hlsl"
#include "blinn_phong.hlsl"

[[vk::binding(0, 1)]] Texture2D<float4> diffuseTexture;
[[vk::binding(1, 1)]] Texture2D<float4> specularTexture;
[[vk::binding(2, 1)]] SamplerState linearSampler;
[[vk::binding(3, 1)]] TextureCube cubemap;

struct FSIn {
    [[vk::location(0)]] float3 color : COLOR0;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
    [[vk::location(2)]] float3 normal : NORMAL0;
    [[vk::location(3)]] float4 worldPos : POSITION0;
};

float4 main(FSIn input) : SV_Target
{

    float3 N = normalize(input.normal);
    float3 L = normalize(SceneUniformData.lightPos.xyz - input.worldPos.xyz);
    float3 V = normalize((float3)SceneUniformData.camPos - (float3)input.worldPos);

    float3 diffuseColor =diffuseTexture.Sample(linearSampler, input.uv).rgb;
    // Ambient Lighting.
    float3 result = AmbientLight(diffuseColor);

    // diffuse
    result += Diffuse(N, L, diffuseColor);

    // specular
    float3 specularColor = specularTexture.Sample(linearSampler, input.uv).rgb;
    result += Specular(L, V, N, specularColor);

    return float4(result, 1.0f);
}