/*

bullets.cc: Bullets implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <unordered_set>

#include <SDL.h>

#include "buffs.h"
#include "bullets.h"
#include "colors.h"
#include "enemies.h"
#include "game.h"
#include "gfx.h"
#include "smocc.h"

using namespace std;
using namespace smocc;
using enum buffs::BuffType;

namespace smocc::bullets
{

const double _BULLET_SPEED = 0.5;
const double _BULLET_OPACITY = 0.7;
const int _BULLET_LENGTH = 6;
const int _DOUBLE_FIRE_BUFF_BULLETS_SPACING = 6;
const double _TRIPLE_FIRE_BUFF_BULLETS_ROTATION_RADIANS = M_PI / 8;
const double _FOLLOW_ENEMIES_BUFF_ROTATION_RADIANS_PER_MILLISECOND = 0.005;

SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;
SDL_Color _DOUBLE_DAMAGE_BULLET_COLOR = SMOCC_FOREGROUND_COLOR;

const Uint8 _BULLET_ALPHA = (Uint8)(round(SDL_ALPHA_OPAQUE * _BULLET_OPACITY));
SDL_Color _BULLET_COLOR = {
    _FG_COLOR.r, _FG_COLOR.g, _FG_COLOR.b, _BULLET_ALPHA
};

unordered_map<unsigned long long, Bullet> _bullets;
unordered_set<unsigned long long> _toDespawn;

unsigned long long _nextID;
bool _resetDone;

double _tripleFireLeftBulletDirectionX;
double _tripleFireLeftBulletDirectionY;
double _tripleFireRightBulletDirectionX;
double _tripleFireRightBulletDirectionY;

void _spawn(double, double, double, double);
void _reset();
void _updateBullet(Bullet& bullet);
const enemies::Enemy* _findClosestEnemy(double, double);

void init()
{
    gfx::rotate(
        1, 0, _TRIPLE_FIRE_BUFF_BULLETS_ROTATION_RADIANS,
        &_tripleFireLeftBulletDirectionX, &_tripleFireLeftBulletDirectionY
    );

    gfx::rotate(
        1, 0, -_TRIPLE_FIRE_BUFF_BULLETS_ROTATION_RADIANS,
        &_tripleFireRightBulletDirectionX, &_tripleFireRightBulletDirectionY
    );

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

    for (auto& [id, bullet] : _bullets)
        _updateBullet(bullet);

    for (unsigned long long id : _toDespawn)
        _bullets.erase(id);

    _toDespawn.clear();

    bool doubleDamage = buffs::isActive(DOUBLE_DAMAGE);
    SDL_Color* c = doubleDamage ? &_DOUBLE_DAMAGE_BULLET_COLOR : &_BULLET_COLOR;

    gfx::setDrawColor(c);

    for (auto& [id, bullet] : _bullets)
        gfx::drawLine(bullet.xBase, bullet.yBase, bullet.xTip, bullet.yTip);
}

void fire(double x, double y, double xDirection, double yDirection)
{
    assert(gfx::isUnitVector(xDirection, yDirection, 0.01));

    vector<pair<double, double>> positions;
    vector<pair<double, double>> directions;

    positions.push_back({x, y});
    directions.push_back({xDirection, yDirection});

    if (buffs::isActive(TRIPLE_FIRE))
    {
        double lx, ly, rx, ry;
        double ldx = _tripleFireLeftBulletDirectionX;
        double ldy = _tripleFireLeftBulletDirectionY;
        double rdx = _tripleFireRightBulletDirectionX;
        double rdy = _tripleFireRightBulletDirectionY;

        gfx::rotate(xDirection, yDirection, ldx, ldy, &lx, &ly);
        gfx::rotate(xDirection, yDirection, rdx, rdy, &rx, &ry);

        positions.push_back({x, y});
        positions.push_back({x, y});

        directions.push_back({lx, ly});
        directions.push_back({rx, ry});
    }

    if (buffs::isActive(DOUBLE_FIRE))
    {
        int n = positions.size();

        vector<pair<double, double>> p(positions);
        vector<pair<double, double>> d(directions);

        positions.clear();
        directions.clear();

        for (int i = 0; i < n; i++)
        {
            double x = p[i].first;
            double y = p[i].second;
            double dx = d[i].first;
            double dy = d[i].second;
            double lx, ly, rx, ry;
            double s = _DOUBLE_FIRE_BUFF_BULLETS_SPACING / 2;

            gfx::leftward(x, y, s, dx, dy, &lx, &ly);
            gfx::rightward(x, y, s, dx, dy, &rx, &ry);

            positions.push_back({lx, ly});
            positions.push_back({rx, ry});

            directions.push_back({dx, dy});
            directions.push_back({dx, dy});
        }
    }

    int n = positions.size();

    for (int i = 0; i < n; i++)
    {
        pair<double, double> pos = positions[i];
        pair<double, double> dir = directions[i];
        _spawn(pos.first, pos.second, dir.first, dir.second);
    }
}

void _spawn(double x, double y, double xDirection, double yDirection)
{
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

    _bullets[bullet.id] = bullet;
}

void despawn(unsigned long long id)
{
    _bullets[id].despawning = true;
    _toDespawn.insert(id);
}

void forEach(std::function<void(const Bullet& bullet)> callback)
{
    for (auto& [_, bullet] : _bullets)
        callback(bullet);
}

void _reset()
{
    _bullets.clear();
    _toDespawn.clear();
    _nextID = 0;

    _resetDone = true;
}

void _updateBullet(Bullet& bullet)
{
    double deltaTime = game::getDeltaTimeMilliseconds();

    double xChange = bullet.xSpeed * deltaTime;
    double yChange = bullet.ySpeed * deltaTime;

    const enemies::Enemy* enemyToFollow = nullptr;
    bool shouldRotateToEnemy = false;
    double ex, ey;   // position of enemy to follow
    double exd, eyd; // direction from bullet to enemy to follow
    double xd = bullet.xDirection;
    double yd = bullet.yDirection;

    bullet.xBase += xChange;
    bullet.yBase += yChange;
    bullet.xTip += xChange;
    bullet.yTip += yChange;

    if (buffs::isActive(FOLLOW_ENEMIES))
        enemyToFollow = _findClosestEnemy(bullet.xTip, bullet.yTip);

    if (enemyToFollow != nullptr)
    {
        ex = enemyToFollow->x;
        ey = enemyToFollow->y;

        gfx::direction(bullet.xBase, bullet.yBase, ex, ey, &exd, &eyd);

        double difference = abs(xd - exd) + abs(yd - eyd);

        shouldRotateToEnemy = difference > 0.001;
    }

    if (shouldRotateToEnemy)
    {
        // 🪄 magic (https://stackoverflow.com/a/3461533)
        bool enemyIsOverLeft = xd * eyd - yd * exd > 0;

        double rotation = enemyIsOverLeft ? -1 : 1;
        rotation *= _FOLLOW_ENEMIES_BUFF_ROTATION_RADIANS_PER_MILLISECOND;
        rotation *= deltaTime;

        gfx::rotate(xd, yd, rotation, &xd, &yd);

        bool enemyWasOverLeft = enemyIsOverLeft;
        enemyIsOverLeft = xd * eyd - yd * exd > 0;

        bool sideFlipped = enemyWasOverLeft != enemyIsOverLeft;

        if (sideFlipped)
        {
            xd = exd;
            yd = eyd;
        }

        bullet.xDirection = xd;
        bullet.yDirection = yd;
        bullet.xSpeed = xd * _BULLET_SPEED;
        bullet.ySpeed = yd * _BULLET_SPEED;
        bullet.xTip = bullet.xBase + xd * _BULLET_LENGTH;
        bullet.yTip = bullet.yBase + yd * _BULLET_LENGTH;
    }

    bool bouncingBuffActive = buffs::isActive(BOUNCING_BULLETS);
    bool bouncingHorizontally = false;
    bool bouncingVertically = false;

    if (buffs::isActive(BOUNCING_BULLETS))
    {
        SDL_Window* win = smocc::getWindow();
        int w, h;
        SDL_GetWindowSize(win, &w, &h);

        bouncingHorizontally = bullet.xTip < 0 || bullet.xTip > w;
        bouncingVertically = bullet.yTip < 0 || bullet.yTip > h;
    }

    if (bouncingHorizontally)
    {
        bullet.xDirection = -bullet.xDirection;
        bullet.xSpeed = -bullet.xSpeed;
        bullet.xTip = bullet.xBase + bullet.xDirection * _BULLET_LENGTH;
    }

    if (bouncingVertically)
    {
        bullet.yDirection = -bullet.yDirection;
        bullet.ySpeed = -bullet.ySpeed;
        bullet.yTip = bullet.yBase + bullet.yDirection * _BULLET_LENGTH;
    }

    bool shouldDespawn =
        !bouncingBuffActive && !gfx::pointOnScreen(bullet.xBase, bullet.yBase);

    if (shouldDespawn) _toDespawn.insert(bullet.id);
}

// Returns nullptr if no enemies are present.
const enemies::Enemy* _findClosestEnemy(double x, double y)
{
    const enemies::Enemy* closestEnemy = nullptr;
    double closestDistance = numeric_limits<double>::max();

    enemies::forEach(
        [&](const enemies::Enemy& enemy)
        {
            double dx = x - enemy.x;
            double dy = y - enemy.y;
            double distance = dx * dx + dy * dy;

            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestEnemy = &enemy;
            }
        }
    );

    return closestEnemy;
}

} // namespace smocc::bullets
