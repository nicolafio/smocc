/*

player.h: Player implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

namespace smocc::player
{

const double PLAYER_SPEED = 0.3;
const int PLAYER_CIRCLE_RADIUS = 3;

void init();
void spawn();
void update();
double getXPosition();
double getYPosition();

} // namespace smocc::player
