set(TEMPLATE light)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIR})

find_package(glfw3 3.0 REQUIRED)

find_package( BLAS REQUIRED )
include_directories(${BLAS_INCLUDE_DIR})

find_package( LAPACK REQUIRED )
include_directories(${LAPACK_INCLUDE_DIR})



set(CMAKE_C_FLAGS "-Wall")

add_executable(${TEMPLATE} main.c ${LIGHT_SOURCES})
 
set_property(TARGET ${TEMPLATE} PROPERTY C_STANDARD 11)

target_link_libraries(${TEMPLATE} m)
target_link_libraries(${TEMPLATE} pthread)
target_link_libraries(${TEMPLATE} ${GLEW_LIBRARIES})
target_link_libraries(${TEMPLATE} glfw)
target_link_libraries(${TEMPLATE} ${OPENGL_LIBRARIES})
target_link_libraries(${TEMPLATE} ${BLAS_LIBRARIES})
target_link_libraries(${TEMPLATE} ${LAPACK_LIBRARIES})
target_link_libraries(${TEMPLATE} lapacke)

