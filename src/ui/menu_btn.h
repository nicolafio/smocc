/*

ui/menu_btn.h: Menu button UI component for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <SDL.h>

#pragma once

namespace smocc::ui::menu_btn
{

const unsigned int WIDTH_PIXELS = 400;
const unsigned int PADDING_PIXELS = 15;
const unsigned int MARGIN_PIXELS = 6;
const unsigned int BORDER_OPACITY = 25;

SDL_Rect rect(SDL_Texture* text, int yPosition);
void draw(SDL_Rect* rect, SDL_Texture* text, SDL_Texture* textHover);

} // namespace smocc::ui::menu_btn
