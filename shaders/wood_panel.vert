#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 tex_coords;

out vec3 vertex_color;
out vec2 vertex_tex_coords;

void main() {
  gl_Position = vec4(position, 1.0);
  vertex_color = color;
  vertex_tex_coords = tex_coords;
};
