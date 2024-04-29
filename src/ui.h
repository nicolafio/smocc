/*

ui.h: User interface implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

#include <SDL.h>

namespace smocc::ui
{

enum PointerStyle
{
    ARROW,
    HAND
};

void init();
void update();
void setPointerStyle(PointerStyle);
SDL_Rect rect();

} // namespace smocc::ui
