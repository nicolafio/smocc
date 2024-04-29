/*

text.h: Text utility for the SMOCC UI

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

#include <SDL.h>

namespace smocc::ui::text
{

const unsigned int REGULAR_FONT_SIZE_PIXELS = 13;

enum FontStyle
{
    REGULAR,
    BOLD
};

const FontStyle ALL_STYLES[] = {REGULAR, BOLD};

void init(int argc, char* argv[]);

// Returns a texture for given text and properties. Saves the texture in a cache
// for future use with the same parameters.
SDL_Texture* get(const char* str, unsigned int size, SDL_Color c, FontStyle st);
SDL_Texture* get(const char* str);
SDL_Texture* get(const char* str, unsigned int size);
SDL_Texture* get(const char* str, SDL_Color color);
SDL_Texture* get(const char* str, FontStyle style);
SDL_Texture* get(unsigned int num);

} // namespace smocc::ui::text