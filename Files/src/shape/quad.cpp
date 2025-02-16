#include <shape.h>

#include <shader.h>
#include <shader/quad.h>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <texture.h>

#include <app.h>

extern Application* APP;

static Shader SHADER;
static unsigned int VAO, VBO;
static glm::mat4 PROJECTION;

void init_quad()
{
    ShaderSource shaders[2] = {
        compile_shader(ShaderType::VERTEX_SHADER, QUAD_VERTEX_SHADER),
        compile_shader(ShaderType::FRAGMENT_SHADER, QUAD_FRAGMENT_SHADER)
    };
    SHADER = link_shaders(2, shaders);
    glUseProgram(SHADER);

    float vertices[] = {
        0.0f, 1.0f,   0.0f, 1.0f, // top left
        0.0f, 0.0f,   0.0f, 0.0f, // bottom left
        1.0f, 0.0f,   1.0f, 0.0f, // bottom right

        0.0f, 1.0f,   0.0f, 1.0f, // top left
        1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        1.0f, 1.0f,   1.0f, 1.0f  // top right
    };

    resize_quad(APP->window.size);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void destroy_quad()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(SHADER);
}

void resize_quad(const glm::vec2& size)
{
    PROJECTION = glm::ortho(0.0f, size.x, 0.0f, size.y);
}

void render_quad(const glm::mat4 vm, const unsigned int texture, const glm::vec4 color)
{
    glUseProgram(SHADER);
    glUniformMatrix4fv(glGetUniformLocation(SHADER, "vm"), 1, false, glm::value_ptr(vm));
    glUniformMatrix4fv(glGetUniformLocation(SHADER, "projection"), 1, false, glm::value_ptr(PROJECTION));
    glUniform4f(glGetUniformLocation(SHADER, "color"), color.x, color.y, color.z, color.w);
    glBindTexture(GL_TEXTURE_2D, texture != 0 ? texture : white_texture().id);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
