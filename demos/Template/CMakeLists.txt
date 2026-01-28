cmake_minimum_required (VERSION 3.8)
project(template VERSION 1.0.0 LANGUAGES C CXX)

add_executable(template)

target_sources(template PRIVATE
	 "src/main.cpp" 
	 "src/template.cpp"
	 ${LITL_SHARED_SOURCE})
	 

target_include_directories(template PUBLIC 
	${Vulkan_INCLUDE_DIRS} 
	"${glfw_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/include")

target_link_libraries(template PRIVATE glfw glm::glm ${Vulkan_LIBRARIES})
target_compile_definitions(template PRIVATE $<$<CONFIG:Debug>:DEBUG>)		# If the debug configuration pass the DEBUG define to the compiler