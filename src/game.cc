/*

game.cc: Game session implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include "game.h"
#include "player.h"

#include <SDL.h>
#include <iostream>

using namespace std;
using namespace smocc;

namespace smocc::game
{

const double _MIN_DIFFICULTY = 0.1;
const double _MAX_DIFFICULTY = 1.0;
const double _DIFFICULTY_RANGE = _MAX_DIFFICULTY - _MIN_DIFFICULTY;

bool _gameRunning;
int _score;
double _difficulty;
unsigned long long _gameStartTimeMilliseconds;

void init() { _gameRunning = false; }

void begin()
{
    cout << "Game start!" << endl;
    _gameRunning = true;
    _gameStartTimeMilliseconds = SDL_GetTicks64();

    _score = 0;

    player::spawn();
}

void update()
{
    if (!_gameRunning)
    {
        return;
    }

    double difficultyFactor = 1.0 - (1.0 / (1.0 + (double)_score / 30));

    _difficulty = _MIN_DIFFICULTY + _DIFFICULTY_RANGE * difficultyFactor;
}

void end() { _gameRunning = false; }

bool isRunning() { return _gameRunning; }

int getScore() { return _score; }

double getDifficulty() { return _difficulty; }

unsigned long long getTimeElapsedMilliseconds()
{
    return SDL_GetTicks64() - _gameStartTimeMilliseconds;
}

} // namespace smocc::game
