/*

gfx.h: Graphics utilities for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#pragma once

#include <filesystem>
#include <SDL.h>
#include <SDL_ttf.h>

namespace smocc::gfx
{
    SDL_Cursor* createSystemCursor(SDL_SystemCursor cursor);

    bool isPointInRect(int x, int y, SDL_Rect* rect);
    bool isMouseInRect(SDL_Rect* rect);

    TTF_Font* openFont(std::filesystem::path& fontPath, int size);

    SDL_Texture* createText(TTF_Font* font, char const* text, SDL_Color color);

    SDL_Rect getTextureSize(SDL_Texture* texture);
    int getTextureHeight(SDL_Texture* texture);

    void setDrawColor(SDL_Color* color);
    void setDrawBlendMode(SDL_BlendMode blendMode);

    void drawRect(SDL_Rect* rect);
    void fillRect(SDL_Rect* rect);

    void renderTexture(SDL_Texture* texture, SDL_Rect* rect);
}