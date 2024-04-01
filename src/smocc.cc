/*

smocc.cc: Entry point of the game

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <SDL.h>
#include <iostream>

#include "enemies.h"
#include "game.h"
#include "player.h"
#include "smocc.h"
#include "ui.h"

using namespace std;

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 720
#define GAME_LOOP_MINIMUM_FRAME_TIME_MILLISECONDS 8

SDL_Window* window;
SDL_Renderer* renderer;
bool quit = false;

void init(int, char*[]);
void event(SDL_Event*);
void update();

int main(int argc, char* argv[])
{
    init(argc, argv);

    while (!quit)
        update();

    return 0;
}

void init(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        cerr << "Failed to initialize SDL: " << SDL_GetError() << endl;
        exit(1);
    }

    int w = WINDOW_WIDTH;
    int h = WINDOW_HEIGHT;
    int flags = 0;

    if (SDL_CreateWindowAndRenderer(w, h, flags, &window, &renderer))
    {
        cerr << "Failed to open window: " << SDL_GetError() << endl;
        exit(1);
    }

    smocc::ui::init(argc, argv);
    smocc::game::init();
    smocc::player::init();
    smocc::enemies::init();
}

void event(SDL_Event* e)
{
    int type = e->type;

    if (type == SDL_QUIT)
    {
        quit = true;
        return;
    }
}

void update()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Event e;

    while (!quit && SDL_PollEvent(&e))
        event(&e);

    smocc::ui::update();
    smocc::game::update();
    smocc::player::update();
    smocc::enemies::update();

    SDL_RenderPresent(renderer);
    SDL_Delay(GAME_LOOP_MINIMUM_FRAME_TIME_MILLISECONDS);
}

SDL_Window* smocc::getWindow() { return window; }

SDL_Renderer* smocc::getRenderer() { return renderer; }