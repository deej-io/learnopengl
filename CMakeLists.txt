cmake_minimum_required(VERSION 3.9)

project(learnopengl VERSION 0.1.0)

find_package(fmt REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Stb REQUIRED)
find_package(glm REQUIRED)

add_executable(learnopengl
	src/main.cpp
	src/gl_debug.cpp
	src/shader.cpp
	src/vendor/stb_image.cpp
)

target_compile_features(learnopengl PUBLIC cxx_std_20)
target_compile_options(learnopengl PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wextra -Wall>)
target_compile_definitions(learnopengl PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(learnopengl PUBLIC fmt::fmt glad::glad glfw)

option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

if(ENABLE_INCLUDE_WHAT_YOU_USE)
	find_program(INCLUDE_WHAT_YOU_USE include-what-you-use REQUIRED)
	set_property(TARGET learnopengl PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
endif()

if(ENABLE_CLANG_TIDY)
	find_program(CLANG_TIDY clang-tidy REQUIRED)
	set_property(TARGET learnopengl PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY})
endif()


