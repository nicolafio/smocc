/*

bullets.cc: Bullets implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <SDL.h>

#include "bullets.h"
#include "colors.h"
#include "game.h"
#include "gfx.h"
#include "smocc.h"

using namespace std;
using namespace smocc;

namespace smocc::bullets
{

const double _BULLET_SPEED = 0.5;
const int _BULLET_LENGTH = 6;

SDL_Color _BULLET_COLOR = SMOCC_FOREGROUND_COLOR;

unordered_map<unsigned long long, Bullet> _pool;
unordered_set<unsigned long long> _toDespawn;

unsigned long long _nextID;
bool _resetDone;

void _reset();
void _updateBullet(Bullet& bullet);

void init()
{
    _reset();
}

void update()
{
    if (!game::isRunning())
    {
        if (!_resetDone) _reset();
        return;
    }

    _resetDone = false;

    for (auto& [id, bullet] : _pool)
    {
        _updateBullet(bullet);
    }

    for (unsigned long long id : _toDespawn)
    {
        _pool.erase(id);
    }

    _toDespawn.clear();

    gfx::setDrawColor(&_BULLET_COLOR);
    for (auto& [id, bullet] : _pool)
    {
        gfx::drawLine(bullet.xBase, bullet.yBase, bullet.xTip, bullet.yTip);
    }
}

void spawn(double x, double y, double xDirection, double yDirection)
{
    // [xDirection, yDirection] must be a unit vector.
    assert(abs(xDirection * xDirection + yDirection * yDirection - 1.0) < 0.01);

    Bullet bullet;
    bullet.id = _nextID++;
    bullet.xBase = x;
    bullet.yBase = y;
    bullet.xTip = x + xDirection * _BULLET_LENGTH;
    bullet.yTip = y + yDirection * _BULLET_LENGTH;
    bullet.xDirection = xDirection;
    bullet.yDirection = yDirection;
    bullet.xSpeed = xDirection * _BULLET_SPEED;
    bullet.ySpeed = yDirection * _BULLET_SPEED;
    bullet.despawning = false;

    _pool[bullet.id] = bullet;
}

void despawn(unsigned long long id)
{
    _pool[id].despawning = true;
    _toDespawn.insert(id);
}

void forEach(std::function<void(const Bullet& bullet)> callback)
{
    for (auto& [_, bullet] : _pool)
    {
        callback(bullet);
    }
}

void _reset()
{
    _pool.clear();
    _toDespawn.clear();
    _nextID = 0;

    _resetDone = true;
}

void _updateBullet(Bullet& bullet)
{
    double deltaTime = game::getDeltaTimeMilliseconds();

    double xChange = bullet.xSpeed * deltaTime;
    double yChange = bullet.ySpeed * deltaTime;

    bullet.xBase += xChange;
    bullet.yBase += yChange;
    bullet.xTip += xChange;
    bullet.yTip += yChange;

    bool shouldDespawn = !gfx::pointOnScreen(bullet.xBase, bullet.yBase);

    if (shouldDespawn)
    {
        _toDespawn.insert(bullet.id);
    }
}

} // namespace smocc::bullets
