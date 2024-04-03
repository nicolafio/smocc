/*

smocc.cc: Entry point of the game

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <SDL.h>
#include <iostream>

#include "bullets.h"
#include "enemies.h"
#include "game.h"
#include "player.h"
#include "smocc.h"
#include "ui.h"

using namespace std;

int main(int argc, char* argv[])
{
    return smocc::main(argc, argv);
}

namespace smocc
{

const int _WINDOW_WIDTH = 1000;
const int _WINDOW_HEIGHT = 720;
const int _GAME_LOOP_MINIMUM_FRAME_TIME_MILLISECONDS = 8;

SDL_Window* _window;
SDL_Renderer* _renderer;
bool _quit = false;

void _init(int, char*[]);
void _event(SDL_Event*);
void _update();

int main(int argc, char* argv[])
{
    _init(argc, argv);

    while (!_quit)
        _update();

    return 0;
}

void _init(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        cerr << "Failed to initialize SDL: " << SDL_GetError() << endl;
        exit(1);
    }

    int w = _WINDOW_WIDTH;
    int h = _WINDOW_HEIGHT;
    int flags = 0;

    if (SDL_CreateWindowAndRenderer(w, h, flags, &_window, &_renderer))
    {
        cerr << "Failed to open _window: " << SDL_GetError() << endl;
        exit(1);
    }

    smocc::ui::init(argc, argv);
    smocc::game::init();
    smocc::player::init();
    smocc::enemies::init();
    smocc::bullets::init();
}

void _event(SDL_Event* e)
{
    int type = e->type;

    if (type == SDL_QUIT)
    {
        _quit = true;
        return;
    }
}

void _update()
{
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
    SDL_RenderClear(_renderer);

    SDL_Event e;

    while (!_quit && SDL_PollEvent(&e))
        _event(&e);

    smocc::ui::update();
    smocc::game::update();
    smocc::player::update();
    smocc::enemies::update();
    smocc::bullets::update();

    SDL_RenderPresent(_renderer);
    SDL_Delay(_GAME_LOOP_MINIMUM_FRAME_TIME_MILLISECONDS);
}

SDL_Window* getWindow()
{
    return _window;
}

SDL_Renderer* getRenderer()
{
    return _renderer;
}

} // namespace smocc