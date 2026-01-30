add_library(litl-core STATIC
	"src/deleteme.cpp"
)

target_include_directories(litl-core
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(litl-core
	PUBLIC
		cxx_std_20
)