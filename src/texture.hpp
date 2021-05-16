#ifndef LOGL_TEXTURE_HPP
#define LOGL_TEXTURE_HPP

#include <fmt/format.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

class texture {
public:
    texture(fs::path filename) {
        if (!fs::exists(filename)) {
            throw std::invalid_argument(fmt::format(FMT_STRING("file {} does not exist"), filename.c_str()));
        }

        int width, height, channels;

        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

        if (!data) {
            throw std::invalid_argument(fmt::format(FMT_STRING("file {} is not a valid image format"), filename.c_str()));
        }

        // FIXME: this is probably brittle!
        auto format = channels == 4 ? GL_RGBA : GL_RGB;

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void bind(GLenum texture_unit) {
        glActiveTexture(texture_unit);
        bind();
    }

private:
    unsigned int id;
};

#endif // LOGL_TEXTURE_HPP
