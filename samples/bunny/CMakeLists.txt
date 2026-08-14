cmake_minimum_required (VERSION 3.10)
project (litl-samples-bunny VERSION 1.0.0 LANGUAGES C CXX)

add_executable(litl-samples-bunny)

target_sources(litl-samples-bunny
	PRIVATE 
		"src/main.cpp")

target_include_directories(litl-samples-bunny
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-samples-bunny PRIVATE litl-engine)
target_compile_definitions(litl-samples-bunny PRIVATE $<$<CONFIG:Debug>:DEBUG>)

add_custom_command(TARGET litl-samples-bunny POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${LITL_ASSETS_DIR}
	$<TARGET_FILE_DIR:litl-samples-bunny>/assets
)