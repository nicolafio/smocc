/*

enemies.cc: Enemies implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>

#include <SDL.h>

#include "colors.h"
#include "enemies.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "smocc.h"
#include "ui.h"

using namespace std;
using namespace smocc;

namespace smocc::enemies
{

const int _SPAWN_DELAY_MILLISECONDS = 500;
const double _MIN_ENEMY_HEALTH = 1.0;
const double _MAX_ENEMY_HEALTH = 30.0;
const double _MIN_ENEMY_SPEED = 0.2;
const double _MAX_ENEMY_SPEED = 0.5;
const int _MIN_ENEMY_COUNT = 10;
const int _MAX_ENEMY_COUNT = 20;
const double _MIN_ENEMY_RADIUS = 15.0;
const double _MAX_ENEMY_RADIUS = 75.0;
const double _ENEMY_RADIUS_CHANGE_SPEED = 0.1;

SDL_Color _ENEMY_COLOR = SMOCC_FOREGROUND_COLOR;

int _maxEnemies;
unsigned long long _spawnRollsDone;
bool _resetDone;
int _lastUpdateTimeMilliseconds;

struct Enemy
{
    unsigned long long id;
    int health;
    double x;
    double y;
    double radius;
    double speed;
    double xSpeed;
    double ySpeed;
};

random_device _rd;
mt19937 _gen(_rd());
uniform_real_distribution<double> _rng(0.0, 1.0);
unsigned long long _nextID;

unordered_map<unsigned long long, Enemy> _pool;

void _spawnEnemy()
{
    Enemy enemy;

    enemy.id = _nextID++;
    enemy.radius = 0;

    double edgeRoll = _rng(_gen);

    bool spawningLeft = edgeRoll <= .25;
    bool spawningRight = edgeRoll > .25 && edgeRoll <= .5;
    bool spawningTop = edgeRoll > .5 && edgeRoll <= .75;
    bool spawningBottom = edgeRoll > .75;

    double locationRoll = _rng(_gen);

    SDL_Window* window = smocc::getWindow();
    int windowWidth, windowHeight;

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    if (spawningLeft)
    {
        enemy.x = 0;
        enemy.y = locationRoll * windowHeight;
    }

    if (spawningRight)
    {
        enemy.x = windowWidth;
        enemy.y = locationRoll * windowHeight;
    }

    if (spawningTop)
    {
        enemy.x = locationRoll * windowWidth;
        enemy.y = 0;
    }

    if (spawningBottom)
    {
        enemy.x = locationRoll * windowWidth;
        enemy.y = windowHeight;
    }

    double healthRoll = _rng(_gen);
    double difficulty = game::getDifficulty();
    const double healthRange = _MAX_ENEMY_HEALTH - _MIN_ENEMY_HEALTH;
    int healthAddendum = round(healthRange * healthRoll * difficulty);

    enemy.health = _MIN_ENEMY_HEALTH + healthAddendum;

    double speedRoll = _rng(_gen);
    const double speedRange = _MAX_ENEMY_SPEED - _MIN_ENEMY_SPEED;
    double speed = _MIN_ENEMY_SPEED + speedRange * speedRoll;

    double rotationRoll = _rng(_gen);
    double rotationRadians = M_PI * rotationRoll;

    if (spawningLeft)
    {
        rotationRadians -= M_PI / 2;
    }

    if (spawningRight)
    {
        rotationRadians += M_PI / 2;
    }

    if (spawningTop)
    {
        rotationRadians += M_PI;
    }

    enemy.speed = speed;
    enemy.xSpeed = speed * cos(rotationRadians);
    enemy.ySpeed = -speed * sin(rotationRadians);

    _pool[enemy.id] = enemy;
}

void _rollEnemySpawn()
{
    int enemiesCount = _pool.size();

    if (enemiesCount < _MIN_ENEMY_COUNT)
    {
        _spawnEnemy();
    }

    if (enemiesCount >= _MIN_ENEMY_COUNT && enemiesCount < _maxEnemies)
    {
        double spawnChance = game::getDifficulty();
        double roll = _rng(_gen);

        if (roll < spawnChance) _spawnEnemy();
    }

    _spawnRollsDone++;
}

void _reset()
{
    _pool.clear();
    _maxEnemies = 0;
    _spawnRollsDone = 0;
    _lastUpdateTimeMilliseconds = -1;
    _nextID = 0;

    _resetDone = true;
}

unsigned long long _getSpawnRollsToDo()
{
    unsigned long long millisecondsElapsed = game::getTimeElapsedMilliseconds();
    unsigned long long target = millisecondsElapsed / _SPAWN_DELAY_MILLISECONDS;
    return target - _spawnRollsDone;
}

void _doNecessarySpawnRolls()
{
    unsigned long long rollsToDo = _getSpawnRollsToDo();

    for (unsigned long long i = 0; i < rollsToDo; i++)
        _rollEnemySpawn();
}

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

    double difficulty = game::getDifficulty();

    _maxEnemies = _MAX_ENEMY_COUNT * difficulty;

    _doNecessarySpawnRolls();

    SDL_Window* window = smocc::getWindow();
    int windowWidth, windowHeight;

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    Uint64 now = SDL_GetTicks64();
    Uint64 deltaTimeMilliseconds = 0;

    if (_lastUpdateTimeMilliseconds != -1)
    {
        deltaTimeMilliseconds = now - _lastUpdateTimeMilliseconds;
    }

    _lastUpdateTimeMilliseconds = now;

    vector<Enemy> toRemove;

    for (auto& [_, enemy] : _pool)
    {
        if (enemy.health <= 0) toRemove.push_back(enemy);
    }

    for (Enemy& enemy : toRemove)
    {
        _pool.erase(enemy.id);
    }

    for (auto& [id, _] : _pool)
    {
        Enemy& enemy = _pool[id];

        const double radiusRange = _MAX_ENEMY_RADIUS - _MIN_ENEMY_RADIUS;
        double radiusFactor = enemy.health / _MAX_ENEMY_HEALTH;
        double targetRadius = _MIN_ENEMY_RADIUS + radiusRange * radiusFactor;
        double radiusChange =
            _ENEMY_RADIUS_CHANGE_SPEED * (double)deltaTimeMilliseconds;

        if (enemy.radius < targetRadius)
        {
            enemy.radius = min(enemy.radius + radiusChange, targetRadius);
        }

        if (enemy.radius > targetRadius)
        {
            enemy.radius = max(enemy.radius - radiusChange, targetRadius);
        }

        double x = enemy.x;
        double y = enemy.y;
        double radius = enemy.radius;
        double playerX = player::getXPosition();
        double playerY = player::getYPosition();
        double playerRadius = player::PLAYER_CIRCLE_RADIUS;

        if (gfx::circlesOverlap(x, y, radius, playerX, playerY, playerRadius))
        {
            game::end();
            ui::showGameOver();
            return;
        }

        if (!gfx::pointInRect(x, y, 0, 0, windowWidth, windowHeight))
        {
            double targetX = player::getXPosition();
            double targetY = player::getYPosition();

            double xDirection, yDirection;

            gfx::direction(x, y, targetX, targetY, &xDirection, &yDirection);

            enemy.xSpeed = xDirection * enemy.speed;
            enemy.ySpeed = yDirection * enemy.speed;
        }

        for (auto& [_, otherEnemy] : _pool)
        {
            if (enemy.id == otherEnemy.id) continue;

            double otherX = otherEnemy.x;
            double otherY = otherEnemy.y;
            double otherRadius = otherEnemy.radius;

            bool collision =
                gfx::circlesOverlap(x, y, radius, otherX, otherY, otherRadius);

            if (!collision) continue;

            double xDirection, yDirection;

            gfx::direction(x, y, otherX, otherY, &xDirection, &yDirection);

            enemy.xSpeed = -xDirection * enemy.speed;
            enemy.ySpeed = -yDirection * enemy.speed;
        }

        enemy.x += enemy.xSpeed * deltaTimeMilliseconds;
        enemy.y += enemy.ySpeed * deltaTimeMilliseconds;
    }

    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::setDrawColor(&_ENEMY_COLOR);

    for (auto& [_, enemy] : _pool)
    {
        gfx::fillCircle(enemy.x, enemy.y, enemy.radius);
    }
}

} // namespace smocc::enemies
