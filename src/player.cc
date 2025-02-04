/*

player.cc: Player implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#include <SDL.h>
#include <algorithm>
#include <iostream>

#include "buffs.h"
#include "bullets.h"
#include "colors.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "smocc.h"

using namespace std;

namespace smocc::player
{

using enum buffs::BuffType;

SDL_Color _PLAYER_COLOR = SMOCC_FOREGROUND_COLOR;
unsigned long long _bulletSourceID;

bool _spawned;
double _x;
double _y;

void init()
{
    _spawned = false;
}

void spawn()
{
    cout << "Player spawned!" << endl;
    _spawned = true;

    _bulletSourceID = bullets::createSource();

    SDL_Window* window = smocc::getWindow();
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    _x = w / 2;
    _y = h / 2;

    bullets::setSourcePosition(_bulletSourceID, _x, _y);
}

void update()
{
    if (!_spawned) return;

    if (!game::isRunning())
    {
        if (_spawned)
        {
            _spawned = false;
            bullets::deleteSource(_bulletSourceID);
        }

        return;
    }

    unsigned int deltaTimeMilliseconds = game::getDeltaTimeMilliseconds();

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
        _y -= PLAYER_SPEED * deltaTimeMilliseconds;

    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
        _y += PLAYER_SPEED * deltaTimeMilliseconds;

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
        _x -= PLAYER_SPEED * deltaTimeMilliseconds;

    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
        _x += PLAYER_SPEED * deltaTimeMilliseconds;

    SDL_Window* window = smocc::getWindow();

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    double minX = PLAYER_CIRCLE_RADIUS;
    double minY = PLAYER_CIRCLE_RADIUS;
    double maxX = windowWidth - PLAYER_CIRCLE_RADIUS;
    double maxY = windowHeight - PLAYER_CIRCLE_RADIUS;

    _x = clamp(_x, minX, maxX);
    _y = clamp(_y, minY, maxY);

    bullets::setSourcePosition(_bulletSourceID, _x, _y);

    double xDirection;
    double yDirection;
    int xMouse;
    int yMouse;

    SDL_GetMouseState(&xMouse, &yMouse);

    gfx::direction(_x, _y, xMouse, yMouse, &xDirection, &yDirection);

    bullets::setSourceDirection(_bulletSourceID, xDirection, yDirection);

    gfx::setDrawColor(&_PLAYER_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::fillCircle(_x, _y, PLAYER_CIRCLE_RADIUS);
}

double getXPosition()
{
    return _x;
}

double getYPosition()
{
    return _y;
}

} // namespace smocc::player
