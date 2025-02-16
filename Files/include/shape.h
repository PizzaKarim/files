#pragma once

#include <glm/glm.hpp>

void init_quad();
void destroy_quad();

void resize_quad(const glm::vec2& size);
void render_quad(const glm::mat4 vm, const unsigned int texture, const glm::vec4 color);
