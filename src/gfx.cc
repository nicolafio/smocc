/*

gfx.cc: Graphics utilities for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

The function `fillEllipse` contains adapted code originating from
SDL2_gfxPrimitives.c by Andreas Schiffler and Richard Russell, originally named
`aaFilledEllipseRGBA`.

Russel's source was found at
https://github.com/rtrussell/BBCSDL/blob/85f1c5f/src/SDL2_gfxPrimitives.c

The notice for that source is given below.

SDL2_gfxPrimitives.c: graphics primitives for SDL2 renderers

Copyright (C) 2012-2014  Andreas Schiffler
Modifications and additions for BBC BASIC (C) 2016-2020 Richard Russell

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net
Richard Russell -- richard at rtrussell dot co dot uk

*/

#include <iostream>

#include "gfx.h"
#include "smocc.h"

using namespace std;
using namespace smocc;

namespace fs = std::filesystem;

namespace smocc::gfx
{

SDL_Cursor* systemCursor(SDL_SystemCursor cursor)
{
    SDL_Cursor* newCursor = SDL_CreateSystemCursor(cursor);

    if (!newCursor)
    {
        cerr << "Failed to create system cursor: " << SDL_GetError() << endl;
        exit(1);
    }

    return newCursor;
}

bool pointInRect(double x, double y, SDL_Rect* rect)
{
    return pointInRect(x, y, rect->x, rect->y, rect->w, rect->h);
}

bool pointInRect(double x, double y, double rx, double ry, double rw, double rh)
{
    return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

bool pointOnScreen(double x, double y)
{
    SDL_Window* window = smocc::getWindow();
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    return pointInRect(x, y, 0, 0, w, h);
}

bool mouseInRect(SDL_Rect* rect)
{
    int mouseX, mouseY;

    SDL_GetMouseState(&mouseX, &mouseY);

    return pointInRect(mouseX, mouseY, rect);
}

bool circlesOverlap(double x1, double y1, double r1, double x2, double y2,
                    double r2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    double distance = sqrt(dx * dx + dy * dy);

    return distance < r1 + r2;
}

bool pointInCircle(double x, double y, double cx, double cy, double r)
{
    double dx = x - cx;
    double dy = y - cy;
    double distance = sqrt(dx * dx + dy * dy);

    return distance < r;
}

void direction(double originX, double originY, double targetX, double targetY,
               double* directionX, double* directionY)
{
    double dx = targetX - originX;
    double dy = targetY - originY;
    double distance = sqrt(dx * dx + dy * dy);

    *directionX = dx / distance;
    *directionY = dy / distance;
}

TTF_Font* font(fs::path& fontPath, int size)
{
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), size);

    if (!font)
    {
        cerr << "Failed to open font: " << TTF_GetError() << endl;
        exit(1);
    }

    return font;
}

SDL_Texture* text(TTF_Font* font, char const* text, SDL_Color color)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);

    if (!surface)
    {
        cerr << "Failed to render text: " << TTF_GetError() << endl;
        exit(1);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (!texture)
    {
        cerr << "Failed to create text texture: " << SDL_GetError() << endl;
        exit(1);
    }

    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND))
    {
        cerr << "Failed to set texture blend mode: " << SDL_GetError() << endl;
        exit(1);
    }

    SDL_FreeSurface(surface);

    return texture;
}

SDL_Rect textureSize(SDL_Texture* texture)
{
    SDL_Rect size;
    size.x = 0;
    size.y = 0;

    if (SDL_QueryTexture(texture, NULL, NULL, &size.w, &size.h))
    {
        cerr << "Failed to query texture width and height: ";
        cerr << SDL_GetError() << endl;
        exit(1);
    }

    return size;
}

int textureHeight(SDL_Texture* texture)
{
    int height;

    if (SDL_QueryTexture(texture, NULL, NULL, NULL, &height))
    {
        cerr << "Failed to query texture height: " << SDL_GetError() << endl;
        exit(1);
    }

    return height;
}

void setDrawColor(SDL_Color* c)
{
    setDrawColor(c->r, c->g, c->b, c->a);
}

void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_SetRenderDrawColor(renderer, r, g, b, a))
    {
        cerr << "Failed to set render draw color: " << SDL_GetError() << endl;
        exit(1);
    }
}

void getDrawColor(Uint8* r, Uint8* g, Uint8* b, Uint8* a)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_GetRenderDrawColor(renderer, r, g, b, a))
    {
        cerr << "Failed to get render draw color: " << SDL_GetError() << endl;
        exit(1);
    }
}

void setDrawBlendMode(SDL_BlendMode blendMode)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_SetRenderDrawBlendMode(renderer, blendMode))
    {
        cerr << "Failed to set render draw blend mode: " << SDL_GetError()
             << endl;
        exit(1);
    }
}

void fillPixel(int x, int y)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderDrawPoint(renderer, x, y))
    {
        cerr << "Failed to render pixel: " << SDL_GetError() << endl;
        exit(1);
    }
}

