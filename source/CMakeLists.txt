set(SRCS
    "main.cpp"
    "QubeGame.h"
    "QubeGame.cpp"
    "QubeWindow.h"
    "QubeWindow.cpp"
    "QubeInput.cpp"
    "QubeControls.cpp"
    "QubeCamera.cpp"
    "QubeRender.cpp"
    "QubeUpdate.cpp"
    "QubeSettings.h"
    "QubeSettings.cpp")

add_subdirectory(Renderer)
add_subdirectory(glew)
add_subdirectory(glm)
add_subdirectory(ini)
add_subdirectory(freetype)
add_subdirectory(Maths)

source_group("source" FILES ${SRCS})
source_group("source\\renderer" FILES ${RENDERER_SRCS})
source_group("source\\glew\\src" FILES ${GLEW_SRCS})
source_group("source\\glew\\include\\GL" FILES ${GLEW_HEADERS})
source_group("source\\glm" FILES ${GLM_SRCS})
source_group("source\\glm\\detail" FILES ${GLM_DETAIL_SRCS})
source_group("source\\glm\\gtc" FILES ${GLM_GTC_SRCS})
source_group("source\\glm\\gtx" FILES ${GLM_GTX_SRCS})
source_group("source\\ini" FILES ${INI_SRCS})
source_group("source\\freetype" FILES ${FREETYPE_SRCS})
source_group("source\\maths" FILES ${MATHS_SRCS})

add_executable(Qube
               ${SRCS}
               ${RENDERER_SRCS}
               ${GLEW_SRCS}
               ${GLEW_HEADERS}
               ${GLM_SRCS}
               ${GLM_DETAIL_SRCS}
               ${GLM_GTC_SRCS}
               ${GLM_GTX_SRCS}
               ${INI_SRCS}
               ${FREETYPE_SRCS}
               ${MATHS_SRCS}
			   )

include_directories(".")			   
include_directories("glfw\\include")
include_directories("glew\\include")
include_directories("freetype\\include")

if(MSVC)
target_link_libraries(Qube "opengl32.lib")
target_link_libraries(Qube "winmm.lib")
elseif(UNIX)
target_link_libraries(Qube "GL")
target_link_libraries(Qube "GLU")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}/glfw/libs/linux/d/libglfw3_64.a")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}/freetype/libs/linux/libfreetype261_64.a")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}/glfw/libs/linux/r/libglfw3_64.a")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}/freetype/libs/linux/libfreetype261_64.a")
	else()
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}/glfw/libs/linux/d/libglfw3.a")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}/freetype/libs/linux/libfreetype261.a")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}/glfw/libs/linux/r/libglfw3.a")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}/freetype/libs/linux/libfreetype261.a")
	endif()
target_link_libraries(Qube "X11")
target_link_libraries(Qube "Xrandr")
target_link_libraries(Qube "Xi")
target_link_libraries(Qube "Xxf86vm")
target_link_libraries(Qube "Xcursor")
target_link_libraries(Qube "Xinerama")
target_link_libraries(Qube "pthread")
target_link_libraries(Qube "dl")
endif()

if(MSVC11)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2012\\d\\glfw3_64.lib")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2012\\freetype261d_64.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2012\\r\\glfw3_64.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2012\\freetype261_64.lib")
	else()
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2012\\d\\glfw3.lib")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2012\\freetype261d.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2012\\r\\glfw3.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2012\\freetype261.lib")
	endif()
endif(MSVC11)
if(MSVC12)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2013\\d\\glfw3_64.lib")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2013\\freetype261d_64.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2013\\r\\glfw3_64.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2013\\freetype261_64.lib")
	else()
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2013\\d\\glfw3.lib")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2013\\freetype261d.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2013\\r\\glfw3.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2013\\freetype261.lib")
	endif()
endif(MSVC12)
if(MSVC14)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2015\\d\\glfw3_64.lib")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2015\\freetype261d_64.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2015\\r\\glfw3_64.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2015\\freetype261_64.lib")
	else()
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2015\\d\\glfw3.lib")
		target_link_libraries(Qube debug "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2015\\freetype261d.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\glfw\\libs\\2015\\r\\glfw3.lib")
		target_link_libraries(Qube optimized "${CMAKE_CURRENT_SOURCE_DIR}\\freetype\\libs\\2015\\freetype261.lib")
	endif()
endif(MSVC14)

if(MSVC)
	set_target_properties(Qube PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../../")
	set_target_properties(Qube PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../")
	set_target_properties(Qube PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../")
	
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif(MSVC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_CONFIGURATION_TYPES Debug Release)
