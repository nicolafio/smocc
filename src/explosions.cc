/*

explosions.cc: Simple explosion effect for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <SDL.h>

#include "colors.h"
#include "explosions.h"
#include "game.h"
#include "gfx.h"

using namespace std;
using namespace smocc;

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace smocc::explosions
{

const double _EXPLOSION_INITIAL_OPACITY = 0.2;
const unsigned int _EXPLOSION_DURATION_MILLISECONDS = 400;
const double _EXPLOSION_FINAL_RADIUS_PIXELS = 40;

SDL_Color _EXPLOSION_COLOR = SMOCC_FOREGROUND_COLOR;

struct Explosion
{
    unsigned long long id;
    unsigned long long spawnTime;
    double x, y;
};

unordered_map<unsigned long long, Explosion> _explosions;
unordered_set<unsigned long long> _toDespawn;
unsigned long long _nextID;
bool _resetDone;

void _reset();
void _updateExplosion(Explosion& explosion);

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

    for (auto& [id, explosion] : _explosions)
        _updateExplosion(explosion);

    for (auto id : _toDespawn)
        _explosions.erase(id);

    _toDespawn.clear();
}

void spawn(double x, double y)
{
    Explosion explosion;
    explosion.id = _nextID++;
    explosion.spawnTime = game::getTimeElapsedMilliseconds();
    explosion.x = x;
    explosion.y = y;

    _explosions[explosion.id] = explosion;
}

void _reset()
{
    _explosions.clear();
    _toDespawn.clear();
    _nextID = 0;

    _resetDone = true;
}

void _updateExplosion(Explosion& explosion)
{
    unsigned long long currentTime = game::getTimeElapsedMilliseconds();
    unsigned long long elapsed = currentTime - explosion.spawnTime;

    if (elapsed > _EXPLOSION_DURATION_MILLISECONDS)
    {
        _toDespawn.insert(explosion.id);
        return;
    }

    double progress = (double)elapsed / _EXPLOSION_DURATION_MILLISECONDS;
    double opacity = _EXPLOSION_INITIAL_OPACITY * (1 - progress);
    double radius = _EXPLOSION_FINAL_RADIUS_PIXELS * progress;

    Uint8 r = _EXPLOSION_COLOR.r;
    Uint8 g = _EXPLOSION_COLOR.g;
    Uint8 b = _EXPLOSION_COLOR.b;
    Uint8 a = _EXPLOSION_COLOR.a * opacity;

    gfx::setDrawColor(r, g, b, a);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::fillCircle(explosion.x, explosion.y, radius);
}

} // namespace smocc::explosions