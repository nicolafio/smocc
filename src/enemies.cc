/*

enemies.cc: Enemies implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>
#include <utility>

#include <SDL.h>

#include "buffs.h"
#include "bullets.h"
#include "colors.h"
#include "enemies.h"
#include "explosions.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "rng.h"
#include "smocc.h"
#include "ui/game_over.h"

using namespace std;

namespace smocc::enemies
{

using enum buffs::BuffType;

const int _SPAWN_DELAY_MILLISECONDS = 500;
const double _MIN_ENEMY_HEALTH = 1.0;
const double _MAX_ENEMY_HEALTH = 30.0;
const double _MIN_ENEMY_SPEED = 0.2;
const double _MAX_ENEMY_SPEED = 0.5;
const double _MAX_ENEMY_PUSHED_SPEED = 2.0;
const int _MIN_ENEMY_COUNT = 1;
const int _MAX_ENEMY_COUNT = 10;
const double _MIN_ENEMY_RADIUS = 15.0;
const double _MAX_ENEMY_RADIUS = 75.0;
const double _ENEMY_RADIUS_CHANGE_SPEED = 0.1;
const double _DROPPED_BUFF_RELATIVE_SPEED = 1.0 / 6.0;
const double _SLOW_ENEMIES_BUFF_FACTOR = 0.2;
const double _PUSH_ENEMIES_BUFF_FACTOR = 0.2;
const double _PUSH_ENEMIES_EFFECT_AT_MIN_HEALTH = 1.5;
const double _PUSH_ENEMIES_EFFECT_AT_MAX_HEALTH = 0.5;

SDL_Color _ENEMY_COLOR = SMOCC_FOREGROUND_COLOR;

int _maxEnemies;
unsigned long long _spawnRollsDone;
bool _resetDone;

unsigned long long _nextID;

unordered_map<unsigned long long, Enemy> _enemies;

enum SpawningEdge
{
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

void _spawnEnemy();
SpawningEdge _rollSpawningEdge();
void _initEnemyPosition(Enemy&, SpawningEdge);
void _initEnemySpeed(Enemy&);
void _initEnemyRotation(Enemy&, SpawningEdge);
void _initEnemyHealth(Enemy&);
void _destroyEnemy(Enemy&);
void _rollEnemySpawn();
void _reset();
unsigned long long _getSpawnRollsToDo();
void _doNecessarySpawnRolls();
void _updateEnemy(Enemy&);
void _updateEnemyRadius(Enemy&);
void _updateEnemyPosition(Enemy&);
void _checkPlayerCollision(Enemy&);
void _checkScreenEdgesCollision(Enemy&);
void _checkEnemyEnemyCollision(Enemy&, const Enemy&);
void _checkBulletCollision(Enemy&, const bullets::Bullet&);
void _pushEnemy(Enemy&, double, double);

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
    _maxEnemies = _MAX_ENEMY_COUNT * game::getDifficulty();

    _doNecessarySpawnRolls();

    vector<Enemy> toRemove;

    for (auto& [_, enemy] : _enemies)
        if (enemy.health <= 0) toRemove.push_back(enemy);

    for (Enemy& enemy : toRemove)
        _destroyEnemy(enemy);

    for (auto& [_, enemy] : _enemies)
    {
        _updateEnemy(enemy);

        // Terminate if enemy caused the game to end.
        if (!game::isRunning()) return;
    }

    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::setDrawColor(&_ENEMY_COLOR);

    for (auto& [_, enemy] : _enemies)
        gfx::fillCircle(enemy.x, enemy.y, enemy.radius);
}

void forEach(function<void(const Enemy& enemy)> callback)
{
    for (auto& [_, enemy] : _enemies)
        callback(enemy);
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

    if (enemiesCount < _MIN_ENEMY_COUNT) _spawnEnemy();

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

    SpawningEdge spawningEdge = _rollSpawningEdge();
    _initEnemyPosition(enemy, spawningEdge);
    _initEnemyHealth(enemy);
    _initEnemySpeed(enemy);
    _initEnemyRotation(enemy, spawningEdge);

    _enemies[enemy.id] = enemy;
}

SpawningEdge _rollSpawningEdge()
{
    double roll = rng::rollInt(0, 3);

    if (roll == 0) return LEFT;
    if (roll == 1) return RIGHT;
    if (roll == 2) return TOP;
    return BOTTOM;
}

void _initEnemyPosition(Enemy& enemy, SpawningEdge spawningEdge)
{
    double locationRoll = rng::roll();

    SDL_Window* window = smocc::getWindow();
    int windowWidth, windowHeight;

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    if (spawningEdge == LEFT)
    {
        enemy.x = 0;
        enemy.y = locationRoll * windowHeight;
    }

    if (spawningEdge == RIGHT)
    {
        enemy.x = windowWidth;
        enemy.y = locationRoll * windowHeight;
    }

    if (spawningEdge == TOP)
    {
        enemy.x = locationRoll * windowWidth;
        enemy.y = 0;
    }

    if (spawningEdge == BOTTOM)
    {
        enemy.x = locationRoll * windowWidth;
        enemy.y = windowHeight;
    }
}

void _initEnemySpeed(Enemy& enemy)
{
    double speed = lerp(_MIN_ENEMY_SPEED, _MAX_ENEMY_SPEED, rng::roll());
    enemy.initialSpeed = speed;
    enemy.speed = speed;
}

void _initEnemyRotation(Enemy& enemy, SpawningEdge spawningEdge)
{
    double rotationRadians = M_PI * rng::roll();

    if (spawningEdge == LEFT) rotationRadians -= M_PI / 2;
    if (spawningEdge == RIGHT) rotationRadians += M_PI / 2;
    if (spawningEdge == TOP) rotationRadians += M_PI;

    enemy.xSpeed = enemy.speed * cos(rotationRadians);
    enemy.ySpeed = enemy.speed * -sin(rotationRadians);
}

void _initEnemyHealth(Enemy& enemy)
{
    double min = _MIN_ENEMY_HEALTH;
    double max = _MAX_ENEMY_HEALTH;
    double difficulty = game::getDifficulty();

    enemy.health = round(lerp(min, max, rng::roll() * difficulty));
}

void _destroyEnemy(Enemy& enemy)
{
    double buffXSpeed = enemy.xSpeed * _DROPPED_BUFF_RELATIVE_SPEED;
    double buffYSpeed = enemy.ySpeed * _DROPPED_BUFF_RELATIVE_SPEED;

    if (buffs::isActive(PUSH_ENEMIES))
    {
        // Invert direction of the spawning buff because otherwise it will most
        // likely go off map when the buff for pushing enemies is active.
        buffXSpeed *= -1;
        buffYSpeed *= -1;
    }

    _enemies.erase(enemy.id);
    buffs::rollSpawn(enemy.x, enemy.y, buffXSpeed, buffYSpeed);
    game::incrementScore();
}

void _updateEnemy(Enemy& enemy)
{
    _checkPlayerCollision(enemy);

    // Terminate if game ended due to player collision.
    if (!game::isRunning()) return;

    _checkScreenEdgesCollision(enemy);

    for (auto& [_, otherEnemy] : _enemies)
        if (enemy.id != otherEnemy.id)
            _checkEnemyEnemyCollision(enemy, otherEnemy);

    bullets::forEach([&](auto b) { _checkBulletCollision(enemy, b); });

    _updateEnemyRadius(enemy);
    _updateEnemyPosition(enemy);
}

void _updateEnemyRadius(Enemy& enemy)
{
    unsigned int deltaTime = game::getDeltaTimeMilliseconds();
    double t = enemy.health / _MAX_ENEMY_HEALTH;
    double targetRadius = lerp(_MIN_ENEMY_RADIUS, _MAX_ENEMY_RADIUS, t);
    double radiusChange = _ENEMY_RADIUS_CHANGE_SPEED * (double)deltaTime;

    if (enemy.radius < targetRadius)
        enemy.radius = min(enemy.radius + radiusChange, targetRadius);

    if (enemy.radius > targetRadius)
        enemy.radius = max(enemy.radius - radiusChange, targetRadius);
}

void _updateEnemyPosition(Enemy& enemy)
{
    unsigned int deltaTime = game::getDeltaTimeMilliseconds();
    double speedFactor = 1.0;

    if (buffs::isActive(SLOW_ENEMIES)) speedFactor = _SLOW_ENEMIES_BUFF_FACTOR;

    enemy.x += speedFactor * enemy.xSpeed * deltaTime;
    enemy.y += speedFactor * enemy.ySpeed * deltaTime;
}

void _checkPlayerCollision(Enemy& enemy)
{
    double x = enemy.x;
    double y = enemy.y;
    double r = enemy.radius;
    double px = player::getXPosition();
    double py = player::getYPosition();
    double pr = player::PLAYER_CIRCLE_RADIUS;

    bool collision = gfx::circlesOverlap(x, y, r, px, py, pr);

    if (collision)
    {
        game::end();
        ui::game_over::show();
    }
}

void _checkScreenEdgesCollision(Enemy& enemy)
{
    double x = enemy.x;
    double y = enemy.y;

    double collision = !gfx::pointOnScreen(x, y);

    if (collision)
    {
        double targetX = player::getXPosition();
        double targetY = player::getYPosition();

        double xDirection, yDirection;

        gfx::direction(x, y, targetX, targetY, &xDirection, &yDirection);

        enemy.speed = enemy.initialSpeed;
        enemy.xSpeed = xDirection * enemy.speed;
        enemy.ySpeed = yDirection * enemy.speed;
    }
}

void _checkEnemyEnemyCollision(Enemy& enemy, const Enemy& otherEnemy)
{
    double x = enemy.x;
    double y = enemy.y;
    double r = enemy.radius;
    double xx = otherEnemy.x;
    double yy = otherEnemy.y;
    double rr = otherEnemy.radius;

    bool collision = gfx::circlesOverlap(x, y, r, xx, yy, rr);

    if (collision)
    {
        double xDirection, yDirection;

        gfx::direction(x, y, xx, yy, &xDirection, &yDirection);

        enemy.xSpeed = -xDirection * enemy.speed;
        enemy.ySpeed = -yDirection * enemy.speed;
    }
}

void _checkBulletCollision(Enemy& enemy, const bullets::Bullet& bullet)
{
    if (bullet.despawning) return;

    double ex = enemy.x;
    double ey = enemy.y;
    double er = enemy.radius;
    double bx = bullet.xTip;
    double by = bullet.yTip;

    bool collision = gfx::pointInCircle(bx, by, ex, ey, er);

    if (collision)
    {
        enemy.health -= bullets::BULLET_DAMAGE;

        if (buffs::isActive(DOUBLE_DAMAGE))
            enemy.health -= bullets::BULLET_DAMAGE;

        if (buffs::isActive(PUSH_ENEMIES))
            _pushEnemy(enemy, bullet.xDirection, bullet.yDirection);

        explosions::spawn(bx, by);
        bullets::despawn(bullet.id);
    }
}

void _pushEnemy(enemies::Enemy& enemy, double xAmount, double yAmount)
{
    double strength = _PUSH_ENEMIES_BUFF_FACTOR;

    // More health reduces the strength of the push.

    double h = enemy.health;
    double t = 1 - gfx::inverseLerp(_MIN_ENEMY_HEALTH, _MAX_ENEMY_HEALTH, h);
    double maxHealthEffect = _PUSH_ENEMIES_EFFECT_AT_MIN_HEALTH;
    double minHealthEffect = _PUSH_ENEMIES_EFFECT_AT_MAX_HEALTH;
    double healthEffect = lerp(minHealthEffect, maxHealthEffect, t);

    strength *= healthEffect;

    enemy.xSpeed += xAmount * strength;
    enemy.ySpeed += yAmount * strength;
    enemy.speed = gfx::magnitude(enemy.xSpeed, enemy.ySpeed);

    if (enemy.speed > _MAX_ENEMY_PUSHED_SPEED)
    {
        double xDirection, yDirection;

        gfx::unit(enemy.xSpeed, enemy.ySpeed, &xDirection, &yDirection);

        enemy.xSpeed = xDirection * _MAX_ENEMY_PUSHED_SPEED;
        enemy.ySpeed = yDirection * _MAX_ENEMY_PUSHED_SPEED;
        enemy.speed = _MAX_ENEMY_PUSHED_SPEED;
    }
}

} // namespace smocc::enemies
