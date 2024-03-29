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
    bool isPointInRect(int x, int y, int rectX, int rectY, int rectW, int rectH);
    bool isMouseInRect(SDL_Rect* rect);

    bool circlesOverlap(
        double x1, double y1, double r1,
        double x2, double y2, double r2
    );

    void getDirection(
        double originX, double originY,
        double targetX, double targetY,
        double* directionX, double* directionY
    );

    TTF_Font* openFont(std::filesystem::path& fontPath, int size);

    SDL_Texture* createText(TTF_Font* font, char const* text, SDL_Color color);

    SDL_Rect getTextureSize(SDL_Texture* texture);
    int getTextureHeight(SDL_Texture* texture);

    void setDrawColor(SDL_Color* color);
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void getDrawColor(Uint8* r, Uint8* g, Uint8* b, Uint8* a);

    void setDrawBlendMode(SDL_BlendMode blendMode);

    void fillPixel(int x, int y);

    void drawLine(int x1, int y1, int x2, int y2);

    void drawRect(SDL_Rect* rect);
    void fillRect(SDL_Rect* rect);

    void fillEllipse(float cx, float cy, float rx, float ry);

    void fillCircle(float x, float y, float radius);

    void renderTexture(SDL_Texture* texture, SDL_Rect* rect);
}