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
#include <utility>

#include <SDL.h>

#include "bullets.h"
#include "colors.h"
#include "enemies.h"
#include "explosions.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "rng.h"
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
const int _MIN_ENEMY_COUNT = 1;
const int _MAX_ENEMY_COUNT = 10;
const double _MIN_ENEMY_RADIUS = 15.0;
const double _MAX_ENEMY_RADIUS = 75.0;
const double _ENEMY_RADIUS_CHANGE_SPEED = 0.1;

SDL_Color _ENEMY_COLOR = SMOCC_FOREGROUND_COLOR;

int _maxEnemies;
unsigned long long _spawnRollsDone;
bool _resetDone;

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

unsigned long long _nextID;

unordered_map<unsigned long long, Enemy> _enemies;

void _spawnEnemy();
void _rollEnemySpawn();
void _reset();
unsigned long long _getSpawnRollsToDo();
void _doNecessarySpawnRolls();

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

    unsigned int deltaTimeMilliseconds = game::getDeltaTimeMilliseconds();

    vector<Enemy> toRemove;

    for (auto& [_, enemy] : _enemies)
    {
        if (enemy.health <= 0) toRemove.push_back(enemy);
    }

    for (Enemy& enemy : toRemove)
    {
        _enemies.erase(enemy.id);
        game::incrementScore();
    }

    for (auto& [id, _] : _enemies)
    {
        Enemy& enemy = _enemies[id];

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

        for (auto& [_, otherEnemy] : _enemies)
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

    bullets::forEach(
        [&](const bullets::Bullet& bullet)
        {
            if (bullet.despawning) return;

            for (auto& [_, enemy] : _enemies)
            {
                double ex = enemy.x;
                double ey = enemy.y;
                double er = enemy.radius;

                double bx = bullet.xTip;
                double by = bullet.yTip;

                bool collision = gfx::pointInCircle(bx, by, ex, ey, er);

                if (collision)
                {
                    enemy.health -= bullets::BULLET_DAMAGE;

                    explosions::spawn(bx, by);
                    bullets::despawn(bullet.id);

                    return;
                }
            }
        });

    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::setDrawColor(&_ENEMY_COLOR);

    for (auto& [_, enemy] : _enemies)
    {
        gfx::fillCircle(enemy.x, enemy.y, enemy.radius);
    }
}

void _reset()
{
    _enemies.clear();
    _maxEnemies = 0;
    _spawnRollsDone = 0;
    _nextID = 0;

    _resetDone = true;
}

void _doNecessarySpawnRolls()
{
    unsigned long long rollsToDo = _getSpawnRollsToDo();

    for (unsigned long long i = 0; i < rollsToDo; i++)
        _rollEnemySpawn();
}

unsigned long long _getSpawnRollsToDo()
{
    unsigned long long millisecondsElapsed = game::getTimeElapsedMilliseconds();
    unsigned long long target = millisecondsElapsed / _SPAWN_DELAY_MILLISECONDS;
    return target - _spawnRollsDone;
}

void _rollEnemySpawn()
{
    int enemiesCount = _enemies.size();

    if (enemiesCount < _MIN_ENEMY_COUNT)
    {
        _spawnEnemy();
    }

    if (enemiesCount >= _MIN_ENEMY_COUNT && enemiesCount < _maxEnemies)
    {
        double spawnChance = game::getDifficulty();
        double roll = rng::roll();

        if (roll < spawnChance) _spawnEnemy();
    }

    _spawnRollsDone++;
}

void _spawnEnemy()
{
    Enemy enemy;

    enemy.id = _nextID++;
    enemy.radius = 0;

    double edgeRoll = rng::roll();

    bool spawningLeft = edgeRoll <= .25;
    bool spawningRight = edgeRoll > .25 && edgeRoll <= .5;
    bool spawningTop = edgeRoll > .5 && edgeRoll <= .75;
    bool spawningBottom = edgeRoll > .75;

    double locationRoll = rng::roll();

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

    double healthRoll = rng::roll();
    double difficulty = game::getDifficulty();
    const double healthRange = _MAX_ENEMY_HEALTH - _MIN_ENEMY_HEALTH;
    int healthAddendum = round(healthRange * healthRoll * difficulty);

    enemy.health = _MIN_ENEMY_HEALTH + healthAddendum;

    double speedRoll = rng::roll();
    const double speedRange = _MAX_ENEMY_SPEED - _MIN_ENEMY_SPEED;
    double speed = _MIN_ENEMY_SPEED + speedRange * speedRoll;

    double rotationRoll = rng::roll();
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

    _enemies[enemy.id] = enemy;
}

} // namespace smocc::enemies
