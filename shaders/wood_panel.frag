#version 330 core

out vec4 fragment_color;
in vec3 vertex_color;
in vec2 vertex_tex_coords;

uniform sampler2D wood_texture;
uniform sampler2D face_texture;

void main() {
  fragment_color = mix(
      texture(wood_texture, vertex_tex_coords),
      texture(face_texture, vertex_tex_coords),
      0.2
  );
};
