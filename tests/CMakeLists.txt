# ------------------------------------------------------------------------------------------
# Catch2
# ------------------------------------------------------------------------------------------

FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.12.0)
FetchContent_MakeAvailable(Catch2)

include (Catch)

# ------------------------------------------------------------------------------------------
# litl-tests
# ------------------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(litl-tests
	"src/litl-core/containers/concurrentQueue_tests.cpp"
	"src/litl-core/containers/concurrentSingleQueue_tests.cpp" 
	"src/litl-core/logging/logging_tests.cpp" 
	"src/litl-core/hash_tests.cpp" 
	"src/litl-core/containers/flatHashMap_tests.cpp" 
	"src/litl-core/refptr_tests.cpp" 
	"src/litl-core/renderer/shaderReflection_tests.cpp" 
	"src/litl-core/containers/pagedVector_tests.cpp" 
	"src/litl-core/math/general_math_tests.cpp" 
	"src/litl-ecs/component_tests.cpp" 
	"src/litl-ecs/chunk_tests.cpp" 
	"src/litl-ecs/archetype_tests.cpp" 
	"src/litl-ecs/entity_tests.cpp" 
	"src/litl-ecs/world_tests.cpp" 
	"src/litl-ecs/system_tests.cpp" 
	"src/litl-engine/frame_tests.cpp" 
	"src/litl-core/math/random_tests.cpp" 
	"src/litl-core/threadinfo_tests.cpp" 
	"src/litl-core/job/jobDeque_tests.cpp" 
	"src/litl-core/job/jobPool_tests.cpp" 
	"src/litl-core/job/jobScheduler_tests.cpp" 
	"src/litl-core/job/job_tests.cpp" 
	"src/litl-core/services/serviceCollection_tests.cpp" 
	"src/litl-core/types_tests.cpp"   
	"src/litl-core/math/dag_tests.cpp" 
	"src/litl-ecs/systemGraph_tests.cpp" 
	"src/litl-ecs/systemInfoGraph_tests.cpp" 
	"src/litl-ecs/entityCommands_tests.cpp" 
	"src/litl-core/containers/fixedSortedArray_tests.cpp" 
	"src/litl-ecs/archetypeComponents_tests.cpp" 
	"src/litl-core/math/vec3_tests.cpp" 
	"src/litl-core/containers/memoryArena_tests.cpp" 
	"src/litl-core/containers/ringBuffer_tests.cpp" 
	"src/litl-core/math/mat4_tests.cpp" 
	"src/litl-core/math/vec4_tests.cpp" 
	"src/litl-core/math/mat3_tests.cpp" 
	"src/litl-core/math/quat_tests.cpp" 
	"src/litl-core/math/types_tests.cpp" 
	"src/litl-engine/scene/scenegraph_tests.cpp" 
	"src/litl-core/assert_tests.cpp" 
	"src/litl-engine/scene/scene_tests.cpp" 
	"src/litl-core/math/bounds_tests.cpp"  
	"src/litl-engine/scene/uniformGridPartition_tests.cpp" 
	"src/litl-core/handles_tests.cpp" 
	"src/litl-core/containers/alignedByteBuffer_tests.cpp" 
	"src/litl-core/containers/flatHashSet_tests.cpp" 
	"src/litl-engine/scene/sceneChangeProcessor_tests.cpp" 
	"src/litl-core/task_tests.cpp" 
	"src/litl-core/moveOnlyFunc_tests.cpp" 
	"src/litl-engine/asset_tests.cpp" 
	"src/litl-import/importObj_tests.cpp" 
	"src/litl-core/formats/litlmesh_tests.cpp" 
	"src/litl-core/math/normals_tests.cpp" 
	"src/litl-core/math/uncommon_tests.cpp" 
	"src/litl-core/math/geomesh_tests.cpp" 
	"src/litl-import/importLitlMat_tests.cpp" 
	"src/litl-import/importSlang_tests.cpp")

target_link_libraries(litl-tests
	PRIVATE
		litl-engine
		Catch2::Catch2WithMain
)

target_compile_features(litl-tests
	PUBLIC
		cxx_std_20
)

target_compile_options(litl-tests PRIVATE 
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fno-rtti>
    $<$<CXX_COMPILER_ID:MSVC>:/GR->
)

target_include_directories(litl-tests
	PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/include")

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
# individual asset copy
# ------------------------------------------------------------------------------------------

set (LITL_TEST_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/assets")

set (LITL_TESTS_ASSETS
	"${LITL_TEST_ASSETS_DIR}/mesh/bunny.obj"
	"${LITL_TEST_ASSETS_DIR}/materials/flat.litlmat"
	"${LITL_TEST_ASSETS_DIR}/shaders/test.slang"
)

foreach(TEST_ASSET_PATH IN LISTS LITL_TESTS_ASSETS)
	# Get just the sub-path (ie "mesh/bunny.obj")
	string(REPLACE ${LITL_TEST_ASSETS_DIR} "" RELATIVE_ASSET "${TEST_ASSET_PATH}")

	add_custom_command(
		TARGET litl-tests POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${TEST_ASSET_PATH}
			"$<TARGET_FILE_DIR:litl-tests>/assets/${RELATIVE_ASSET}"
		COMMENT "Updating asset: ${RELATIVE_ASSET}"
		VERBATIM
	)
endforeach()

# ------------------------------------------------------------------------------------------
# test discovery
# ------------------------------------------------------------------------------------------

catch_discover_tests(litl-tests)