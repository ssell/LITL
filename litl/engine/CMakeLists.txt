add_library(litl-engine STATIC
	"src/engine.cpp"
)

target_include_directories(litl-engine
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-engine PUBLIC litl-core)

if (LITL_ENABLE_VULKAN)
	target_link_libraries(litl-engine PRIVATE litl-renderer-vulkan)
endif()

target_compile_features(litl-engine
	PUBLIC
		cxx_std_20
)