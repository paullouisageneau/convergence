cmake_minimum_required(VERSION 3.7)
project(convergence
	DESCRIPTION "Convergence"
	VERSION 0.1.0
	LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

file(GLOB_RECURSE SOURCES_CONVERGENCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SOURCES_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/pla/*.cpp)

add_executable(convergence ${SOURCES_PLATFORM} ${SOURCES_CONVERGENCE})
set_target_properties(convergence PROPERTIES CXX_STANDARD 17)

target_include_directories(convergence PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)

if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(WASM_OPTS
		-O3 -flto -fno-rtti
		"SHELL:-s WASM=1"
		"SHELL:-s BINARYEN_METHOD=native-wasm"
		"SHELL:-s TOTAL_MEMORY=256MB"
		"SHELL:-s DISABLE_EXCEPTION_CATCHING=1"
		"SHELL:-s USE_PTHREADS=0"
		"SHELL:-s USE_WEBGL2=1"
		"SHELL:-s USE_GLFW=3"
		"SHELL:-s USE_SDL_IMAGE=2"
		"SHELL:-s USE_FREETYPE=1")

	target_compile_options(convergence PRIVATE ${WASM_OPTS} -DUSE_OPENGL_ES)
	target_link_options(convergence PRIVATE ${WASM_OPTS}
		--closure 1
		--use-preload-plugins
		"SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shader@shader"
		"SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/res@res"
		"SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/html/index.html")

	set_target_properties(convergence PROPERTIES OUTPUT_NAME "convergence")
	set_target_properties(convergence PROPERTIES SUFFIX ".html")
else()
	cmake_policy(SET CMP0072 NEW)
	find_package(OpenGL REQUIRED)
	find_package(GLEW REQUIRED)
	find_package(GLFW REQUIRED)
	find_package(DevIL REQUIRED)
	find_package(Freetype REQUIRED)
	if(NOT TARGET DevIL::IL)
		add_library(DevIL::IL UNKNOWN IMPORTED)
		set_target_properties(DevIL::IL PROPERTIES
			IMPORTED_LOCATION "${IL_LIBRARIES}"
			INTERFACE_INCLUDE_DIRECTORIES "${IL_INCLUDE_DIR}"
			IMPORTED_LINK_INTERFACE_LANGUAGES C)
	endif()
	target_link_libraries(convergence OpenGL::GL GLEW::GLEW GLFW::GLFW DevIL::IL Freetype::Freetype)
endif()

option(BUILD_SHARED_LIBS "Build shared library" OFF)
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
target_link_libraries(convergence glm)

add_subdirectory(deps/datachannel-wasm EXCLUDE_FROM_ALL)
target_link_libraries(convergence datachannel-wasm)

