add_library(litl-platform STATIC
	"src/window/window.cpp" 
	"src/platformConfiguration.cpp"
	"src/platform.cpp" 
	"src/input/input.cpp" 
	"src/input/inputCallbacks.cpp" 
	"src/window/windowConfiguration.cpp"
	"src/input/actionMap.cpp" 
	"src/input/inputActions.cpp" 
	"src/input/inputConfiguration.cpp" 
	"src/input/processedInputTracker.cpp" "src/window/windowCallbacks.cpp")

target_include_directories(litl-platform PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(litl-platform PUBLIC litl-core)
target_compile_features(litl-platform PUBLIC cxx_std_23)

if (MSVC)
	target_compile_options(litl-platform PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-platform PRIVATE -fno-exceptions -fno-rtti)
endif()