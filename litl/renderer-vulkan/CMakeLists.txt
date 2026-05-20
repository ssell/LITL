# Vulkan
find_package(Vulkan REQUIRED)

if (VULKAN_FOUND)
	message(STATUS "Vulkan SDK found")
else()
	message(FATAL_ERROR "Vulkan SDK not found!")
endif()

# Volk
FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG        1.4.350
    GIT_SHALLOW    TRUE
)
set(VOLK_PULL_IN_VULKAN ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(volk)

# GLFW
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Vulkan Memory Allocator (VMA)
FetchContent_Declare(VulkanMemoryAllocator GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git GIT_TAG v3.3.0)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

add_library(litl-renderer-vulkan 
	STATIC
		"src/litl-renderer-vulkan/window.cpp" 
		"src/litl-renderer-vulkan/renderer.cpp"
		"src/litl-renderer-vulkan/resourceManager.cpp" 
		"src/litl-renderer-vulkan/swapChainSupport.cpp"
		"src/litl-renderer-vulkan/impl/impl.cpp"
 "src/litl-renderer-vulkan/ops/rendererResourceOps.cpp" "src/litl-renderer-vulkan/ops/rendererDrawOps.cpp" "src/litl-renderer-vulkan/ops/rendererCommandOps.cpp" "src/litl-renderer-vulkan/impl/validate.cpp")

target_include_directories(litl-renderer-vulkan
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
		${Vulkan_INCLUDE_DIRS}
)

# Tell every translation unit that includes vulkan.h to skip prototypes so Volk's function pointers are the only entry points.
target_compile_definitions(litl-renderer-vulkan
	PUBLIC
		VK_NO_PROTOTYPES
)

target_link_libraries(litl-renderer-vulkan
	PRIVATE
		litl-core
		litl-renderer
		glfw
		Vulkan::Headers
		GPUOpen::VulkanMemoryAllocator
		volk::volk_headers
)

target_compile_features(litl-renderer-vulkan
	PUBLIC
		cxx_std_20
)


if (MSVC)
	target_compile_options(litl-renderer-vulkan PRIVATE /EHs-c- /D_HAS_EXCEPTIONS=0 /GR-)
else()
	target_compile_options(litl-renderer-vulkan PRIVATE -fno-exceptions -fno-rtti)
endif()