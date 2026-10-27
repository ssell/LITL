cmake_minimum_required (VERSION 3.10)
project (litl-samples-sponza VERSION 1.0.0 LANGUAGES C CXX)

add_executable(litl-samples-sponza)

target_sources(litl-samples-sponza
	PRIVATE 
		"src/main.cpp" "src/freeFlyController.cpp")

target_include_directories(litl-samples-sponza
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-samples-sponza PRIVATE litl-engine)
target_compile_definitions(litl-samples-sponza PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# ------------------------------------------------------------------------------------------
# Copy the sample local assets directory
# ------------------------------------------------------------------------------------------

set (LITL_SAMPLES_SPONZA_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

add_custom_command(TARGET litl-samples-sponza POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_SOURCE_DIR}/assets"
	$<TARGET_FILE_DIR:litl-samples-sponza>/assets
)

# ------------------------------------------------------------------------------------------
# Copy engine assets
# ------------------------------------------------------------------------------------------

add_custom_command(TARGET litl-samples-sponza POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${LITL_ASSETS_DIR}
	$<TARGET_FILE_DIR:litl-samples-sponza>/assets
)