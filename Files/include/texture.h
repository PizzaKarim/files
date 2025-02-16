#pragma once

#include <string>

struct Texture
{
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int channels;
};

Texture load_texture(std::string path);
Texture white_texture();
