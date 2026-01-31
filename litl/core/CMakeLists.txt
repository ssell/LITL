# spdlog
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.17.0)
FetchContent_MakeAvailable(spdlog)

add_library(litl-core 
	STATIC
		 "src/litl-core/logger.cpp")

target_include_directories(litl-core
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-core
	PRIVATE
		spdlog::spdlog
)

target_compile_features(litl-core
	PUBLIC
		cxx_std_20
)