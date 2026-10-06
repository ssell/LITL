cmake_minimum_required (VERSION 3.10)
project (litl-samples-sponza VERSION 1.0.0 LANGUAGES C CXX)

add_executable(litl-samples-sponza)

target_sources(litl-samples-sponza
	PRIVATE 
		"src/main.cpp")

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
# Copy selected engine assets
# ------------------------------------------------------------------------------------------

set (LITL_SAMPLES_SPONZA_ASSETS
	"${LITL_ASSETS_DIR}/shaders/litl/core.slang"
	"${LITL_ASSETS_DIR}/shaders/unlit.slang"
	"${LITL_ASSETS_DIR}/shaders/lit.slang"
	"${LITL_ASSETS_DIR}/materials/unlit.litlmat"
	"${LITL_ASSETS_DIR}/materials/lit.litlmat"
	"${LITL_ASSETS_DIR}/models/sphere.glb"
)

foreach(LITL_SAMPLES_SPONZA_ASSET_PATH IN LISTS LITL_SAMPLES_SPONZA_ASSETS)
	string(REPLACE ${LITL_ASSETS_DIR} "" RELATIVE_ASSET "${LITL_SAMPLES_SPONZA_ASSET_PATH}")

	add_custom_command(
		TARGET litl-samples-sponza POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${LITL_SAMPLES_SPONZA_ASSET_PATH}
			"$<TARGET_FILE_DIR:litl-samples-sponza>/assets/${RELATIVE_ASSET}"
		COMMENT "Updating asset: ${RELATIVE_ASSET}"
		VERBATIM
	)
endforeach()