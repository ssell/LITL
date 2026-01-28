cmake_minimum_required (VERSION 3.8)
project(triangle VERSION 1.0.0 LANGUAGES CXX)

add_executable(triangle)

target_sources(triangle PRIVATE
	 "src/main.cpp" 
	 "src/triangleProgram.cpp"
	 ${LITL_SHARED_SOURCE})

target_include_directories(triangle PUBLIC 
	${Vulkan_INCLUDE_DIRS} 
	"${PROJECT_SOURCE_DIR}/include")

target_link_libraries(triangle PRIVATE GLFW ${Vulkan_LIBRARIES})
target_compile_definitions(triangle PRIVATE $<$<CONFIG:Debug>:DEBUG>)

add_custom_command(TARGET triangle POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${LITL_ASSETS}
			$<TARGET_FILE_DIR:triangle>/assets
)