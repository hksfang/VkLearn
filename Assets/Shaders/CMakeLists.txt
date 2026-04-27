add_custom_target(ShaderTarget)
add_dependencies(Assets ShaderTarget)

add_hlsl_shader(ShaderTarget
        SOURCE "gltf_pbr.vert"
        ENTRY "main"
        PROFILE "vs_6_6"
        OUTPUT "gltf_pbr.vert.spv"
)

add_hlsl_shader(ShaderTarget
        SOURCE "gltf_pbr.frag"
        ENTRY "main"
        PROFILE "ps_6_6"
        OUTPUT "gltf_pbr.frag.spv"
)

add_hlsl_shader(ShaderTarget
        SOURCE "skybox.vert"
        ENTRY "main"
        PROFILE "vs_6_6"
        OUTPUT "skybox.vert.spv"
)

add_hlsl_shader(ShaderTarget
        SOURCE "skybox.frag"
        ENTRY "main"
        PROFILE "ps_6_6"
        OUTPUT "skybox.frag.spv"
)


# add_hlsl_shader(ShaderTarget
#         SOURCE "deferred_gbuffer.vert"
#         ENTRY "main"
#         PROFILE "vs_6_6"
#         OUTPUT "deferred_gbuffer.vert.spv"
# )
#
# add_hlsl_shader(ShaderTarget
#         SOURCE "deferred_gbuffer.frag"
#         ENTRY "main"
#         PROFILE "ps_6_6"
#         OUTPUT "deferred_gbuffer.frag.spv"
# )
