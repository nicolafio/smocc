/*

explosions.h: Simple explosion effect for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#pragma once

namespace smocc::explosions
{

void init();
void update();
void spawn(double x, double y);

}