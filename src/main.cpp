#include <GLFW/glfw3.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <glad/glad.h>

#include <experimental/memory>
#include <filesystem>
#include <string>

#include "shader.hpp"

using std::experimental::make_observer;
using std::experimental::observer_ptr;

namespace fs = std::filesystem;

template<typename Func>
struct [[nodiscard]] scope_exit {
    scope_exit(Func&& func)
    : func_(std::forward<Func>(func)) {}
    ~scope_exit() {
        func_();
    }

private:
    Func func_;
};

template<typename Func>
scope_exit(Func&&) -> scope_exit<Func>;

[[noreturn]] static void error_handler(int error_code, const char* message) {
    fmt::print(stderr, fmt::fg(fmt::color::red), FMT_STRING("Error ({0}): {1}\n"), error_code, message);
    glfwTerminate();
    std::exit(error_code);
}

int main(int, const char** argv) {
    if (!glfwInit()) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "Failed to initialize GLFW\n");
        return 1;
    }

    auto on_exit = scope_exit([] {
        fmt::print(fmt::fg(fmt::color::green), "Closing down GLFW\n");
        glfwTerminate();
    });

    glfwSetErrorCallback(&error_handler);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false); // This makes it float in i3

    auto window = make_observer(glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr));

    glfwMakeContextCurrent(window.get());

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "Failed to initialize GLAD\n");
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    });

    // FIXME: Copy shader directory to build dir in cmake?
    auto shader_dir = fs::path{ argv[0] }.parent_path() / "../shaders";
    shader shader{ shader_dir / "basic.vert", shader_dir / "basic.frag" };

    auto vertices = std::array{
        // positions       
         0.5f, -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
         0.0f,  0.5f, 0.0f, // top 

        // colors
        1.0f, 0.0f, 0.0f,   // bottom right
        0.0f, 1.0f, 0.0f,   // bottom left
        0.0f, 0.0f, 1.0f    // top 
    };

    auto indices = std::array{ 0, 1, 2 };

    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(9 * sizeof(float)));
    glEnableVertexAttribArray(1);

    shader.use();

    while (!glfwWindowShouldClose(window.get())) {
        if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window.get(), true);
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }
}
