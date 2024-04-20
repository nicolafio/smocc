/*

ui/buffs.cc: Buffs UI component for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <vector>

#include <SDL.h>

#include "../buffs.h"
#include "../colors.h"
#include "../game.h"
#include "../gfx.h"
#include "text.h"
#include "../ui.h"

using namespace std;
using namespace smocc::ui;

namespace smocc::ui::buffs
{

const int _TIME_BAR_THICKNESS_PIXELS = 3;
const int _TIME_BAR_WIDTH_PIXELS_PER_SECOND = 30;

SDL_Texture* _buffText[smocc::buffs::BUFF_TYPES_COUNT];
SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;
int _buffTextWidth[smocc::buffs::BUFF_TYPES_COUNT];
int _buffTextHeight[smocc::buffs::BUFF_TYPES_COUNT];

SDL_Color _TIME_BAR_COLOR = {_FG_COLOR.r, _FG_COLOR.g, _FG_COLOR.b, 85};

void init()
{
    for (smocc::buffs::BuffType type : smocc::buffs::BUFF_TYPES)
    {
        char* title = smocc::buffs::getTitle(type);
        _buffText[type] = text::get(title);
        _buffTextWidth[type] = gfx::textureWidth(_buffText[type]);
        _buffTextHeight[type] = gfx::textureHeight(_buffText[type]);
    }
}

void update()
{
    if (!game::isRunning()) return;

    vector<smocc::buffs::BuffType> active;

    unsigned int totalHeight = 0;

    for (smocc::buffs::BuffType type : smocc::buffs::BUFF_TYPES)
        if (smocc::buffs::isActive(type))
        {
            active.push_back(type);
            totalHeight += _buffTextHeight[type];
            totalHeight += _TIME_BAR_THICKNESS_PIXELS;
        }

    int count = active.size();
    SDL_Rect textRect[count];
    SDL_Rect barRect[count];
    SDL_Rect uiRect = smocc::ui::rect();

    for (int i = 0; i < count; i++)
    {
        textRect[i].w = _buffTextWidth[active[i]];
        textRect[i].h = _buffTextHeight[active[i]];
        textRect[i].x = uiRect.x;
    }

    textRect[0].y = uiRect.y + uiRect.h - totalHeight;

    for (int i = 1; i < count; i++)
    {
        int prevY = textRect[i - 1].y;
        int prevH = textRect[i - 1].h;
        textRect[i].y = prevY + prevH + _TIME_BAR_THICKNESS_PIXELS;
    }

    for (int i = 0; i < count; i++)
    {
        unsigned int millis = smocc::buffs::getTimeLeftMilliseconds(active[i]);
        double secs = millis / 1000.0;

        barRect[i].w = secs * _TIME_BAR_WIDTH_PIXELS_PER_SECOND;
        barRect[i].h = _TIME_BAR_THICKNESS_PIXELS;
        barRect[i].x = uiRect.x;
        barRect[i].y = textRect[i].y + textRect[i].h;
    }

    gfx::setDrawColor(&_TIME_BAR_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);

    for (int i = 0; i < count; i++)
    {
        gfx::renderTexture(_buffText[active[i]], &textRect[i]);
        gfx::fillRect(&barRect[i]);
    }
}

} // namespace smocc::ui::buffs
