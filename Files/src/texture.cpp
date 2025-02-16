#include <texture.h>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <unordered_map>

static std::unordered_map<std::string, Texture> CACHE;
static Texture WHITE_TEXTURE{};

Texture load_texture(std::string path)
{
    auto it = CACHE.find(path);
    if (it != CACHE.end()) return it->second;
    unsigned int texture = 0;
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (data)
    {
        GLenum format;
        switch (channels)
        {
        case 1: { format = GL_RED; break; }
        case 3: { format = GL_RGB; break; }
        case 4: { format = GL_RGBA; break; }
        default:
            stbi_image_free(data);
            return {};
        }

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        CACHE.insert({ path, { texture, static_cast<unsigned int>(width), static_cast<unsigned int>(height), static_cast<unsigned int>(channels) } });
    }
    stbi_image_free(data);
    return { texture, static_cast<unsigned int>(width), static_cast<unsigned int>(height), static_cast<unsigned int>(channels) };
}

Texture white_texture()
{
    if (WHITE_TEXTURE.id != 0) return WHITE_TEXTURE;
    glGenTextures(1, &WHITE_TEXTURE.id);
    glBindTexture(GL_TEXTURE_2D, WHITE_TEXTURE.id);
    unsigned char data[] = { 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    WHITE_TEXTURE = { WHITE_TEXTURE.id, 1, 1, 3 };
    return WHITE_TEXTURE;
}
