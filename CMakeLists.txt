cmake_minimum_required (VERSION 3.8)
project (LITL VERSION 1.0.0 LANGUAGES C CXX)

include(FetchContent)

# -- Compiler Setup --
set(CMAKE_CXX_STANDARD 20)

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

# -- GLM --

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.3)
FetchContent_MakeAvailable(glm)

# -- Shared Common and Deps --

set(LITL_DEPS "${CMAKE_SOURCE_DIR}/deps")
set(LITL_SHARED_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shared/src")
set(LITL_ASSETS "${CMAKE_SOURCE_DIR}/assets")

set(LITL_SHARED_SOURCE PRIVATE
	"${LITL_SHARED_SOURCE_DIR}/renderer/renderer.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/queueFamily.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/swapChainSupport.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/commandBuffer.cpp"
	"${LITL_SHARED_SOURCE_DIR}/renderer/mesh.cpp"
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

# -- Child Directories --

add_subdirectory(demos)