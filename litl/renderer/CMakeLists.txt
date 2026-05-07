add_library(litl-renderer STATIC 
	"src/litl-renderer/rendererConfiguration.cpp"
    "src/litl-renderer/pipeline/shaderModule.cpp"
    "src/litl-renderer/pipeline/shaderReflection.cpp"
)

target_include_directories(litl-renderer 
	PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
		"${CMAKE_SOURCE_DIR}/deps/SPIRV-Reflect"
)

target_link_libraries(litl-renderer PUBLIC litl-core)
target_compile_features(litl-renderer PUBLIC cxx_std_20)

if (MSVC)
	target_compile_options(litl-renderer PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-renderer PRIVATE -fno-exceptions -fno-rtti)
endif()