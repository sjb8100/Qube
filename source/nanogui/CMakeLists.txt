file(GLOB NANOGUI_SRCS include/nanogui/*.h)
set(NANOGUI_SRCS ${NANOGUI_SRCS} PARENT_SCOPE)

file(GLOB NANOGUI_SERIALIZER_SRCS include/nanogui/serializer/*.h)
set(NANOGUI_SERIALIZER_SRCS ${NANOGUI_SERIALIZER_SRCS} PARENT_SCOPE)

source_group("nanogui\\include\\nanogui" FILES ${NANOGUI_SRCS})
source_group("nanogui\\include\\nanogui\\serializer" FILES ${NANOGUI_SERIALIZER_SRCS})