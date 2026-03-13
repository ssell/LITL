add_library(litl-engine STATIC
	"src/engine.cpp"
	"src/windowFactory.cpp" 
	"src/rendererFactory.cpp" 
	"src/cache/pipelineLayoutCache.cpp" "src/frameLimiter.cpp" "src/config.cpp")

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

target_compile_options(litl-engine PRIVATE 
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fno-rtti>
    $<$<CXX_COMPILER_ID:MSVC>:/GR->
)