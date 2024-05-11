/*

bots.h: Friendly bots implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

#include "player.h"

namespace smocc::bots
{

const double BOT_SPEED = player::PLAYER_SPEED;
const unsigned int BOT_CIRCLE_RADIUS = player::PLAYER_CIRCLE_RADIUS;
const int BOTS_COUNT = 3;

void init();
void update();

bool isActive(unsigned int botIndex);
void deactivate(unsigned int botIndex);
void location(unsigned int botIndex, double* x, double* y);

} // namespace smocc::bots