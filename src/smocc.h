/*

smocc.h: Entry point of the game

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#pragma once
#include <SDL.h>

namespace smocc
{
    SDL_Window* getWindow();
    SDL_Renderer* getRenderer();
}
