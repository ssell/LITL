cmake_minimum_required (VERSION 3.10)
project (litl VERSION 1.0.0 LANGUAGES C CXX)

include(FetchContent)

# -- Compiler Setup --
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

set(LITL_SHARED_HEADER_DIR "${CMAKE_SOURCE_DIR}/litl/include")
set(LITL_SHARED_SOURCE_DIR "${CMAKE_SOURCE_DIR}/litl/src")
set(LITL_VULKAN_HEADER_DIR "${CMAKE_SOURCE_DIR}/libs/vulkan/include")
set(LITL_VULKAN_SOURCE_DIR "${CMAKE_SOURCE_DIR}/libs/vulkan/src")
set(LITL_ASSETS_DIR        "${CMAKE_SOURCE_DIR}/assets")

SET(LITL_SHARED_SOURCE 
    PRIVATE
        "${LITL_SHARED_SOURCE_DIR}/engine.cpp"
)

SET(LITL_VULKAN_SOURCE
	PRIVATE
		"${LITL_VULKAN_SOURCE_DIR}/renderer.cpp"
)

include_directories(
	"${LITL_SHARED_HEADER_DIR}"
	"${LITL_VULKAN_HEADER_DIR}"
)

# -- Child Directories --

add_subdirectory(samples)