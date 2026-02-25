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

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------------------
# -- Shared Common and Deps
# ------------------------------------------------------------------------------------------

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