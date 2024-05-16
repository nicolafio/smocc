/*

buffs.cc: Buffs implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <SDL.h>

#include "buffs.h"
#include "colors.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "rng.h"

using namespace std;

namespace smocc::buffs
{

static double _BUFF_DROP_OPACITY = 0.7;
// static double _BUFF_DROP_SPAWN_CHANCE = 0.15;
static double _BUFF_DROP_SPAWN_CHANCE = 1.0; // DEBUG
static double _BUFF_DROP_CHASE_SPEED = 0.2;
static double _BUFF_DROP_MAGNETIC_RADIUS = 100;
static double _BUFF_DROP_ROTATION_ANIMATION_RADIANS_PER_MILLISECOND = 0.01;
static int _BUFF_DROP_SQUARE_SIDES_LENGTH = 10;

static unordered_map<BuffType, string> _BUFF_TITLES = {
    {FOLLOW_ENEMIES, "Follow Enemies"}, {DOUBLE_DAMAGE, "Double Damage"},
    {RAPID_FIRE, "Rapid Fire"},         {TRIPLE_FIRE, "Triple Fire"},
    {SLOW_ENEMIES, "Slow Enemies"},     {BOUNCING_BULLETS, "Bouncing Bullets"},
    {DOUBLE_FIRE, "Double Fire"},       {PUSH_ENEMIES, "Push Enemies"},
    {FRIENDLY_BOTS, "Friendly Bots"}
};

static double _BUFF_DROP_SQUARE_DIAMETER =
    sqrt(pow(_BUFF_DROP_SQUARE_SIDES_LENGTH, 2) * 2);

static unsigned int _BUFF_DROP_BOUNDING_RADIUS =
    ceil(_BUFF_DROP_SQUARE_DIAMETER / 2);

static unsigned int _BUFF_DROP_TRIGGER_RADIUS = _BUFF_DROP_BOUNDING_RADIUS * 2;

static double _BUFF_DROP_ROTATION_ANIMATION_TIME_INTERVAL_MILLISECONDS =
    (2 * M_PI) / _BUFF_DROP_ROTATION_ANIMATION_RADIANS_PER_MILLISECOND;

static Uint8 _BUFF_DROP_ALPHA = (Uint8)(SDL_ALPHA_OPAQUE * _BUFF_DROP_OPACITY);

SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;
SDL_Color _BUFF_COLOR = {
    _FG_COLOR.r, _FG_COLOR.g, _FG_COLOR.b, _BUFF_DROP_ALPHA
};

struct BuffDrop
{
    unsigned long long id;
    unsigned long long spawnTime;
    double x, y;
    double speedX, speedY;
};

unordered_map<BuffType, unsigned int> _timeLeftMilliseconds;
unordered_map<unsigned long long, BuffDrop> _buffDrops;
unordered_set<unsigned long long> _toDespawn;
unsigned long long _nextID;
bool _resetDone;

void _reset();
void _spawnBuff(double x, double y, double speedX, double speedY);
void _updateBuffDrop(BuffDrop& buffDrop);
void _updateBuffDropLinearMovement(BuffDrop& buffDrop);
void _updateBuffDropMagneticEffect(BuffDrop& buffDrop);
void _renderBuffDrop(BuffDrop& buffDrop);
void _rollBuff();

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

    gfx::setDrawColor(&_BUFF_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);

    unsigned int deltaTime = game::getDeltaTimeMilliseconds();

    for (BuffType buff : BUFF_TYPES)
    {
        unsigned int t = _timeLeftMilliseconds[buff];
        _timeLeftMilliseconds[buff] = t > deltaTime ? t - deltaTime : 0;
    }

    for (auto& [_, buff] : _buffDrops)
        _updateBuffDrop(buff);

    for (auto id : _toDespawn)
        _buffDrops.erase(id);
}

void rollSpawn(double x, double y, double speedX, double speedY)
{
    if (rng::roll() < _BUFF_DROP_SPAWN_CHANCE) _spawnBuff(x, y, speedX, speedY);
}

bool isActive(BuffType type)
{
    return _timeLeftMilliseconds[type] > 0;
}

unsigned int getTimeLeftMilliseconds(BuffType type)
{
    return _timeLeftMilliseconds[type];
}

char* getTitle(BuffType type)
{
    return (char*)_BUFF_TITLES[type].c_str();
}

void _reset()
{
    _buffDrops.clear();
    _toDespawn.clear();
    _nextID = 0;

    for (BuffType type : BUFF_TYPES)
        _timeLeftMilliseconds[type] = 0;

    _resetDone = true;
}

void _spawnBuff(double x, double y, double speedX, double speedY)
{
    BuffDrop buffDrop;
    buffDrop.id = _nextID++;
    buffDrop.spawnTime = game::getTimeElapsedMilliseconds();
    buffDrop.x = x;
    buffDrop.y = y;
    buffDrop.speedX = speedX;
    buffDrop.speedY = speedY;

    _buffDrops[buffDrop.id] = buffDrop;
}

void _updateBuffDrop(BuffDrop& buffDrop)
{
    double boundX = buffDrop.x - _BUFF_DROP_BOUNDING_RADIUS;
    double boundY = buffDrop.y - _BUFF_DROP_BOUNDING_RADIUS;
    double boundWH = _BUFF_DROP_BOUNDING_RADIUS * 2;
    double outOfScreen = !gfx::rectOnScreen(boundX, boundY, boundWH, boundWH);

    if (outOfScreen)
    {
        _toDespawn.insert(buffDrop.id);
        return;
    }

    double playerX = player::getXPosition();
    double playerY = player::getYPosition();
    double distance = gfx::distance(buffDrop.x, buffDrop.y, playerX, playerY);

    if (distance < _BUFF_DROP_TRIGGER_RADIUS)
    {
        _rollBuff();
        _toDespawn.insert(buffDrop.id);
        return;
    }

    _updateBuffDropLinearMovement(buffDrop);
    _updateBuffDropMagneticEffect(buffDrop);
    _renderBuffDrop(buffDrop);
}

void _updateBuffDropLinearMovement(BuffDrop& buffDrop)
{
    unsigned int deltaTime = game::getDeltaTimeMilliseconds();

    buffDrop.x += buffDrop.speedX * (double)deltaTime;
    buffDrop.y += buffDrop.speedY * (double)deltaTime;
}

void _updateBuffDropMagneticEffect(BuffDrop& buffDrop)
{
    unsigned int deltaTime = game::getDeltaTimeMilliseconds();

    double playerX = player::getXPosition();
    double playerY = player::getYPosition();
    double distance = gfx::distance(buffDrop.x, buffDrop.y, playerX, playerY);

    if (distance > _BUFF_DROP_MAGNETIC_RADIUS) return;

    double change = _BUFF_DROP_CHASE_SPEED * (double)deltaTime;

    if (change > distance) change = distance;

    double dx, dy;

    gfx::direction(buffDrop.x, buffDrop.y, playerX, playerY, &dx, &dy);

    buffDrop.x += dx * change;
    buffDrop.y += dy * change;
}

void _renderBuffDrop(BuffDrop& buffDrop)
{
    unsigned long long start = buffDrop.spawnTime;
    unsigned long long elapsed = game::getTimeElapsedMilliseconds() - start;

    double animationElapsed = remainder(
        elapsed, _BUFF_DROP_ROTATION_ANIMATION_TIME_INTERVAL_MILLISECONDS
    );

    double animationProgress =
        animationElapsed /
        _BUFF_DROP_ROTATION_ANIMATION_TIME_INTERVAL_MILLISECONDS;

    double squareRotationRadians = M_PI * 2 - (animationProgress * M_PI * 2);
    double squareRotationX = cos(squareRotationRadians);
    double squareRotationY = -sin(squareRotationRadians);

    static pair<double, double> squarePoints[] = {
        {-1, -1}, {1, -1}, {1, 1}, {-1, 1}
    };

    double polygonX[4];
    double polygonY[4];

    for (int i = 0; i < 4; i++)
    {
        double l = (double)_BUFF_DROP_SQUARE_SIDES_LENGTH / 2;
        double sx = squarePoints[i].first * l;
        double sy = squarePoints[i].second * l;
        double rx, ry;

        gfx::rotate(sx, sy, squareRotationX, squareRotationY, &rx, &ry);

        polygonX[i] = buffDrop.x + rx;
        polygonY[i] = buffDrop.y + ry;
    }

    gfx::fillPolygon(polygonX, polygonY, 4);
}

void _rollBuff()
{
    // BuffType type = BUFF_TYPES[rng::rollInt(0, BUFF_TYPES_COUNT - 1)];
    BuffType type = FRIENDLY_BOTS; // DEBUG

    _timeLeftMilliseconds[type] += BUFF_DURATION_MILLISECONDS;

    cout << "Applied buff: " << getTitle(type) << endl;
}

} // namespace smocc::buffs