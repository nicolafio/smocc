/*

enemies.h: Enemies implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#pragma once

#include <functional>

namespace smocc::enemies
{

struct Enemy
{
    unsigned long long id;
    int health;
    double x;
    double y;
    double radius;
    double speed;
    double initialSpeed;
    double xSpeed;
    double ySpeed;
};

void init();
void update();
void forEach(std::function<void(const Enemy& enemy)> callback);

} // namespace smocc::enemies
