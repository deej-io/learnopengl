#include <GLFW/glfw3.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <experimental/memory>
#include <filesystem>
#include <string>
#include <utility>

#include "gl_debug.hpp"
#include "shader.hpp"
#include "texture.hpp"

using std::experimental::make_observer;

namespace fs = std::filesystem;

template<typename Func>
struct [[nodiscard]] scope_exit {
    explicit scope_exit(Func&& func)
    : func_(std::forward<Func>(func)) {}
    scope_exit(const scope_exit&) = delete;
    scope_exit(scope_exit&&) = delete;
    scope_exit& operator=(const scope_exit&) = delete;
    scope_exit& operator=(scope_exit&&) = delete;
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

template<typename T, glm::qualifier Q, typename F>
auto operator|(const glm::mat<4, 4, T, Q>& m, F&& f) {
    return std::forward<F>(f)(m);
}

template<typename T, glm::qualifier Q = glm::qualifier::defaultp>
static auto scale(glm::vec<3, T, Q> const& scale) {
    return [&scale](glm::mat<4, 4, T, Q> const& mat) {
        return glm::scale(mat, scale);
    };
}

template<typename T, glm::qualifier Q>
static auto translate(glm::vec<3, T, Q> const& direction) {
    return [&direction](glm::mat<4, 4, T, Q> const& mat) {
        return glm::translate(mat, direction);
    };
}

template<typename A, typename T, glm::qualifier Q>
static auto rotate(A angle, glm::vec<3, T, Q> const& axis) {
    return [angle, &axis](glm::mat<4, 4, T, Q> const& mat) {
        return glm::rotate(mat, angle, axis);
    };
}

struct window_data {
    float width;
    float height;
    float blend_value;
};

static void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS)
        return;

    auto* data = static_cast<window_data*>(glfwGetWindowUserPointer(window));

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;
    case GLFW_KEY_UP:
        data->blend_value = std::clamp(data->blend_value + 0.1f, 0.0f, 1.0f);
        break;
    case GLFW_KEY_DOWN:
        data->blend_value = std::clamp(data->blend_value - 0.1f, 0.0f, 1.0f);
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

    glEnable(GL_DEPTH_TEST);

    window_data window_data{ 800, 600, 0.2f };

    glfwSetWindowUserPointer(window.get(), &window_data);
    glfwSetKeyCallback(window.get(), key_callback);

    glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    });

    // FIXME: Copy shader directory to build dir in cmake?
    auto exec_dir = fs::path{ argv[0] }.parent_path();
    auto shader_dir = exec_dir / "../shaders";
    shader shader{ shader_dir / "wood_panel.vert", shader_dir / "wood_panel.frag" };

    // clang-format off
    auto vertices = std::array{
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    // clang-format on

    auto cube_positions = std::array{
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };

    unsigned int VAO = 0, VBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    //
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    shader.use();

    auto textures_dir = exec_dir / "../textures";
    texture wood_panel_texture{ textures_dir / "container.jpg" };
    texture face_texture{ textures_dir / "awesomeface.png" };

    shader.uniform("wood_texture", texture_unit_index(GL_TEXTURE0));
    shader.uniform("face_texture", texture_unit_index(GL_TEXTURE1));

    while (!glfwWindowShouldClose(window.get())) {
        const auto angle = static_cast<float>(glfwGetTime());

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);

        wood_panel_texture.bind(GL_TEXTURE0);
        face_texture.bind(GL_TEXTURE1);

        shader.uniform("blend_value", window_data.blend_value);

        glm::mat4 view = glm::identity<glm::mat4>() | translate(glm::vec3(0.0f, 0.0f, -3.0f));
        shader.uniform("view", view);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), window_data.width / window_data.height, 0.1f, 100.0f);
        shader.uniform("projection", projection);

        for (std::size_t i = 0; i < cube_positions.size(); ++i) {
            const auto model =
                glm::identity<glm::mat4>()
                | translate(cube_positions[i])
                | rotate(glm::radians(20.f * (static_cast<float>(i) + angle)), glm::vec3(1.0f, 0.3f, 0.5f));

            shader.uniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }
}
