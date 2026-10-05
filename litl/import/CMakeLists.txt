# ------------------------------------------------------------------------------------------
# litl-import
# ------------------------------------------------------------------------------------------

add_library(litl-import STATIC 
	"src/mesh/import/fbx.cpp"  
	"src/mesh/import/gltf.cpp" 
	"src/importService.cpp" 
	"src/mesh/export/meshExporter.cpp" 
	"src/mesh/intermediate/litlbmsh.cpp" 
	"src/importerRegistry.cpp" 
	"src/exporterRegistry.cpp" 
	"src/material/import/litlmat.cpp" 
	"src/material/export/materialExporter.cpp" 
	"src/material/intermediate/litlbmat.cpp" 
	"src/material/intermediate/materialIntermediateData.cpp" 
	"src/shader/export/shaderExporter.cpp" 
	"src/shader/import/spirv.cpp" 
	"src/shader/import/slang.cpp" 
	"src/shader/intermediate/litlbshd.cpp" 
	"src/shader/intermediate/shaderIntermediateData.cpp" 
	"src/model/export/modelExporter.cpp" 
	"src/model/import/obj.cpp" 
	"src/model/intermediate/modelIntermediateData.cpp" 
	"src/model/intermediate/litlmdl.cpp" 
	"src/importedData.cpp" 
	"src/texture/import/stb.cpp" 
	"src/texture/export/textureExporter.cpp" 
	"src/texture/intermediate/textureIntermediateData.cpp" 
	"src/texture/intermediate/litlbtex.cpp" 
	"src/texture/import/tga.cpp" 
	"src/importer.cpp" 
	"src/texture/import/png.cpp" 
	"src/texture/import/bmp.cpp" 
	"src/model/import/glb.cpp"
 "src/model/import/cgltf.cpp")

target_include_directories(litl-import
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
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

FetchContent_Declare(glaze GIT_REPOSITORY https://github.com/stephenberry/glaze.git GIT_TAG v8.2.0 GIT_SHALLOW TRUE SYSTEM)
FetchContent_MakeAvailable(glaze)

target_link_libraries(litl-import PRIVATE glaze::glaze)

# ------------------------------------------------------------------------------------------
# rapidobj (obj)
# ------------------------------------------------------------------------------------------

FetchContent_Declare(rapidobj GIT_REPOSITORY https://github.com/guybrush77/rapidobj.git GIT_TAG v1.1 GIT_SHALLOW TRUE SYSTEM)
FetchContent_MakeAvailable(rapidobj)

target_link_libraries(litl-import PRIVATE rapidobj::rapidobj)

# ------------------------------------------------------------------------------------------
# cgltf (gltf, glb)
# ------------------------------------------------------------------------------------------

FetchContent_Declare(cgltf GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git GIT_TAG v1.15 GIT_SHALLOW TRUE SYSTEM)
FetchContent_MakeAvailable(cgltf)

target_include_directories(litl-import SYSTEM PRIVATE "${cgltf_SOURCE_DIR}")

# ------------------------------------------------------------------------------------------
# stb (png, bmp, tga, etc.)
# ------------------------------------------------------------------------------------------

FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG 2c980bb59875b0d32144a71867fbdebb2f77cd20 GIT_SHALLOW TRUE SYSTEM)
FetchContent_MakeAvailable(stb)

target_include_directories(litl-import SYSTEM PRIVATE "${stb_SOURCE_DIR}")

# ------------------------------------------------------------------------------------------
# ufbx (fbx)
# ------------------------------------------------------------------------------------------

FetchContent_Declare(ufbx GIT_REPOSITORY https://github.com/ufbx/ufbx.git GIT_TAG v0.23.1 GIT_SHALLOW TRUE SYSTEM)
FetchContent_MakeAvailable(ufbx)

target_include_directories(litl-import SYSTEM PRIVATE "${ufbx_SOURCE_DIR}")
target_sources(litl-import PRIVATE "${ufbx_SOURCE_DIR}/ufbx.c")

# Future todo for ufbx: can make it is own small static library to both avoid third-party C code in litl-import and so it doesn't pick up our C++ only compile options.