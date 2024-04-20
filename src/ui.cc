/*

ui.cc: User interface implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include "ui.h"
#include "gfx.h"
#include "smocc.h"

#include "ui/buffs.h"
#include "ui/game_over.h"
#include "ui/main_menu.h"
#include "ui/menu_btn.h"
#include "ui/score_record.h"
#include "ui/text.h"

namespace smocc::ui
{

const int _UI_WIDTH_PERCENTAGE = 90;
const int _UI_HEIGHT_PERCENTAGE = 90;

SDL_Cursor* _arrowCursor;
SDL_Cursor* _handCursor;

SDL_Rect _uiRect;

PointerStyle _pointerStyle;

void init(int argc, char* argv[])
{
    _arrowCursor = gfx::systemCursor(SDL_SYSTEM_CURSOR_ARROW);
    _handCursor = gfx::systemCursor(SDL_SYSTEM_CURSOR_HAND);
    _pointerStyle = PointerStyle::ARROW;

    ui::text::init(argc, argv);
    ui::main_menu::init();
    ui::game_over::init();
    ui::score_record::init();
    ui::buffs::init();
}

void update()
{
    SDL_Window* window = smocc::getWindow();

    SDL_Rect windowRect;
    windowRect.x = 0;
    windowRect.y = 0;

    SDL_GetWindowSize(window, &windowRect.w, &windowRect.h);

    _uiRect.w = windowRect.w * _UI_WIDTH_PERCENTAGE / 100;
    _uiRect.h = windowRect.h * _UI_HEIGHT_PERCENTAGE / 100;
    _uiRect.x = (windowRect.w - _uiRect.w) / 2;
    _uiRect.y = (windowRect.h - _uiRect.h) / 2;

    smocc::ui::main_menu::update();
    smocc::ui::game_over::update();
    smocc::ui::score_record::update();
    smocc::ui::buffs::update();

    if (_pointerStyle == ARROW) SDL_SetCursor(_arrowCursor);
    if (_pointerStyle == HAND) SDL_SetCursor(_handCursor);
}

void setPointerStyle(PointerStyle style)
{
    _pointerStyle = style;
}

SDL_Rect rect()
{
    return _uiRect;
}

} // namespace smocc::ui
