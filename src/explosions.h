/*

explosions.h: Simple explosion effect for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

namespace smocc::explosions
{

void init();
void update();
void spawn(double x, double y);

}