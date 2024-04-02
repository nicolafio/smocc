/*

player.cc: Player implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <SDL.h>
#include <iostream>

#include "colors.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "smocc.h"

using namespace std;
using namespace smocc;

namespace smocc::player
{

const double _PLAYER_SPEED = 0.3;
SDL_Color _PLAYER_COLOR = SMOCC_FOREGROUND_COLOR;

bool _spawned;
double _xPosition;
double _yPosition;
Uint64 _lastUpdateTimeMilliseconds;

void init() { _spawned = false; }

void spawn()
{
    cout << "Player spawned!" << endl;
    _spawned = true;

    SDL_Window* window = smocc::getWindow();
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    _xPosition = w / 2;
    _yPosition = h / 2;

    _lastUpdateTimeMilliseconds = -1;
}

void update()
{
    if (!_spawned)
    {
        return;
    }

    if (!game::isRunning())
    {
        _spawned = false;
        return;
    }

    Uint64 now = SDL_GetTicks64();
    Uint64 deltaTimeMilliseconds = 0;

    if (_lastUpdateTimeMilliseconds != -1)
    {
        deltaTimeMilliseconds = now - _lastUpdateTimeMilliseconds;
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
    {
        _yPosition -= _PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
    {
        _yPosition += _PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
    {
        _xPosition -= _PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
    {
        _xPosition += _PLAYER_SPEED * deltaTimeMilliseconds;
    }

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

    _lastUpdateTimeMilliseconds = now;

    gfx::setDrawColor(&_PLAYER_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::fillCircle(_xPosition, _yPosition, PLAYER_CIRCLE_RADIUS);
}

double getXPosition() { return _xPosition; }

double getYPosition() { return _yPosition; }

} // namespace smocc::player
