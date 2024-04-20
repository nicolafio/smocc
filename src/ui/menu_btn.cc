/*

ui/menu_btn.cc: Menu button UI component for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include "menu_btn.h"
#include "../colors.h"
#include "../gfx.h"
#include "../smocc.h"
#include "../ui.h"

using namespace smocc;

namespace smocc::ui::menu_btn
{

SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;
SDL_Color _BORDER_COLOR = {_FG_COLOR.r, _FG_COLOR.g, _FG_COLOR.b,
                           BORDER_OPACITY};

SDL_Rect rect(SDL_Texture* text, int yPosition)
{
    SDL_Rect uiRect = ui::rect();

    SDL_Rect rect;
    rect.w = WIDTH_PIXELS;
    rect.h = gfx::textureHeight(text) + 2 * PADDING_PIXELS;
    rect.x = uiRect.x + (uiRect.w - rect.w) / 2;
    rect.y = yPosition;

    return rect;
}

void draw(SDL_Rect* rect, SDL_Texture* text, SDL_Texture* textHover)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    bool hover = gfx::mouseInRect(rect);

    SDL_Rect textRect = gfx::textureSize(text);

    textRect.x = rect->x + (rect->w - textRect.w) / 2;
    textRect.y = rect->y + PADDING_PIXELS;

    gfx::setDrawColor(&_BORDER_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::drawRect(rect);

    if (!hover)
    {
        gfx::renderTexture(text, &textRect);
    }

    if (hover)
    {
        gfx::setDrawColor(&_FG_COLOR);
        gfx::fillRect(rect);
        gfx::renderTexture(textHover, &textRect);
    }
}

} // namespace smocc::ui::menu_btn