void drawLine(int x1, int y1, int x2, int y2)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2))
    {
        cerr << "Failed to render line: " << SDL_GetError() << endl;
        exit(1);
    }
}

void drawRect(SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderDrawRect(renderer, rect))
    {
        cerr << "Failed to render rectangle: " << SDL_GetError() << endl;
        exit(1);
    }
}

void fillRect(SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderFillRect(renderer, rect))
    {
        cerr << "Failed to fill rectangle: " << SDL_GetError() << endl;
        exit(1);
    }
}

/*

The function `fillEllipse` is adapted from `aaFilledEllipseRGBA` by
Richard Russell in SDL2_gfxPrimitives.c. The relevant copyright notice is given
at the top of this file. Please review it before using this code.

The source of Richard Russell's function was found at:
https://github.com/rtrussell/BBCSDL/blob/85f1c5f/src/SDL2_gfxPrimitives.c#L4668

Paramters:

 - cx X coordinate of the center of the filled ellipse.
 - cy Y coordinate of the center of the filled ellipse.
 - rx Horizontal radius in pixels of the filled ellipse.
 - ry Vertical radius in pixels of the filled ellipse.

*/
void fillEllipse(float cx, float cy, float rx, float ry)
{
    Uint8 r, g, b, a;

    getDrawColor(&r, &g, &b, &a);

    int n, xi, yi, result = 0;
    double s, v, x, y, dx, dy;

    if ((rx <= 0.0) || (ry <= 0.0)) return;

    if (rx >= ry)
    {
        n = ry + 1;

        for (yi = cy - n - 1; yi <= cy + n + 1; yi++)
        {
            y = yi;

            if (yi >= cy - 0.5) y += 1;

            s = (y - cy) / ry;
            s = s * s;
            x = 0.5;

            if (s < 1.0)
            {
                x = rx * sqrt(1.0 - s);

                if (x >= 0.5)
                {
                    setDrawColor(r, g, b, a);
                    drawLine(cx - x + 1, yi, cx + x - 1, yi);
                }
            }

            s = 8 * ry * ry;
            dy = fabs(y - cy) - 1.0;
            xi = cx - x; // left

            while (true)
            {
                dx = (cx - xi - 1) * ry / rx;
                v = s - 4 * (dx - dy) * (dx - dy);

                if (v < 0) break;

                v = (sqrt(v) - 2 * (dx + dy)) / 4;

                if (v < 0) break;
                if (v > 1.0) v = 1.0;

                setDrawColor(r, g, b, (double)a * v);
                fillPixel(xi, yi);

                xi -= 1;
            }

            xi = cx + x; // right

            while (true)
            {
                dx = (xi - cx) * ry / rx;
                v = s - 4 * (dx - dy) * (dx - dy);

                if (v < 0) break;

                v = (sqrt(v) - 2 * (dx + dy)) / 4;

                if (v < 0) break;
                if (v > 1.0) v = 1.0;

                setDrawColor(r, g, b, (double)a * v);
                fillPixel(xi, yi);

                xi += 1;
            }
        }
    }

    if (rx < ry)
    {
        n = rx + 1;

        for (xi = cx - n - 1; xi <= cx + n + 1; xi++)
        {
            x = xi;

            if (xi >= (cx - 0.5)) x += 1;

            s = (x - cx) / rx;
            s = s * s;
            y = 0.5;

            if (s < 1.0)
            {
                y = ry * sqrt(1.0 - s);

                if (y >= 0.5)
                {
                    setDrawColor(r, g, b, a);
                    drawLine(xi, cy - y + 1, xi, cy + y - 1);
                }
            }

            s = 8 * rx * rx;
            dx = fabs(x - cx) - 1.0;
            yi = cy - y; // top

            while (true)
            {
                dy = (cy - yi - 1) * rx / ry;
                v = s - 4 * (dy - dx) * (dy - dx);

                if (v < 0) break;

                v = (sqrt(v) - 2 * (dy + dx)) / 4;

                if (v < 0) break;
                if (v > 1.0) v = 1.0;

                setDrawColor(r, g, b, (double)a * v);
                fillPixel(xi, yi);

                yi -= 1;
            }

            yi = cy + y; // bottom

            while (true)
            {
                dy = (yi - cy) * rx / ry;
                v = s - 4 * (dy - dx) * (dy - dx);

                if (v < 0) break;

                v = (sqrt(v) - 2 * (dy + dx)) / 4;

                if (v < 0) break;
                if (v > 1.0) v = 1.0;

                setDrawColor(r, g, b, (double)a * v);
                fillPixel(xi, yi);

                yi += 1;
            }
        }
    }

    setDrawColor(r, g, b, a);
}

void fillCircle(float x, float y, float radius)
{
    fillEllipse(x, y, radius, radius);
}

void renderTexture(SDL_Texture* texture, SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderCopy(renderer, texture, NULL, rect))
    {
        cerr << "Failed to render texture: " << SDL_GetError() << endl;
        exit(1);
    }
}

} // namespace smocc::gfx