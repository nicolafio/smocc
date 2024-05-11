/*

bullets.h: Bullets implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

#include <functional>

namespace smocc::bullets
{

const int BULLET_DAMAGE = 1;

struct Bullet
{
    unsigned long long id;

    double xBase;
    double yBase;
    double xTip;
    double yTip;
    double xDirection;
    double yDirection;
    double xSpeed;
    double ySpeed;

    bool despawning;
};

void init();
void update();

unsigned long long createSource();
void setSourcePosition(unsigned long long sourceID, double x, double y);
void setSourceDirection(unsigned long long sourceID, double dx, double dy);
void deleteSource(unsigned long long sourceID);

void despawn(unsigned long long id);
void forEach(std::function<void(const Bullet& bullet)> callback);

} // namespace smocc::bullets
