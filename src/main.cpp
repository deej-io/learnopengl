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

static const char *fragment_shader_source = R"(
#version 330 core

out vec4 FragColor;

void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)";

int main() {

    if (!glfwInit()) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "Failed to initialize GLFW");
        return 1;
    }

    auto on_exit = scope_exit([]{
        fmt::print(fmt::fg(fmt::color::green), "Closing down GLFW");
        glfwTerminate();
    });

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

    auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        std::array<char, 512> info;
        glGetShaderInfoLog(fragment_shader, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}"), info.data());
        return -1;
    }

    auto shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        std::array<char, 512> info;
        glGetProgramInfoLog(shader_program, info.size(), nullptr, info.data());
        fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}"), info.data());
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    auto vertices = std::array<float, 18> {
        -0.9f, -0.9f, 0.0f,
        -0.1f, -0.9f, 0.0f,
        -0.5 ,  0.9f, 0.0f,
         0.1f, -0.9f, 0.0f,
         0.9f, -0.9f, 0.0f,
         0.5 ,  0.9f, 0.0f
    };

    auto indices = std::array {
        0, 1, 2,
        3, 4, 5
    };

    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    while(!glfwWindowShouldClose(window.get()))
    {
        if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window.get(), true);
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }
}
