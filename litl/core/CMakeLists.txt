FetchContent_Declare(abseil-cpp GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git GIT_TAG 20260107.0)
FetchContent_MakeAvailable(abseil-cpp)

FetchContent_Declare(spirv_reflect GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git GIT_TAG vulkan-sdk-1.4.335.0)
FetchContent_MakeAvailable(spirv_reflect)

add_library(litl-core 
	STATIC
		"src/litl-core/logging/logging.cpp" "src/litl-core/logging/sinks/consoleSink.cpp" "src/litl-core/logging/sinks/loggingSink.cpp" "src/litl-core/logging/sinks/fileSink.cpp" "src/litl-core/hash.cpp" "src/litl-core/containers/flatHashMap.cpp")

target_include_directories(litl-core
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
		"${CMAKE_SOURCE_DIR}/deps/xxHash"
)

target_link_libraries(litl-core 
	PRIVATE 
		absl::flat_hash_map
		spirv-reflect-static
)

target_compile_features(litl-core
	PUBLIC
		cxx_std_20
)