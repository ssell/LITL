add_library(litl-core 
	STATIC
		"src/litl-core/logging/logging.cpp" "src/litl-core/logging/sinks/consoleSink.cpp" "src/litl-core/logging/sinks/loggingSink.cpp" "src/litl-core/logging/sinks/fileSink.cpp" "src/litl-core/hash.cpp")

target_include_directories(litl-core
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
		"${CMAKE_SOURCE_DIR}/deps/xxHash"
)

target_compile_features(litl-core
	PUBLIC
		cxx_std_20
)