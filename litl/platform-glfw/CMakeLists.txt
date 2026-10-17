add_library(litl-platform-glfw STATIC
	"src/window.cpp" 
	"src/platform.cpp" 
	"src/input.cpp")

target_include_directories(litl-platform-glfw PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" PRIVATE "src/include")
target_link_libraries(litl-platform-glfw PUBLIC litl-platform)
target_compile_features(litl-platform-glfw PUBLIC cxx_std_23)

if (MSVC)
	target_compile_options(litl-platform-glfw PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-platform-glfw PRIVATE -fno-exceptions -fno-rtti)
endif()

# GLFW
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4 GIT_SHALLOW TRUE SYSTEM)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

target_link_libraries(litl-platform-glfw PRIVATE glfw)