#version 330 core

out vec4 fragment_color;
in vec3 vertex_color;

void main() {
    fragment_color = vec4(vertex_color, 1.0f);
};
