#pragma once

#include <glm/glm.hpp>

void render_explorer(glm::vec2 position, glm::vec2 offset_size);

void explorer_cursor(float x, float y);
void explorer_enter(int entered);
void explorer_scroll(float dx, float dy);
void explorer_resize(float width, float height);
void explorer_click(int button, int action, int mods);
void explorer_key(int key, int scancode, int action, int mods);
