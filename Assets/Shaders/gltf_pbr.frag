#include "input_structures.hlsl"
#include "metallic_roughness.hlsl"

struct FSIn {
    [[vk::location(0)]] float3 color : COLOR0;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
    [[vk::location(2)]] float3 normal : NORMAL0;
    [[vk::location(3)]] float4 worldPos : POSITION0;
    [[vk::location(4)]] float4 worldTangent : TANGENT;
};


float4 main(FSIn input) : SV_Target {
    PBRMaterialData m = vk::BufferPointer<PBRMaterialData>(pc.materialDataBuffer).Get();
    SceneData sceneData = pc.sceneDataBuffer.Get();
    float4 baseColor = GlobalTextures[m.baseColorIdx].Sample(GlobalSamplers[m.baseColorSamplerIdx], input.uv) * m.colorFactors;
    float3 normal = GlobalTextures[m.normalIdx].Sample(GlobalSamplers[m.normalSamplerIdx], input.uv).xyz;
    float4 metalRoughness = GlobalTextures[m.metallicRoughIdx].Sample(GlobalSamplers[m.metallicRoughSamplerIdx], input.uv);
    float ao = GlobalTextures[m.occlusionIdx].Sample(GlobalSamplers[m.occlusionSamplerIdx], input.uv).r;
    float3 emissive = GlobalTextures[m.emissiveIdx].Sample(GlobalSamplers[m.emissiveSamplerIdx], input.uv).rgb;
        float3 worldNormal = normalize(input.normal);
        float3 worldTangent = normalize(input.worldTangent.xyz);
        worldTangent = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal);
        float3 B = cross(worldNormal, worldTangent);
        float3x3 TBN = float3x3(worldTangent, B, worldNormal);
        float3 N = mul(normal * 2.0 - 1.0, TBN);
        N = normalize(N);
    float3 irradiance = GlobalCubeMaps[m.irradianceIdx].Sample(GlobalSamplers[m.irradianceSamplerIdx], N).rgb;
    float3 V = normalize(sceneData.camPos - input.worldPos).xyz;
    float3 R = reflect(-V, N);
    float roughness = metalRoughness.g;
    const float MAX_REFLECTION_LOD = 8.0;
    float3 prefilteredColor = GlobalCubeMaps[m.radianceIdx].SampleLevel(GlobalSamplers[m.irradianceSamplerIdx], R, roughness * MAX_REFLECTION_LOD).rgb;

    float3 albedo = baseColor.rgb;
    float alpha = baseColor.a;
    if (alpha < 0.1f) {
        discard;
    }

    float metalness = metalRoughness.b;

    float NdotV = max(dot(N, V), 0.0);
    float3 L = normalize(-sceneData.sunlightDirection.xyz);
    float3 sunlightColor = sceneData.sunlightColor.xyz;

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metalness);

    float3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);

    float3 Lo_direct = DirectLighting(N, H, L, V, F0, albedo, roughness, metalness, sunlightColor * sceneData.sunlightDirection.w);

    float3 color = Lo_direct + IBL(F0, irradiance, albedo, prefilteredColor, roughness, metalness, N, V) * ao + emissive;

    color = color / (color + 1.0);

    return float4(color, alpha);
}