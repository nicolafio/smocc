/*

buffs.h: Buffs implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

namespace smocc::buffs
{

static double BUFF_DURATION_MILLISECONDS = 10000;

enum BuffType
{
    FOLLOW_ENEMIES,
    DOUBLE_DAMAGE,
    RAPID_FIRE,
    TRIPLE_FIRE,
    SLOW_ENEMIES,
    BOUNCING_BULLETS,
    DOUBLE_FIRE,
    PUSH_ENEMIES,
};

const BuffType BUFF_TYPES[] = {FOLLOW_ENEMIES, DOUBLE_DAMAGE, RAPID_FIRE,
                               TRIPLE_FIRE,    SLOW_ENEMIES,  BOUNCING_BULLETS,
                               DOUBLE_FIRE,    PUSH_ENEMIES};

const unsigned int BUFF_TYPES_COUNT = sizeof(BUFF_TYPES) / sizeof(BuffType);

void init();
void update();
void rollSpawn(double x, double y, double speedX, double speedY);
bool isActive(BuffType type);
unsigned int getTimeLeftMilliseconds(BuffType type);
char* getTitle(BuffType type);

} // namespace smocc::buffs