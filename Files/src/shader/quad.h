#pragma once

constexpr const char* QUAD_VERTEX_SHADER = // vertex shader:
R"glsl(
#version 330 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 in_texture_coords;

out vec2 texture_coords;

uniform mat4 vm;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vm * vec4(vertex, 0.0, 1.0);
    texture_coords = in_texture_coords;
}
)glsl";

constexpr const char* QUAD_FRAGMENT_SHADER = // fragment shader:
R"glsl(
#version 330 core
in vec2 texture_coords;

out vec4 color_out;

uniform sampler2D texture0;
uniform vec4 color;

void main()
{
    color_out = texture(texture0, texture_coords) * color;
}
)glsl";
