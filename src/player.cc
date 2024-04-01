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

#define PLAYER_SPEED 0.3f
#define PLAYER_CIRCLE_RADIUS 3

bool spawned;
double xPosition;
double yPosition;
bool firstUpdate;
Uint64 lastUpdateTimeMilliseconds;

SDL_Color playerColor = SMOCC_FOREGROUND_COLOR;

void player::init() { spawned = false; }

void player::spawn()
{
    cout << "Player spawned!" << endl;
    spawned = true;

    SDL_Window* window = smocc::getWindow();
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    xPosition = w / 2;
    yPosition = h / 2;

    lastUpdateTimeMilliseconds = -1;
}

void player::update()
{
    if (!spawned)
    {
        return;
    }

    if (!game::isRunning())
    {
        spawned = false;
        return;
    }

    Uint64 now = SDL_GetTicks64();
    Uint64 deltaTimeMilliseconds = 0;

    if (lastUpdateTimeMilliseconds != -1)
    {
        deltaTimeMilliseconds = now - lastUpdateTimeMilliseconds;
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
    {
        yPosition -= PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
    {
        yPosition += PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
    {
        xPosition -= PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
    {
        xPosition += PLAYER_SPEED * deltaTimeMilliseconds;
    }

    SDL_Window* window = smocc::getWindow();

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    int minX = PLAYER_CIRCLE_RADIUS;
    int minY = PLAYER_CIRCLE_RADIUS;
    int maxX = windowWidth - PLAYER_CIRCLE_RADIUS;
    int maxY = windowHeight - PLAYER_CIRCLE_RADIUS;

    if (xPosition < minX) xPosition = minX;
    if (xPosition > maxX) xPosition = maxX;
    if (yPosition < minY) yPosition = minY;
    if (yPosition > maxY) yPosition = maxY;

    lastUpdateTimeMilliseconds = now;

    gfx::setDrawColor(&playerColor);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::fillCircle(xPosition, yPosition, PLAYER_CIRCLE_RADIUS);
}

double player::getXPosition() { return xPosition; }

double player::getYPosition() { return yPosition; }
