add_library(litl-import STATIC 
	"src/mesh/import/fbx.cpp" "src/mesh/import/meshImporter.cpp")

target_include_directories(litl-import
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-import PRIVATE litl-core)

target_compile_features(litl-import
	PUBLIC
		cxx_std_20
)

if (MSVC)
	target_compile_options(litl-import PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-import PRIVATE -fno-exceptions -fno-rtti)
endif()