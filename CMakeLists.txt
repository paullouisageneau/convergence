cmake_minimum_required(VERSION 3.7)
project(convergence
	DESCRIPTION "Convergence"
	VERSION 0.1.0
	LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

file(GLOB_RECURSE SOURCES_CONVERGENCE ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SOURCES_PLATFORM ${PROJECT_SOURCE_DIR}/pla/*.cpp)

add_executable(convergence ${SOURCES_PLATFORM} ${SOURCES_CONVERGENCE})
set_target_properties(convergence PROPERTIES
	VERSION ${PROJECT_VERSION}
	CXX_STANDARD 17)

target_include_directories(convergence PRIVATE ${PROJECT_SOURCE_DIR}/)

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(DevIL REQUIRED)
if(NOT TARGET DevIL::IL)
	add_library(DevIL::IL UNKNOWN IMPORTED)
	set_target_properties(DevIL::IL PROPERTIES
		IMPORTED_LOCATION "${IL_LIBRARIES}"
		INTERFACE_INCLUDE_DIRECTORIES "${IL_INCLUDE_DIR}"
		IMPORTED_LINK_INTERFACE_LANGUAGES C)
endif()

target_link_libraries(convergence OpenGL::GL GLEW::GLEW GLFW::GLFW DevIL::IL)

add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
target_link_libraries(convergence glm)

add_subdirectory(deps/datachannels EXCLUDE_FROM_ALL)
target_link_libraries(convergence datachannels)

