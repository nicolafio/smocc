/*

gfx.cc: Graphics utilities for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

The functions `fillEllipse` and `fillPolygon` have adapted code originating from
SDL2_gfxPrimitives.c by Andreas Schiffler and Richard Russell, originally named
`aaFilledEllipseRGBA` and `aaFilledPolygonRGBA`, respectively.

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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

#include "gfx.h"
#include "smocc.h"

using namespace std;

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

double inverseLerp(double a, double b, double value)
{
    return (value - a) / (b - a);
}

double distance(double x1, double y1, double x2, double y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;

    return sqrt(dx * dx + dy * dy);
}

double magnitude(double x, double y)
{
    return sqrt(x * x + y * y);
}

bool isUnitVector(double x, double y, double precision)
{
    return abs(magnitude(x, y) - 1.0) < precision;
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

bool rectOnScreen(double x, double y, double w, double h)
{
    SDL_Window* window = smocc::getWindow();
    int ww, wh;

    SDL_GetWindowSize(window, &ww, &wh);

    return rectsOverlap(x, y, w, h, 0, 0, ww, wh);
}

bool mouseInRect(SDL_Rect* rect)
{
    int mouseX, mouseY;

    SDL_GetMouseState(&mouseX, &mouseY);

    return pointInRect(mouseX, mouseY, rect);
}

bool rectsOverlap(
    double x1, double y1, double w1, double h1, double x2, double y2, double w2,
    double h2
)
{
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

bool circlesOverlap(
    double x1, double y1, double r1, double x2, double y2, double r2
)
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

void unit(double x, double y, double* unitX, double* unitY)
{
    double m = magnitude(x, y);

    *unitX = x / m;
    *unitY = y / m;
}

void direction(
    double originX, double originY, double targetX, double targetY,
    double* directionX, double* directionY
)
{
    double dx = targetX - originX;
    double dy = targetY - originY;
    double distance = sqrt(dx * dx + dy * dy);

    *directionX = dx / distance;
    *directionY = dy / distance;
}

void left(double directionX, double directionY, double* leftX, double* leftY)
{
    assert(isUnitVector(directionX, directionY, 0.01));

    *leftX = directionY;
    *leftY = -directionX;
}

void right(double directionX, double directionY, double* rightX, double* rightY)
{
    assert(isUnitVector(directionX, directionY, 0.01));

    *rightX = -directionY;
    *rightY = directionX;
}

void leftward(
    double x, double y, double distance, double directionX, double directionY,
    double* leftwardX, double* leftwardY
)
{
    assert(isUnitVector(directionX, directionY, 0.01));

    double leftX, leftY;

    left(directionX, directionY, &leftX, &leftY);

    *leftwardX = x + leftX * distance;
    *leftwardY = y + leftY * distance;
}

void rightward(
    double x, double y, double distance, double directionX, double directionY,
    double* rightwardX, double* rightwardY
)
{
    assert(isUnitVector(directionX, directionY, 0.01));

    double rightX, rightY;

    right(directionX, directionY, &rightX, &rightY);

    *rightwardX = x + rightX * distance;
    *rightwardY = y + rightY * distance;
}

void rotate(double x, double y, double dx, double dy, double* rx, double* ry)
{
    assert(isUnitVector(dx, dy, 0.01));

    *rx = x * dx - y * dy;
    *ry = x * dy + y * dx;
}

void rotate(double x, double y, double radians, double* rx, double* ry)
{
    rotate(x, y, cos(radians), -sin(radians), rx, ry);
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

unsigned int textureWidth(SDL_Texture* texture)
{
    int width;

    if (SDL_QueryTexture(texture, NULL, NULL, &width, NULL))
    {
        cerr << "Failed to query texture width: " << SDL_GetError() << endl;
        exit(1);
    }

    return width;
}

unsigned int textureHeight(SDL_Texture* texture)
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

/*

The function `fillPolygon` is adapted from `aaFilledPolygonRGBA` by
Richard Russell in SDL2_gfxPrimitives.c. The relevant copyright notice is given
at the top of this file. Please review it before using this code.

The source of Richard Russell's function was found at:
https://github.com/rtrussell/BBCSDL/blob/85f1c5f/src/SDL2_gfxPrimitives.c#L4801-L5023

Paramters:

- vx Vertex array containing X coordinates of the points of the filled polygon.
- vy Vertex array containing Y coordinates of the points of the filled polygon.
- n Number of points in the vertex array. Minimum number is 3.

*/
void fillPolygon(const double* vx, const double* vy, int n)
{
    assert(n >= 3);

    Uint8 r, g, b, a;
    int i, j, xi, yi;
    double x1, x2, y0, y1, y2, minx, maxx, prec;
    float *list, *strip;
    const int POLYSIZE = 16384;

    getDrawColor(&r, &g, &b, &a);

    if (n < 3) return;

    auto compareFloat2 = [](const void* a, const void* b) -> int
    {
        float diff = *((float*)a + 1) - *((float*)b + 1);
        if (diff != 0.0) return (diff > 0) - (diff < 0);
        diff = *(float*)a - *(float*)b;
        return (diff > 0) - (diff < 0);
    };

    // Find extrema:
    minx = 99999.0;
    maxx = -99999.0;
    prec = 0.00001;

    for (i = 0; i < n; i++)
    {
        double x = vx[i];
        double y = fabs(vy[i]);
        if (x < minx) minx = x;
        if (x > maxx) maxx = x;
        if (y > prec) prec = y;
    }

    minx = floor(minx);
    maxx = floor(maxx);
    prec = floor(pow(2, 19) / prec);

    // Allocate main array, this determines the maximum polygon size and
    // complexity:
    list = (float*)malloc(POLYSIZE * sizeof(float));

    assert(list != NULL);

    if (list == NULL) return;

    // Build vertex list.  Special x-values used to indicate vertex type:
    // x = -100001.0 indicates /\, x = -100003.0 indicates \/, x = -100002.0
    // neither
    yi = 0;
    y0 = floor(vy[n - 1] * prec) / prec;
    y1 = floor(vy[0] * prec) / prec;

    for (i = 1; i <= n; i++)
    {
        assert(yi <= POLYSIZE - 4);

        if (yi > POLYSIZE - 4)
        {
            free(list);
            return;
        }

        y2 = floor(vy[i % n] * prec) / prec;

        if (((y1 < y2) - (y1 > y2)) == ((y0 < y1) - (y0 > y1)))
        {
            list[yi++] = -100002.0;
            list[yi++] = y1;
            list[yi++] = -100002.0;
            list[yi++] = y1;
        }
        else
        {
            if (y0 != y1)
            {
                list[yi++] = (y1 < y0) - (y1 > y0) - 100002.0;
                list[yi++] = y1;
            }
            if (y1 != y2)
            {
                list[yi++] = (y1 < y2) - (y1 > y2) - 100002.0;
                list[yi++] = y1;
            }
        }
        y0 = y1;
        y1 = y2;
    }
    xi = yi;

    // Sort vertex list:
    qsort(list, yi / 2, sizeof(float) * 2, compareFloat2);

    // Append line list to vertex list:
    for (i = 1; i <= n; i++)
    {
        double x, y;
        double d = 0.5 / prec;

        x1 = vx[i - 1];
        y1 = floor(vy[i - 1] * prec) / prec;
        x2 = vx[i % n];
        y2 = floor(vy[i % n] * prec) / prec;

        if (y2 < y1)
        {
            double tmp;

            tmp = x1;
            x1 = x2;
            x2 = tmp;

            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        if (y2 != y1) y0 = (x2 - x1) / (y2 - y1);

        for (j = 1; j < xi; j += 4)
        {
            y = list[j];

            if (((y + d) <= y1) || (y == list[j + 4])) continue;
            if ((y -= d) >= y2) break;

            assert(yi <= POLYSIZE - 4);

            if (yi > POLYSIZE - 4)
            {
                free(list);
                return;
            }

            if (y > y1)
            {
                list[yi++] = x1 + y0 * (y - y1);
                list[yi++] = y;
            }

            y += d * 2.0;

            if (y < y2)
            {
                list[yi++] = x1 + y0 * (y - y1);
                list[yi++] = y;
            }
        }

        y = floor(y1) + 1.0;

        while (y <= y2)
        {
            x = x1 + y0 * (y - y1);

            assert(yi <= POLYSIZE - 2);

            if (yi > POLYSIZE - 2)
            {
                free(list);
                return;
            }

            list[yi++] = x;
            list[yi++] = y;

            y += 1.0;
        }
    }

    // Sort combined list:
    qsort(list, yi / 2, sizeof(float) * 2, compareFloat2);

    // Plot lines:
    strip = (float*)malloc((maxx - minx + 2) * sizeof(float));

    assert(strip != NULL);

    if (strip == NULL)
    {
        free(list);
        return;
    }

    memset(strip, 0, (maxx - minx + 2) * sizeof(float));

    n = yi;
    yi = list[1];
    j = 0;

    for (i = 0; i < n - 7; i += 4)
    {
        float x1 = list[i + 0];
        float y1 = list[i + 1];
        float x3 = list[i + 2];
        float x2 = list[i + j + 0];
        float y2 = list[i + j + 1];
        float x4 = list[i + j + 2];

        if (x1 + x3 == -200002.0)
            j += 4;
        else if (x1 + x3 == -200006.0)
            j -= 4;
        else if ((x1 >= minx) && (x2 >= minx))
        {
            if (x1 > x2)
            {
                float tmp = x1;
                x1 = x2;
                x2 = tmp;
            }

            if (x3 > x4)
            {
                float tmp = x3;
                x3 = x4;
                x4 = tmp;
            }

            for (xi = x1 - minx; xi <= x4 - minx; xi++)
            {
                float u, v;
                float x = minx + xi;

                if (x < x2)
                    u = (x - x1 + 1) / (x2 - x1 + 1);
                else
                    u = 1.0;

                if (x >= x3 - 1)
                    v = (x4 - x) / (x4 - x3 + 1);
                else
                    v = 1.0;

                if ((u > 0.0) && (v > 0.0))
                    strip[xi] += (y2 - y1) * (u + v - 1.0);
            }
        }

        if ((yi == (list[i + 5] - 1.0)) || (i == n - 8))
        {
            for (xi = 0; xi <= maxx - minx; xi++)
            {
                if (strip[xi] != 0.0)
                {
                    if (strip[xi] >= 0.996)
                    {
                        int x0 = xi;

                        while (strip[++xi] >= 0.996)
                            ;

                        xi--;

                        setDrawColor(r, g, b, a);
                        drawLine(minx + x0, yi, minx + xi, yi);
                    }
                    else
                    {
                        setDrawColor(r, g, b, a * strip[xi]);
                        fillPixel(minx + xi, yi);
                    }
                }
            }

            memset(strip, 0, (maxx - minx + 2) * sizeof(float));

            yi++;
        }
    }

    setDrawColor(r, g, b, a);

    // Free arrays:
    free(list);
    free(strip);
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

void renderTexture(SDL_Texture* texture, int x, int y)
{
    SDL_Rect rect = textureSize(texture);

    rect.x = x;
    rect.y = y;

    renderTexture(texture, &rect);
}

} // namespace smocc::gfx