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

void game::cleanup()
{

}

bool game::isRunning()
{
    return gameRunning;
}