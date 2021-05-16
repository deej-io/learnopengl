#ifndef LOGL_SHADER_HPP
#define LOGL_SHADER_HPP

#include <filesystem>

namespace fs = std::filesystem;

class shader {
public:
    shader(fs::path vertex_shader_path, fs::path fragment_shader_path);
    void use() const noexcept;
    void uniform(const std::string &name, float value) const noexcept;

private:
    unsigned int id;
};

#endif // LOGL_SHADER_HPP
