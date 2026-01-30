# -- Vulkan SDK --
find_package(Vulkan REQUIRED)

if (VULKAN_FOUND)
	message(STATUS "Vulkan SDK found")
elseif (VULKAN_FOUND)
	message(FATAL_ERROR "Vulkan SDK not found!")
endif (VULKAN_FOUND)

# -- GLFW --
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)


add_library(litl-renderer-vulkan STATIC
	"src/renderer.cpp"
)

target_include_directories(litl-renderer-vulkan
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(litl-renderer-vulkan
	PRIVATE
		litl-core
)

target_compile_features(litl-renderer-vulkan
	PUBLIC
		cxx_std_20
)