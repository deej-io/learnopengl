#include <GLFW/glfw3.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <experimental/memory>
#include <filesystem>
#include <sstream>
#include <string>

#include "gl_debug.hpp"
#include "shader.hpp"
#include "texture.hpp"

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

[[nodiscard]] static int texture_unit_index(GLenum texture_unit) {
    return static_cast<int>(texture_unit - GL_TEXTURE0);
}

static void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS)
        return;

    float* blend_value = static_cast<float*>(glfwGetWindowUserPointer(window));

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;
    case GLFW_KEY_UP:
        *blend_value = std::clamp(*blend_value + 0.1f, 0.0f, 1.0f);
        break;
    case GLFW_KEY_DOWN:
        *blend_value = std::clamp(*blend_value - 0.1f, 0.0f, 1.0f);
        break;
    }
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false); // This makes it float in i3

    auto window = make_observer(glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr));

    glfwMakeContextCurrent(window.get());

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "Failed to initialize GLAD\n");
        return -1;
    }

    maybe_setup_opengl_logging();

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    });

    // FIXME: Copy shader directory to build dir in cmake?
    auto exec_dir = fs::path{ argv[0] }.parent_path();
    auto shader_dir = exec_dir / "../shaders";
    shader shader{ shader_dir / "wood_panel.vert", shader_dir / "wood_panel.frag" };

    // clang-format off
    auto vertices = std::array{
        // positions
         0.5f,  0.5f, 0.0f, // top right
         0.5f, -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f, // top left P

        // colors
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,

        // texture coords
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };

    auto indices = std::array{
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    // clang-format on

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

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(12 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(24 * sizeof(float)));
    glEnableVertexAttribArray(2);

    shader.use();

    auto textures_dir = exec_dir / "../textures";
    texture wood_panel_texture{ textures_dir / "container.jpg" };
    texture face_texture{ textures_dir / "awesomeface.png" };

    shader.uniform("wood_texture", texture_unit_index(GL_TEXTURE0));
    shader.uniform("face_texture", texture_unit_index(GL_TEXTURE1));

    float blend_value = 0.2f;
    glfwSetWindowUserPointer(window.get(), &blend_value);
    glfwSetKeyCallback(window.get(), key_callback);

    while (!glfwWindowShouldClose(window.get())) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        wood_panel_texture.bind(GL_TEXTURE0);
        face_texture.bind(GL_TEXTURE1);
        shader.uniform("blend_value", blend_value);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }
}
