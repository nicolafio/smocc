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
const int _SCORE_INCREMENT = 100;

bool _gameRunning;
unsigned int _score;
unsigned int _record;
double _difficulty;
unsigned long long _gameStartTimeMilliseconds;
unsigned long long _lastUpdateTimeMilliseconds;
unsigned int _deltaTime;

void init()
{
    _gameRunning = false;
    _record = 0;
}

void begin()
{
    cout << "Game start!" << endl;
    _gameRunning = true;
    _gameStartTimeMilliseconds = SDL_GetTicks64();
    _lastUpdateTimeMilliseconds = _gameStartTimeMilliseconds;
    _deltaTime = 0;

    _score = 0;

    player::spawn();
}

void update()
{
    if (!_gameRunning)
    {
        return;
    }

    if (_score > _record)
    {
        _record = _score;
    }

    unsigned long long currentTime = SDL_GetTicks64();

    _deltaTime = currentTime - _lastUpdateTimeMilliseconds;
    _lastUpdateTimeMilliseconds = currentTime;

    double difficultyFactor = 1.0 - (1.0 / (1.0 + (double)_score / 30));

    _difficulty = _MIN_DIFFICULTY + _DIFFICULTY_RANGE * difficultyFactor;
}

void end()
{
    _gameRunning = false;
}

bool isRunning()
{
    return _gameRunning;
}

unsigned int getScore()
{
    return _score;
}

void incrementScore()
{
    _score += _SCORE_INCREMENT;
}

unsigned int getRecord()
{
    return _record;
}

double getDifficulty()
{
    return _difficulty;
}

unsigned long long getTimeElapsedMilliseconds()
{
    return SDL_GetTicks64() - _gameStartTimeMilliseconds;
}

unsigned int getDeltaTimeMilliseconds()
{
    return _deltaTime;
}

} // namespace smocc::game
