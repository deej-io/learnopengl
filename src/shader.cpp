#include "shader.hpp"

#include <fmt/color.h>
#include <fmt/format.h>
#include <glad/glad.h>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static auto load_shader(GLenum shader_type, fs::path filename) {
    std::ifstream file{ filename };
    std::string code{
        std::istreambuf_iterator<char>{ file },
        std::istreambuf_iterator<char>{}
    };

    const char* const code_c_str = code.c_str();

    auto shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &code_c_str, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        std::array<char, 512> info{};
        glGetShaderInfoLog(shader, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::COMPILATION_FAILED\n{}"), info.data());
        throw std::runtime_error{ "shader compile error" };
    }

    return shader;
}

shader::shader(fs::path vertex_shader_path, fs::path fragment_shader_path) {
    auto vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_path);
    auto fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_path);

    id = glCreateProgram();
    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);
    glLinkProgram(id);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        std::array<char, 512> info;
        glGetProgramInfoLog(id, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}"), info.data());
        throw std::runtime_error{ "shader link error" };
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void shader::use() const noexcept {
    glUseProgram(id);
}
