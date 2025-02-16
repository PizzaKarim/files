#pragma once

typedef unsigned int ShaderSource;
typedef unsigned int Shader;

enum ShaderType : unsigned int
{
	FRAGMENT_SHADER = 0x8B30,
	VERTEX_SHADER = 0x8B31
};

ShaderSource compile_shader(const ShaderType type, const char* source);
Shader link_shaders(unsigned int argc, ShaderSource* shaders);
