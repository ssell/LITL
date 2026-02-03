FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.12.0)
FetchContent_MakeAvailable(Catch2)

add_executable(litl-tests
	"src/litl-core/containers/concurrentQueue_tests.cpp"
	"src/litl-core/containers/concurrentSingleQueue_tests.cpp" 
	"src/litl-core/logging/logging_tests.cpp")

target_link_libraries(litl-tests
	PRIVATE
		litl-engine
		Catch2::Catch2WithMain
)

target_compile_features(litl-tests
	PUBLIC
		cxx_std_20
)

include (Catch)
catch_discover_tests(litl-tests)