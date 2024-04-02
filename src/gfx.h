/*

gfx.h: Graphics utilities for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <filesystem>

namespace smocc::gfx
{

SDL_Cursor* systemCursor(SDL_SystemCursor cursor);

bool pointInRect(int x, int y, SDL_Rect* rect);
bool pointInRect(int x, int y, int rectX, int rectY, int rectW, int rectH);
bool mouseInRect(SDL_Rect* rect);

bool circlesOverlap(double x1, double y1, double r1, double x2, double y2,
                    double r2);

void direction(double originX, double originY, double targetX, double targetY,
               double* directionX, double* directionY);

TTF_Font* font(std::filesystem::path& fontPath, int size);

SDL_Texture* text(TTF_Font* font, char const* text, SDL_Color color);

SDL_Rect textureSize(SDL_Texture* texture);
int textureHeight(SDL_Texture* texture);

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

} // namespace smocc::gfx