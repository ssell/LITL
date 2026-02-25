add_library(litl-engine STATIC
	"src/engine.cpp"
	"src/windowFactory.cpp" 
	"src/rendererFactory.cpp" "src/cache/pipelineLayoutCache.cpp" "src/scene/sceneTree.cpp"  "src/scene/sceneObject.cpp"   "src/ecs/world.cpp"  "src/ecs/archetype/archetype.cpp" "src/ecs/archetype/chunk.cpp"  "src/ecs/archetype/archetypeRegistry.cpp" "src/ecs/archetype/chunkLayout.cpp" "src/ecs/component.cpp" "src/ecs/componentRegistry.cpp" "src/ecs/entityRegistry.cpp" "src/ecs/system/systemSchedule.cpp" "src/ecs/system/systemScheduler.cpp" "src/ecs/system/systemRegistry.cpp" "src/ecs/system/systemWrapper.cpp" "src/ecs/system/system.cpp")

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
		cxx_std_23
)