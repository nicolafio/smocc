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
double inverseLerp(double a, double b, double value);
double distance(double x1, double y1, double x2, double y2);
double magnitude(double x1, double x2);
bool isUnitVector(double x, double y, double precision);
bool pointInRect(double x, double y, SDL_Rect* rect);
bool pointInRect(
    double x, double y, double rectX, double rectY, double rectW, double rectH
);
bool pointOnScreen(double x, double y);
bool rectOnScreen(double x, double y, double w, double h);
bool mouseInRect(SDL_Rect* rect);
bool rectsOverlap(
    double x1, double y1, double w1, double h1, double x2, double y2, double w2,
    double h2
);
bool circlesOverlap(
    double x1, double y1, double r1, double x2, double y2, double r2
);
bool pointInCircle(double x, double y, double cx, double cy, double r);
void unit(double x, double y, double* unitX, double* unitY);
void direction(
    double originX, double originY, double targetX, double targetY,
    double* directionX, double* directionY
);
void left(double directionX, double directionY, double* leftX, double* leftY);
void right(
    double directionX, double directionY, double* rightX, double* rightY
);
void leftward(
    double x, double y, double distance, double directionX, double directionY,
    double* leftwardX, double* leftwardY
);
void rightward(
    double x, double y, double distance, double directionX, double directionY,
    double* rightwardX, double* rightwardY
);
void rotate(
    double x, double y, double directionX, double directionY, double* rotatedX,
    double* rotatedY
);
void rotate(
    double x, double y, double radians, double* rotatedX, double* rotatedY
);
TTF_Font* font(std::filesystem::path& fontPath, int size);
SDL_Texture* text(TTF_Font* font, char const* text, SDL_Color color);
SDL_Rect textureSize(SDL_Texture* texture);
unsigned int textureWidth(SDL_Texture* texture);
unsigned int textureHeight(SDL_Texture* texture);
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
void fillPolygon(const double* vx, const double* vy, int n);
void renderTexture(SDL_Texture* texture, SDL_Rect* rect);
void renderTexture(SDL_Texture* texture, int x, int y);

} // namespace smocc::gfx