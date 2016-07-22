set(RENDERER_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Renderer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/colour.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/colour.cpp"
    PARENT_SCOPE)

source_group("Renderer" FILES ${RENDERER_SRCS})