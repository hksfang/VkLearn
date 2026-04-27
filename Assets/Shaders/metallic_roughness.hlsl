static const float PI = 3.14159265359;

struct PBRMaterialData {
    float4 colorFactors;
    float4 metalRoughFactors;
    uint32_t baseColorIdx;
    uint32_t baseColorSamplerIdx;
    uint32_t normalIdx;
    uint32_t normalSamplerIdx;
    uint32_t metallicRoughIdx;
    uint32_t metallicRoughSamplerIdx;
    uint32_t radianceIdx;
    uint32_t radianceSamplerIdx;
    uint32_t irradianceIdx;
    uint32_t irradianceSamplerIdx;
    uint32_t emissiveIdx;
    uint32_t emissiveSamplerIdx;
    uint32_t occlusionIdx;
    uint32_t occlusionSamplerIdx;
};

float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3x3 CalculateTBN(float3 worldNormal, float3 worldPos, float2 uv)
{
    float3 dp1 = ddx(worldPos);
    float3 dp2 = ddy(worldPos);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float det = duv1.x * duv2.y - duv2.x * duv1.y;

    float sign_det = det < 0.0 ? -1.0 : 1.0;
    float inv_det = sign_det / max(abs(det), 1e-8);

    float3 T = (dp1 * duv2.y - dp2 * duv1.y) * inv_det;
    float3 B = (dp2 * duv1.x - dp1 * duv2.x) * inv_det;

    float3 N = normalize(worldNormal);
    T = normalize(T - dot(T, N) * N);

    float3 orthoB = cross(N, T) * sign(dot(cross(N, T), B));

    return float3x3(T, orthoB, N);
}

float3 EnvBRDFApprox(float3 f0, float roughness, float NdotV) {
    // unreal approx
    float4 c0 = float4(-1.0, -0.0275, -0.572, 0.022);
    float4 c1 = float4(1.0, 0.0425, 1.04, -0.04);
    float4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NdotV)) * r.x + r.y;
    float2 AB = float2(-1.04, 1.04) * a004 + r.zw;

    return f0 * AB.x + AB.y;
}

float3 DirectLighting(float3 N, float3 H, float3 L, float3 V, float3 F0, float3 albedo, float roughness, float metalness, float3 lightColor) {
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    float3 F_direct  = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 kS_direct = F_direct;
    float3 kD_direct = float3(1.0, 1.0, 1.0) - kS_direct;
    kD_direct *= 1.0 - metalness;

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float3 numerator    = NDF * G * F_direct;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    float3 specular_direct = numerator / denominator;

    return (kD_direct * albedo / PI + specular_direct) * lightColor * NdotL;
}

float3 IBL(float3 F0, float3 irradiance, float3 albedo, float3 prefilteredColor, float roughness, float metalness, float3 N, float3 V) {
    float NdotV = max(dot(N, V), 0.0);
    float3 F_IBL = F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - NdotV, 5.0);

    float3 kS_IBL = F_IBL;
    float3 kD_IBL = float3(1.0, 1.0, 1.0) - kS_IBL;
    kD_IBL *= 1.0 - metalness;

    float3 diffuse_IBL = irradiance * albedo;

    float3 envBRDF = EnvBRDFApprox(F0, roughness, NdotV);

    float3 specular_IBL = prefilteredColor * envBRDF;

    return kD_IBL * diffuse_IBL + specular_IBL;
}
