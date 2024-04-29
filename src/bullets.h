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

void fire(double x, double y, double xDirection, double yDirection);
void despawn(unsigned long long id);
void forEach(std::function<void(const Bullet& bullet)> callback);

} // namespace smocc::bullets
