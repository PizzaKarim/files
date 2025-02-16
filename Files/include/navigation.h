#pragma once

#include <glm/glm.hpp>

#include <filesystem>
#include <string>

struct File
{
    std::string path;
    std::string name;
    unsigned int thumbnail;
};

bool navigate_directory(std::string directory);
bool navigate_back();
bool navigate_forward();
bool navigate_up();
