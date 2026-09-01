# ------------------------------------------------------------------------------------------
# litl-import
# ------------------------------------------------------------------------------------------

add_library(litl-import STATIC 
	"src/mesh/import/fbx.cpp"  
	"src/mesh/import/obj.cpp" 
	"src/mesh/import/gltf.cpp" 
	"src/mesh/import/glb.cpp" 
	"src/importService.cpp" 
	"src/mesh/export/meshExporter.cpp" 
	"src/importerRegistry.cpp" 
	"src/exporterRegistry.cpp" 
	"src/material/import/litlmat.cpp" 
	"src/material/export/materialExporter.cpp" 
	"src/material/intermediate/litlmatb.cpp" 
	"src/material/intermediate/materialIntermediateData.cpp" 
	"src/shader/export/shaderExporter.cpp" 
	"src/shader/import/spirv.cpp" 
	"src/shader/import/slang.cpp" 
	"src/shader/intermediate/litlshader.cpp" 
	"src/shader/intermediate/shaderIntermediateData.cpp")

target_include_directories(litl-import
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
		"${CMAKE_SOURCE_DIR}/deps/rapidobj/include"
)

target_link_libraries(litl-import PRIVATE litl-core litl-renderer)

target_compile_features(litl-import
	PUBLIC
		cxx_std_20
)

if (MSVC)
	target_compile_options(litl-import PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-import PRIVATE -fno-exceptions -fno-rtti)
endif()

# ------------------------------------------------------------------------------------------
# Slang (via Vulkan SDK)
# ------------------------------------------------------------------------------------------

target_include_directories(litl-import PRIVATE "$ENV{VULKAN_SDK}/Include")
target_link_libraries(litl-import PRIVATE "$ENV{VULKAN_SDK}/Lib/slang.lib")

# ------------------------------------------------------------------------------------------
# glaze (json, yaml, beve)
# ------------------------------------------------------------------------------------------

FetchContent_Declare(glaze GIT_REPOSITORY https://github.com/stephenberry/glaze.git GIT_TAG v8.2.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glaze)

target_link_libraries(litl-import PUBLIC glaze::glaze)