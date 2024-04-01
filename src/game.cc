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

#define MIN_DIFFICULTY 0.1
#define MAX_DIFFICULTY 1.0

const double DIFFICULTY_RANGE = MAX_DIFFICULTY - MIN_DIFFICULTY;

bool gameRunning;
int score;
double difficulty;
unsigned long long gameStartTimeMilliseconds;

void game::init() { gameRunning = false; }

void game::begin()
{
    cout << "Game start!" << endl;
    gameRunning = true;
    gameStartTimeMilliseconds = SDL_GetTicks64();

    score = 0;

    player::spawn();
}

void game::update()
{
    if (!gameRunning)
    {
        return;
    }

    double difficultyFactor = 1.0 - (1.0 / (1.0 + (double)score / 30));

    difficulty = MIN_DIFFICULTY + DIFFICULTY_RANGE * difficultyFactor;
}

void game::end() { gameRunning = false; }

bool game::isRunning() { return gameRunning; }

int game::getScore() { return score; }

double game::getDifficulty() { return difficulty; }

unsigned long long game::getTimeElapsedMilliseconds()
{
    return SDL_GetTicks64() - gameStartTimeMilliseconds;
}
