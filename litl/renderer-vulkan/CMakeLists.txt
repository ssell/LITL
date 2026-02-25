# Vulkan
find_package(Vulkan REQUIRED)

if (VULKAN_FOUND)
	message(STATUS "Vulkan SDK found")
elseif (VULKAN_FOUND)
	message(FATAL_ERROR "Vulkan SDK not found!")
endif (VULKAN_FOUND)

# GLFW
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

add_library(litl-renderer-vulkan STATIC
	"src/litl-renderer-vulkan/renderer.cpp" "src/litl-core-vulkan/window.cpp" "src/litl-renderer-vulkan/queueFamily.cpp" "src/litl-renderer-vulkan/swapchainSupport.cpp" "src/litl-renderer-vulkan/commands/commandBuffer.cpp" "src/litl-renderer-vulkan/pipeline/pipelineLayout.cpp" "src/litl-renderer-vulkan/pipeline/graphicsPipeline.cpp" "src/litl-renderer-vulkan/resourceAllocator.cpp" "src/litl-renderer-vulkan/pipeline/shaderModule.cpp")

target_include_directories(litl-renderer-vulkan
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE 
		${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(litl-renderer-vulkan
	PRIVATE
		litl-core
		glfw
		${Vulkan_LIBRARIES}
)

target_compile_features(litl-renderer-vulkan
	PUBLIC
		cxx_std_23
)