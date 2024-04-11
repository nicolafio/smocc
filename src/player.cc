/*

player.cc: Player implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <SDL.h>
#include <iostream>

#include "buffs.h"
#include "bullets.h"
#include "colors.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "smocc.h"

using namespace std;
using namespace smocc;
using enum buffs::BuffType;

namespace smocc::player
{

const double _PLAYER_SPEED = 0.3;
const int _BULLETS_SPAWN_DELAY_MILLISECONDS = 70;

SDL_Color _PLAYER_COLOR = SMOCC_FOREGROUND_COLOR;

bool _spawned;
double _xPosition;
double _yPosition;
int _bulletsSpawnCooldown;

void init()
{
    _spawned = false;
}

void spawn()
{
    cout << "Player spawned!" << endl;
    _spawned = true;

    SDL_Window* window = smocc::getWindow();
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    _xPosition = w / 2;
    _yPosition = h / 2;

    _bulletsSpawnCooldown = _BULLETS_SPAWN_DELAY_MILLISECONDS;
}

void update()
{
    if (!_spawned) return;

    if (!game::isRunning())
    {
        _spawned = false;
        return;
    }

    unsigned int deltaTimeMilliseconds = game::getDeltaTimeMilliseconds();

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
        _yPosition -= _PLAYER_SPEED * deltaTimeMilliseconds;

    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
        _yPosition += _PLAYER_SPEED * deltaTimeMilliseconds;

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
        _xPosition -= _PLAYER_SPEED * deltaTimeMilliseconds;

    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
        _xPosition += _PLAYER_SPEED * deltaTimeMilliseconds;

    SDL_Window* window = smocc::getWindow();

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    int minX = PLAYER_CIRCLE_RADIUS;
    int minY = PLAYER_CIRCLE_RADIUS;
    int maxX = windowWidth - PLAYER_CIRCLE_RADIUS;
    int maxY = windowHeight - PLAYER_CIRCLE_RADIUS;

    if (_xPosition < minX) _xPosition = minX;
    if (_xPosition > maxX) _xPosition = maxX;
    if (_yPosition < minY) _yPosition = minY;
    if (_yPosition > maxY) _yPosition = maxY;

    double xDirection;
    double yDirection;
    int xMouse;
    int yMouse;

    SDL_GetMouseState(&xMouse, &yMouse);

    gfx::direction(_xPosition, _yPosition, xMouse, yMouse, &xDirection,
                   &yDirection);

    _bulletsSpawnCooldown -= deltaTimeMilliseconds;

    if (buffs::isActive(RAPID_FIRE))
        _bulletsSpawnCooldown -= deltaTimeMilliseconds;

    if (_bulletsSpawnCooldown < 0)
    {
        _bulletsSpawnCooldown += _BULLETS_SPAWN_DELAY_MILLISECONDS;
        bullets::fire(_xPosition, _yPosition, xDirection, yDirection);
    }

    gfx::setDrawColor(&_PLAYER_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::fillCircle(_xPosition, _yPosition, PLAYER_CIRCLE_RADIUS);
}

double getXPosition()
{
    return _xPosition;
}

double getYPosition()
{
    return _yPosition;
}

} // namespace smocc::player
