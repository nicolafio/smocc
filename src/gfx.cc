/*

gfx.cc: Graphics utilities for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <iostream>

#include "smocc.h"
#include "gfx.h"

using namespace std;
using namespace smocc;

namespace fs = std::filesystem;

SDL_Cursor* gfx::createSystemCursor(SDL_SystemCursor cursor)
{
    SDL_Cursor* newCursor = SDL_CreateSystemCursor(cursor);

    if (!newCursor)
    {
        cerr << "Failed to create system cursor: " << SDL_GetError() << endl;
        exit(1);
    }

    return newCursor;
}

bool gfx::isPointInRect(int x, int y, SDL_Rect* rect)
{
    return x >= rect->x && x <= rect->x + rect->w &&
           y >= rect->y && y <= rect->y + rect->h;
}

bool gfx::isMouseInRect(SDL_Rect* rect)
{
    int mouseX, mouseY;

    SDL_GetMouseState(&mouseX, &mouseY);

    return isPointInRect(mouseX, mouseY, rect);
}

TTF_Font* gfx::openFont(fs::path& fontPath, int size)
{
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), size);

    if (!font)
    {
        cerr << "Failed to open font: " << TTF_GetError() << endl;
        exit(1);
    }

    return font;
}

SDL_Texture* gfx::createText(TTF_Font* font, char const* text, SDL_Color color)
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

SDL_Rect gfx::getTextureSize(SDL_Texture* texture)
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

int gfx::getTextureHeight(SDL_Texture* texture)
{
    int height;

    if (SDL_QueryTexture(texture, NULL, NULL, NULL, &height))
    {
        cerr << "Failed to query texture height: " << SDL_GetError() << endl;
        exit(1);
    }

    return height;
}

void gfx::setDrawColor(SDL_Color* color)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    int r = color->r;
    int g = color->g;
    int b = color->b;
    int a = color->a;

    if (SDL_SetRenderDrawColor(renderer, r, g, b, a))
    {
        cerr << "Failed to set render draw color: " << SDL_GetError() << endl;
        exit(1);
    }
}

void gfx::setDrawBlendMode(SDL_BlendMode blendMode)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_SetRenderDrawBlendMode(renderer, blendMode))
    {
        cerr << "Failed to set render draw blend mode: " << SDL_GetError() << endl;
        exit(1);
    }
}

void gfx::drawRect(SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderDrawRect(renderer, rect))
    {
        cerr << "Failed to render rectangle: " << SDL_GetError() << endl;
        exit(1);
    }
}

void gfx::fillRect(SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderFillRect(renderer, rect))
    {
        cerr << "Failed to fill rectangle: " << SDL_GetError() << endl;
        exit(1);
    }
}

void gfx::renderTexture(SDL_Texture* texture, SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderCopy(renderer, texture, NULL, rect))
    {
        cerr << "Failed to render texture: " << SDL_GetError() << endl;
        exit(1);
    }
}