#include <text.h>

#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>

#include <shader.h>
#include <shader/text.h>
#include <app.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static Shader SHADER;
static unsigned int VAO, VBO;

extern Application* APP;

void init_font_renderer()
{
    ShaderSource shaders[2] = {
        compile_shader(ShaderType::VERTEX_SHADER, TEXT_VERTEX_SHADER),
        compile_shader(ShaderType::FRAGMENT_SHADER, TEXT_FRAGMENT_SHADER)
    };
    SHADER = link_shaders(2, shaders);
    glUseProgram(SHADER);

    resize_font_projection();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void destroy_font_renderer()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(SHADER);
}

void resize_font_projection()
{
    glUseProgram(SHADER);
    glm::mat4 projection = glm::ortho(0.0f, (float)APP->window.size.x, 0.0f, (float)APP->window.size.y);
    glUniformMatrix4fv(glGetUniformLocation(SHADER, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

Font load_font(const char* path, unsigned int height)
{
    Font font;

    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        exit(-1);
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        exit(-1);
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, height);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            font.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return font;
}

void render_text(Font* font, std::string text, glm::vec2 position, float scale, glm::vec3 color, glm::mat4 view)
{
    // activate corresponding render state	
    glUseProgram(SHADER);
    glUniformMatrix4fv(glGetUniformLocation(SHADER, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3f(glGetUniformLocation(SHADER, "text_color"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = (*font)[*c];

        float xpos = position.x + ch.bearing.x * scale;
        float ypos = position.y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.id);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        position.x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Text preview_text(Font* font, const std::string& content, float scale, float max_length, const std::string& truncation)
{
    std::string string;
    if (!truncation.empty()) max_length -= preview_text(font, truncation, scale).size.x;
    float width = 0;
    float y_min = 0;
    glm::vec2 size{};
    for (auto c = content.begin(); c != content.end(); c++)
    {
        Character ch = (*font)[*c];
        if (max_length != 0 && size.x + (ch.advance >> 6) * scale > max_length) return { font, string + truncation, size, scale, y_min };
        size.x += (ch.advance >> 6) * scale;
        size.y = std::max(size.y, ch.size.y * scale);
        y_min = std::max(y_min, (ch.size.y - ch.bearing.y) * scale);
        string += *c;
    }
    return { font, string, size, scale, y_min };
}

void render_text(const Text* text, glm::vec2 position, glm::vec3 color, glm::mat4 view)
{
    // activate corresponding render state	
    glUseProgram(SHADER);
    glUniformMatrix4fv(glGetUniformLocation(SHADER, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3f(glGetUniformLocation(SHADER, "text_color"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text->content.begin(); c != text->content.end(); c++)
    {
        Character ch = (*text->font)[*c];

        float xpos = position.x + ch.bearing.x * text->scale;
        float ypos = position.y - (ch.size.y - ch.bearing.y) * text->scale;

        float w = ch.size.x * text->scale;
        float h = ch.size.y * text->scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.id);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        position.x += (ch.advance >> 6) * text->scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
