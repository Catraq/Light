cmake_minimum_required(VERSION 3.0.2)
project(light)


find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIR})

find_package(glfw3 3.0 REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/src/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/libs/Linux-Joystick-input/include/")

set(LIGHT_SOURCES 
	"${PROJECT_SOURCE_DIR}/libs/Linux-Joystick-input/src/joystick.c"
	"${PROJECT_SOURCE_DIR}/libs/Linux-Joystick-input/src/joystick_map.c"
	"${PROJECT_SOURCE_DIR}/src/math/mat4x4.c"
	"${PROJECT_SOURCE_DIR}/src/math/vec.c"
	"${PROJECT_SOURCE_DIR}/src/math/quaternion.c"
	"${PROJECT_SOURCE_DIR}/src/camera.c"
	"${PROJECT_SOURCE_DIR}/src/camera_input.c"
	"${PROJECT_SOURCE_DIR}/src/vertex_buffer.c"
	"${PROJECT_SOURCE_DIR}/src/vertex_instance.c"
	"${PROJECT_SOURCE_DIR}/src/vertex_buffer_model.c"
	"${PROJECT_SOURCE_DIR}/src/render_framebuffer.c"
	"${PROJECT_SOURCE_DIR}/src/model/raw_model.c"
	"${PROJECT_SOURCE_DIR}/src/platform.c"

)

add_executable(${PROJECT_NAME} ${LIGHT_SOURCES} src/main.c)
set_property(TARGET light PROPERTY C_STANDARD 11)


#add_subdirectory(tools/objtoraw)
#add_subdirectory(tools/rawviewer)

target_link_libraries(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

