#include "gl_debug.hpp"

#include <fmt/format.h>
#include <glad/glad.h>

static const char* gl_debug_severity_to_string(GLenum severity) {
    // clang-format off
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:   return "!!!";
    case GL_DEBUG_SEVERITY_MEDIUM: return "!! ";
    case GL_DEBUG_SEVERITY_LOW:    return "!  ";
    }
    // clang-format on
    return "   ";
}

static const char* gl_debug_source_to_string(GLenum source) {
    // clang-format off
    switch (source) {
    case GL_DEBUG_SOURCE_API:             return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
    case GL_DEBUG_SOURCE_OTHER:           return "Other";
    }
    // clang-format on
    return "Unknown";
}

static const char* gl_debug_type_to_string(GLenum type) {
    // clang-format off
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
    case GL_DEBUG_TYPE_MARKER:              return "Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
    case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
    case GL_DEBUG_TYPE_OTHER:               return "Other";
    }
    // clang-format on
    return "Unknown";
}

static void gl_debug_handler(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void*) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    fmt::print(
        FMT_STRING("{} {} in {}: {}\n"),
        gl_debug_severity_to_string(severity),
        gl_debug_type_to_string(type),
        gl_debug_source_to_string(source),
        std::basic_string_view(message, static_cast<std::size_t>(length)));
}

void maybe_setup_opengl_logging() {
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        fmt::print("Enabling debug logging for OpenGL\n");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_handler, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}
