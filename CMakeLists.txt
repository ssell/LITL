cmake_minimum_required (VERSION 3.16)
project (litl VERSION 1.0.0 LANGUAGES C CXX)

include(FetchContent)

# ------------------------------------------------------------------------------------------
# -- Options
# ------------------------------------------------------------------------------------------

option(LITL_BUILD_TESTS "Build Tests" ON)
option(LITL_ENABLE_VULKAN "Enable Vulkan Renderer" ON)

# ------------------------------------------------------------------------------------------
# -- Compiler Setup
# ------------------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ------------------------------------------------------------------------------------------
# -- Shared Common and Deps
# ------------------------------------------------------------------------------------------

# GLM
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.3)
FetchContent_MakeAvailable(glm)

# Assets
set(LITL_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

# ------------------------------------------------------------------------------------------
# -- Child Directories
# ------------------------------------------------------------------------------------------

add_subdirectory(litl/core)
add_subdirectory(litl/engine)
add_subdirectory(samples)

if (LITL_ENABLE_VULKAN)
	add_subdirectory(litl/renderer-vulkan)
endif()

if (LITL_BUILD_TESTS)
	add_subdirectory(tests)
endif()