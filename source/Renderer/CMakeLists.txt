set(RENDERER_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Renderer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/colour.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/colour.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/viewport.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/light.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/material.h"
    PARENT_SCOPE)

source_group("Renderer" FILES ${RENDERER_SRCS})