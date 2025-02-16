#include <shader.h>

#include <glad/glad.h>

#include <stdio.h>

ShaderSource compile_shader(const ShaderType type, const char* source)
{
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char log[512];
		glGetShaderInfoLog(shader, 512, nullptr, log);
		fprintf(stderr, "Error compiling shader source: %s\n", log);
		return -1;
	}
	return shader;
}

Shader link_shaders(unsigned int argc, ShaderSource* shaders)
{
	unsigned int shader = glCreateProgram();
	for (unsigned int i = 0; i < argc; i++)
	{
		glAttachShader(shader, shaders[i]);
	}
	glLinkProgram(shader);
	int success;
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success)
	{
		char log[512];
		glGetProgramInfoLog(shader, 512, nullptr, log);
		fprintf(stderr, "Error linking shader program: %s\n", log);
		return -1;
	}
	for (unsigned int i = 0; i < argc; i++)
	{
		glDeleteShader(shaders[i]);
	}
	return shader;
}
