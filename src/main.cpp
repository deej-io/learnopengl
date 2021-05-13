#include <experimental/memory>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "fmt/format.h"
#include "fmt/color.h"

using std::experimental::observer_ptr;
using std::experimental::make_observer;

template <typename Func>
struct [[nodiscard]] scope_exit {
    scope_exit(Func&& func) : func_(std::forward<Func>(func)) {}
    ~scope_exit() { func_(); }
private:
    Func func_;
};

template <typename Func> scope_exit(Func &&) -> scope_exit<Func>;

[[noreturn]] static void error_handler(int error_code, const char *message) {
    fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("Error ({0}): {1}"), error_code, message);
    glfwTerminate();
    std::exit(error_code);
}

static const char *vertex_shader_source = R"(
#version 330 core

layout (location = 0) in vec3 position;

void main() {
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
}
)";

static std::string fragment_shader_source(std::string_view color) {
    const char *format = R"(
#version 330 core

out vec4 FragColor;

void main() {{
    FragColor = vec4({0}, 1.0f);
}}
)";

    return fmt::format(format, color);
}

int main() {

    if (!glfwInit()) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "Failed to initialize GLFW");
        return 1;
    }

    auto on_exit = scope_exit(&glfwTerminate);

    glfwSetErrorCallback(&error_handler);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false); // This makes it float in i3

    auto window = make_observer(glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr));

    glfwMakeContextCurrent(window.get());

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        fmt::print(stderr, fmt::fg(fmt::color::red), "Failed to initialize GLAD");
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow *, int width, int height) {
        glViewport(0, 0, width, height);
    });

    auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    int success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        std::array<char, 512> info;
        glGetShaderInfoLog(vertex_shader, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}"), info.data());
        return -1;
    }

    const auto fragment_shader_source_1 = fragment_shader_source("0.7f, 0.2f, 0.1f");
    const auto fragment_shader_source_2 = fragment_shader_source("0.1f, 0.2f, 0.7f");
    const auto fragment_shader_source_1_cstr = fragment_shader_source_1.c_str();
    const auto fragment_shader_source_2_cstr = fragment_shader_source_2.c_str();

    auto fragment_shader_1 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_1, 1, &fragment_shader_source_1_cstr, nullptr);
    glCompileShader(fragment_shader_1);

    glGetShaderiv(fragment_shader_1, GL_COMPILE_STATUS, &success);

    if (!success) {
        std::array<char, 512> info;
        glGetShaderInfoLog(fragment_shader_1, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}"), info.data());
        return -1;
    }

    auto fragment_shader_2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_2, 1, &fragment_shader_source_2_cstr, nullptr);
    glCompileShader(fragment_shader_2);

    glGetShaderiv(fragment_shader_1, GL_COMPILE_STATUS, &success);

    if (!success) {
        std::array<char, 512> info;
        glGetShaderInfoLog(fragment_shader_2, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}"), info.data());
        return -1;
    }

    auto shader_program_1 = glCreateProgram();
    glAttachShader(shader_program_1, vertex_shader);
    glAttachShader(shader_program_1, fragment_shader_1);
    glLinkProgram(shader_program_1);

    glGetProgramiv(shader_program_1, GL_LINK_STATUS, &success);
    if(!success) {
        std::array<char, 512> info;
        glGetProgramInfoLog(shader_program_1, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}"), info.data());
        return -1;
    }

    auto shader_program_2 = glCreateProgram();
    glAttachShader(shader_program_2, vertex_shader);
    glAttachShader(shader_program_2, fragment_shader_2);
    glLinkProgram(shader_program_2);

    glGetProgramiv(shader_program_2, GL_LINK_STATUS, &success);
    if(!success) {
        std::array<char, 512> info;
        glGetProgramInfoLog(shader_program_2, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}"), info.data());
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader_1);
    glDeleteShader(fragment_shader_2);

    auto vertices_1 = std::array<float, 18> {
        -0.9f, -0.9f, 0.0f,
        -0.1f, -0.9f, 0.0f,
        -0.5 ,  0.9f, 0.0f,
    };

    auto vertices_2 = std::array<float, 18> {
         0.1f, -0.9f, 0.0f,
         0.9f, -0.9f, 0.0f,
         0.5 ,  0.9f, 0.0f
    };

    unsigned int VAOs[2], VBOs[2];

    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);


    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_1), vertices_1.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_2), vertices_2.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    while(!glfwWindowShouldClose(window.get())) {
        if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window.get(), true);
        }

        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program_1);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shader_program_2);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }
}
