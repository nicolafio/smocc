/*

enemies.h: Enemies implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#pragma once

#include <functional>

namespace smocc::enemies
{

const double MIN_ENEMY_HEALTH = 1.0;
const double MAX_ENEMY_HEALTH = 30.0;
const double MIN_ENEMY_SPEED = 0.2;
const double MAX_ENEMY_SPEED = 0.5;

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
