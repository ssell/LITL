add_library(litl-engine STATIC
	"src/engine.cpp"
	"src/windowFactory.cpp" 
	"src/rendererFactory.cpp" 
	"src/cache/pipelineLayoutCache.cpp" "src/frameLimiter.cpp" "src/config.cpp" "src/bootstrap.cpp" "src/scene/scenegraph.cpp" "src/scene/scene.cpp" "src/scene/partition/uniformGridPartition.cpp")

target_include_directories(litl-engine
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-engine PUBLIC litl-core litl-ecs)

if (LITL_ENABLE_VULKAN)
	target_link_libraries(litl-engine PRIVATE litl-renderer-vulkan)
endif()

target_compile_features(litl-engine
	PUBLIC
		cxx_std_20
)

if (MSVC)
	target_compile_options(litl-engine PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-engine PRIVATE -fno-exceptions -fno-rtti)
endif()