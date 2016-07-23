set(NANOVG_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/fontstash.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/nanovg.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/nanovg.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/nanovg_gl.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/nanovg_gl_utils.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/stb_truetype.h"
    PARENT_SCOPE)

source_group("nanovg" FILES ${NANOVG_SRCS})