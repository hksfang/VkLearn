add_custom_target(
        Assets
)

add_subdirectory(Shaders)

add_custom_command(
        TARGET Assets POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/Assets/Meshes
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Meshes ${CMAKE_BINARY_DIR}/Assets/Meshes
        COMMENT "Copying meshes"
)

add_custom_command(
        TARGET Assets POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/Assets/Skybox
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Skybox ${CMAKE_BINARY_DIR}/Assets/Skybox
        COMMENT "Copying skyboxes"
)

add_custom_command(
        TARGET Assets POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/Assets/Textures
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Textures ${CMAKE_BINARY_DIR}/Assets/Textures
        COMMENT "Copying textures"
)