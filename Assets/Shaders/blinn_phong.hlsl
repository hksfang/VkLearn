
float3 AmbientLight(float3 diffuseColor) {
    return (float3)SceneUniformData.ambientColor * diffuseColor;
}

float3 Diffuse(float3 N, float3 L, float3 diffuseColor) {
    float diff = saturate(dot(N, L));
    return diff * (float3)SceneUniformData.sunlightColor * diffuseColor;
}

float3 Specular(float3 L, float3 V, float3 N, float3 specularColor) {
    float3 H = normalize(L + V);
    float specular = pow(saturate(dot(N, H)), 32.0f);
    return specular * specularColor;
}
