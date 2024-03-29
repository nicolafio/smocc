/*

game.cc: Game session implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include "game.h"
#include "player.h"

#include <iostream>

using namespace std;
using namespace smocc;

bool gameRunning;

void game::init()
{
    gameRunning = false;
}

void game::begin()
{
    cout << "Game start!" << endl;
    gameRunning = true;
    player::spawn();
}

void game::update()
{
    if (!gameRunning)
    {
        return;
    }
}

void game::end()
{
    gameRunning = false;
}

bool game::isRunning()
{
    return gameRunning;
}