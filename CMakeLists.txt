cmake_minimum_required(VERSION 3.0.2)
project(light)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIR})

find_package(glfw3 3.3 REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/src/")


set(SOURCES 
	"src/math/mat4x4.cpp"
	"src/math/quaternion.cpp"
	"src/camera.cpp"
	"src/camera_input.cpp"
	"src/physic.cpp"
	"src/renderer.cpp"
	"src/vertex_buffer.cpp"
	"src/model/raw_model.cpp"
	"src/platform.cpp"
	"src/scene.cpp"
	"src/main.cpp"	
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(tools/objtoraw)
add_subdirectory(tools/rawviewer)

target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

