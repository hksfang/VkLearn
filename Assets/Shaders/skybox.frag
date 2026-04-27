#include "input_structures.hlsl"

struct SkyboxData {
    uint32_t cubemapIdx;
    uint32_t cubemapSamplerIdx;
};

struct FSInput {
    float3 uvw : TEXCOORD0;
};

float4 main(FSInput input) : SV_Target0 {
    vk::BufferPointer<SkyboxData> s = vk::BufferPointer<SkyboxData>(pc.materialDataBuffer);

  	float4 sampled = GlobalCubeMaps[s.Get().cubemapIdx].Sample(GlobalSamplers[s.Get().cubemapSamplerIdx], input.uvw);
  	// Tone Mapping.
  	return sampled / (sampled + 1.0);
}