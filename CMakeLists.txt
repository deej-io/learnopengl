cmake_minimum_required(VERSION 3.9)
set(GLAD_PROFILE "core")

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

add_executable(glm glm/main.cpp)
target_compile_features(glm PRIVATE cxx_std_20)
target_compile_options(glm PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wextra -Wall>)
target_link_libraries(glm PRIVATE glm::glm fmt::fmt)
