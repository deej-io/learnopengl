#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;

out vec3 vertex_color;
out vec2 vertex_tex_coords;

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  vertex_tex_coords = tex_coords;
};
