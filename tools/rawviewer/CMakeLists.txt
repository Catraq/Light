


set(TOOL RAWVIEWER)

add_executable(${TOOL} rawviewer.cpp) 
target_link_libraries(${TOOL} ${GLEW_LIBRARIES})
target_link_libraries(${TOOL} glfw)
target_link_libraries(${TOOL} ${OPENGL_LIBRARIES})
