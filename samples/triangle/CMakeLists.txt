cmake_minimum_required (VERSION 3.10)
project (litl-samples-triangle VERSION 1.0.0 LANGUAGES C CXX)

add_executable(litl-samples-triangle)

target_sources(litl-samples-triangle PRIVATE
	"src/main.cpp" 
	${LITL_SHARED_SOURCE}
	${LITL_VULKAN_SOURCE})

target_include_directories(litl-samples-triangle PUBLIC 
	${Vulkan_INCLUDE_DIRS} 
	"${glfw_SOURCE_DIR}/include"
	"${glm_SOURCE_DIR}/include"
	"${LITL_SHARED_HEADER_DIR}"
    "${LITL_VULKAN_HEADER_DIR}")

target_link_libraries(litl-samples-triangle PRIVATE glfw glm::glm ${Vulkan_LIBRARIES})
target_compile_definitions(litl-samples-triangle PRIVATE $<$<CONFIG:Debug>:DEBUG>)

add_custom_command(TARGET litl-samples-triangle POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${LITL_ASSETS_DIR}
			$<TARGET_FILE_DIR:litl-samples-triangle>/assets
)