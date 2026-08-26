add_library(litl-engine STATIC
	"src/engine.cpp"
	"src/windowFactory.cpp" 
	"src/frameLimiter.cpp" 
	"src/config.cpp" 
	"src/bootstrap.cpp" 
	"src/scene/scenegraph.cpp" 
	"src/scene/partition/uniformGridPartition.cpp" 
	"src/scene/sceneManager.cpp" 
	"src/scene/scene.cpp"  
	"src/scene/sceneView.cpp" 
	"src/engineCallbacks.cpp" 
	"src/scene/sceneChangeProcessor.cpp"
	"src/containers/containerSpecializations.cpp"
	"src/objects/objectPool.cpp" 
	"src/objects/mesh.cpp" 
	"src/objects/gpuBuffer.cpp"  
	"src/scene/sceneTransforms.cpp" 
	"src/scene/sceneCulling.cpp"  
	"src/objects/camera.cpp" 
	"src/scene/sceneCameras.cpp" 
	"src/ecs/systems/cullingSystem.cpp" 
	"src/render/renderPass.cpp" 
	"src/render/renderManager.cpp" 
	"src/objects/material/material.cpp" 
	"src/assets/assetManager.cpp" 
    "src/assets/assetLoadTask.cpp"
	"src/tasks/taskManager.cpp" 
	"src/objects/texture2d.cpp" 
	"src/objects/text.cpp" 
	"src/tasks/ownedTask.cpp" 
	"src/assets/textAsset.cpp" 
	"src/assets/texture2dAsset.cpp" 
	"src/assets/meshAsset.cpp" 
	"src/assets/materialAsset.cpp"  "src/objects/material/materialProperties.cpp" "src/objects/material/materialManager.cpp" "src/ecs/systems/activeMaterialSystem.cpp" "src/objects/material/deferredMaterialCommands.cpp")

target_include_directories(litl-engine
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-engine 
	PUBLIC 
		litl-core 
		litl-import
		litl-ecs
		litl-renderer
)

if (LITL_ENABLE_VULKAN)
	target_link_libraries(litl-engine PUBLIC litl-renderer-vulkan)
endif()

target_compile_features(litl-engine
	PUBLIC
		cxx_std_20
)

target_compile_definitions(litl-engine
	PUBLIC LITL_RENDERER_VULKAN
)

if (MSVC)
	target_compile_options(litl-engine PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-engine PRIVATE -fno-exceptions -fno-rtti)
endif()