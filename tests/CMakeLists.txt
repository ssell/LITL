# ------------------------------------------------------------------------------------------
# Catch2
# ------------------------------------------------------------------------------------------

FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.12.0)
FetchContent_MakeAvailable(Catch2)

include (Catch)

# ------------------------------------------------------------------------------------------
# litl-tests
# ------------------------------------------------------------------------------------------

add_executable(litl-tests
	"src/litl-core/containers/concurrentQueue_tests.cpp"
	"src/litl-core/containers/concurrentSingleQueue_tests.cpp" 
	"src/litl-core/logging/logging_tests.cpp" 
	"src/litl-core/hash_tests.cpp" 
	"src/litl-core/containers/flatHashMap_tests.cpp" 
	"src/litl-core/refptr_tests.cpp" 
	"src/litl-core/renderer/shaderReflection_tests.cpp" 
	"src/litl-core/containers/pagedVector_tests.cpp" 
	"src/litl-core/math/general_tests.cpp" 
	"src/litl-engine/ecs/component_tests.cpp" 
	"src/litl-engine/ecs/chunk_tests.cpp" 
	"src/litl-engine/ecs/archetype_tests.cpp" 
	"src/litl-engine/ecs/entity_tests.cpp" 
	"src/litl-engine/ecs/world_tests.cpp" "src/litl-engine/ecs/system_tests.cpp")

target_link_libraries(litl-tests
	PRIVATE
		litl-engine
		Catch2::Catch2WithMain
)

target_compile_features(litl-tests
	PUBLIC
		cxx_std_20
)

# ------------------------------------------------------------------------------------------
# data copy
# ------------------------------------------------------------------------------------------

add_custom_target(copy-tests-data
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_SOURCE_DIR}/data/"
	"${CMAKE_CURRENT_BINARY_DIR}/data"
	COMMENT "Copying test data ...")

add_dependencies(litl-tests copy-tests-data)

# ------------------------------------------------------------------------------------------
# test discovery
# ------------------------------------------------------------------------------------------

catch_discover_tests(litl-tests)