add_library(litl-ecs STATIC 
	"src/litl-ecs/world.cpp" 
	"src/litl-ecs/archetype/archetype.cpp" 
	"src/litl-ecs/archetype/chunk.cpp" 
	"src/litl-ecs/archetype/archetypeRegistry.cpp" 
	"src/litl-ecs/archetype/chunkLayout.cpp" 
	"src/litl-ecs/component.cpp" 
	"src/litl-ecs/componentRegistry.cpp" 
	"src/litl-ecs/entityRegistry.cpp" 
	"src/litl-ecs/system/systemSchedule.cpp" 
	"src/litl-ecs/system/systemScheduler.cpp"
	"src/litl-ecs/system/system.cpp"
)

target_include_directories(litl-ecs
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-ecs PRIVATE litl-core)

target_compile_features(litl-ecs
	PUBLIC
		cxx_std_20
)