#pragma once
#include <SDL.h>

namespace ui
{
    void init(int, char**, SDL_Renderer*);
    void update(SDL_Renderer*, SDL_Window*);
    void cleanup();
}
