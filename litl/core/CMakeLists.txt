add_library(litl-core 
	STATIC
		 "src/litl-core/logger.cpp")

target_include_directories(litl-core
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-core
	PRIVATE
		quill::quill
)

target_compile_features(litl-core
	PUBLIC
		cxx_std_20
)