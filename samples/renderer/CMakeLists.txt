cmake_minimum_required (VERSION 3.10)
project (litl-samples-renderer VERSION 1.0.0 LANGUAGES C CXX)

add_executable(litl-samples-renderer)

target_sources(litl-samples-renderer 
	PRIVATE
		"src/main.cpp")

target_include_directories(litl-samples-renderer 
	PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-samples-renderer PRIVATE litl-engine)
target_compile_definitions(litl-samples-renderer PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# Copy the samples asset directory
add_custom_command(TARGET litl-samples-renderer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/assets"
			$<TARGET_FILE_DIR:litl-samples-renderer>/assets
)