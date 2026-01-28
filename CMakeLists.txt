cmake_minimum_required (VERSION 3.8)
project (LITL VERSION 1.0.0 LANGUAGES CXX)

# -- Compiler Setup --
set(CMAKE_CXX_STANDARD 20)

# -- Vulkan SDK --
find_package(Vulkan REQUIRED)

if (VULKAN_FOUND)
	message(STATUS "Vulkan SDK found")
elseif (VULKAN_FOUND)
	message(FATAL_ERROR "Vulkan SDK not found!")
endif (VULKAN_FOUND)

# -- Shared Common and Deps --

set(LITL_DEPS "${CMAKE_SOURCE_DIR}/deps")
set(LITL_SHARED_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shared/src")
set(LITL_ASSETS "${CMAKE_SOURCE_DIR}/assets")

set(LITL_SHARED_SOURCE PRIVATE
	"${LITL_SHARED_SOURCE_DIR}/renderer/renderer.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/queueFamily.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/swapChainSupport.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/commandBuffer.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/shader.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/vertexShader.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/preTessellationShader.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/postTessellationShader.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/geometryShader.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/fragmentShader.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/computeShader.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/shader/shaderProgram.cpp"
)

include_directories("${CMAKE_SOURCE_DIR}/shared/include")

# -- GLFW --

add_library(GLFW STATIC IMPORTED)
set_target_properties(GLFW PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${LITL_DEPS}/include/"
	IMPORTED_LOCATION "${LITL_DEPS}/lib/Debug/x64/glfw3.lib")

# -- Child Directories --

add_subdirectory(demos)