set(TOOL OBJTORAW)

set(SOURCES 
	"${PROJECT_SOURCE_DIR}/src/model/raw_model.cpp"
	"src/objtoraw.cpp"
)
add_executable(${TOOL} ${SOURCES}) 


target_link_libraries(${TOOL} ${GLEW_LIBRARIES})
target_link_libraries(${TOOL} glfw)
target_link_libraries(${TOOL} ${OPENGL_LIBRARIES})
