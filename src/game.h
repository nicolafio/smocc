/*

game.h: Game session implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#pragma once

namespace smocc::game
{

void init();
void begin();
void update();
void end();
bool isRunning();
unsigned int getScore();
void incrementScore();
unsigned int getRecord();
double getDifficulty();
unsigned long long getTimeElapsedMilliseconds();
unsigned int getDeltaTimeMilliseconds();

} // namespace smocc::game
