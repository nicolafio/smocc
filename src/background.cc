/*

background.cc: Background drawing for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#include <cassert>

#include <SDL.h>
#include <SDL_image.h>

#include "background.h"
#include "smocc.h"

namespace smocc::background
{

SDL_Texture* _background;

void init()
{
    SDL_Renderer* renderer = smocc::getRenderer();

    _background = IMG_LoadTexture(renderer, "background.png");

    assert(_background != NULL);
}

void update()
{
    SDL_Renderer* renderer = smocc::getRenderer();
    SDL_Rect windowRect;

    windowRect.x = 0;
    windowRect.y = 0;

    SDL_GetWindowSize(smocc::getWindow(), &windowRect.w, &windowRect.h);

    int res = SDL_RenderCopy(renderer, _background, NULL, &windowRect);

    assert(res == 0);
}

} // namespace smocc::background