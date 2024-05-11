/*

smocc.h: Entry point of the game

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once
#include <SDL.h>

namespace smocc
{

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 720;

int main(int, char**);
SDL_Window* getWindow();
SDL_Renderer* getRenderer();

} // namespace smocc
