#version 330 core

out vec4 fragment_color;
in vec2 vertex_tex_coords;

uniform sampler2D wood_texture;
uniform sampler2D face_texture;
uniform float blend_value;

void main() {
  vec2 x_flipped_tex_coords = vec2(-vertex_tex_coords.x, vertex_tex_coords.y);
  fragment_color = mix(
      texture(wood_texture, vertex_tex_coords),
      texture(face_texture, x_flipped_tex_coords),
      blend_value
  );
};
