find_program(SLANGC NAMES slangc slangc.exe HINTS "$ENV{VULKAN_SDK}/bin")

function(add_hlsl_shader TARGET_NAME)
    set(oneValueArgs SOURCE ENTRY PROFILE OUTPUT)
    set(multiValueArgs OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    cmake_path(ABSOLUTE_PATH ARG_SOURCE NORMALIZE)

    # Compute the relative path to root.
    cmake_path(GET ARG_SOURCE PARENT_PATH _parent)
    cmake_path(RELATIVE_PATH _parent
            BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE _rel_path)
    # Make the output path the same relative path to root.
    cmake_path(APPEND CMAKE_BINARY_DIR "${_rel_path}"
            OUTPUT_VARIABLE OUT_PATH)

    cmake_path(APPEND OUT_PATH "${ARG_OUTPUT}")

    cmake_path(GET OUT_PATH PARENT_PATH OUT_DIR)
    file(MAKE_DIRECTORY "${OUT_DIR}")

    add_custom_command(
            OUTPUT ${OUT_PATH}
            COMMAND ${SLANGC} "${ARG_SOURCE}" -target spirv -profile ${ARG_PROFILE} -entry ${ARG_ENTRY} ${ARG_OPTIONS} -o "${OUT_PATH}"
            DEPENDS ${ARG_SOURCE}
            COMMENT "Compiling HLSL/Slang: ${ARG_SOURCE}"
            VERBATIM
    )

    target_sources(${TARGET_NAME} PRIVATE ${OUT_PATH})

endfunction()