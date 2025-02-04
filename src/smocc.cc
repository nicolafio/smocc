/*

smocc.cc: Entry point of the game

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#include <SDL.h>
#include <iostream>

#include "background.h"
#include "bots.h"
#include "buffs.h"
#include "bullets.h"
#include "enemies.h"
#include "explosions.h"
#include "game.h"
#include "player.h"
#include "smocc.h"
#include "ui.h"
#include "ui/buffs.h"
#include "ui/game_over.h"
#include "ui/info.h"
#include "ui/main_menu.h"
#include "ui/menu_btn.h"
#include "ui/score_record.h"
#include "ui/text.h"

using namespace std;

int main(int argc, char* argv[])
{
    return smocc::main(argc, argv);
}

namespace smocc
{

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

SDL_Window* getWindow()
{
    return _window;
}

SDL_Renderer* getRenderer()
{
    return _renderer;
}

void _init(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        cerr << "Failed to initialize SDL: " << SDL_GetError() << endl;
        exit(1);
    }

    int w = WINDOW_WIDTH;
    int h = WINDOW_HEIGHT;
    int flags = 0;

    if (SDL_CreateWindowAndRenderer(w, h, flags, &_window, &_renderer))
    {
        cerr << "Failed to open _window: " << SDL_GetError() << endl;
        exit(1);
    }

    smocc::background::init();
    smocc::ui::init();
    smocc::ui::text::init(argc, argv);
    smocc::ui::main_menu::init();
    smocc::ui::info::init();
    smocc::ui::game_over::init();
    smocc::ui::score_record::init();
    smocc::ui::buffs::init();
    smocc::game::init();
    smocc::player::init();
    smocc::enemies::init();
    smocc::bots::init();
    smocc::bullets::init();
    smocc::explosions::init();
    smocc::buffs::init();
}

void _event(SDL_Event* e)
{
    if (e->type == SDL_QUIT) _quit = true;
}

void _update()
{
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
    SDL_RenderClear(_renderer);

    SDL_Event e;

    while (!_quit && SDL_PollEvent(&e))
        _event(&e);

    smocc::background::update();
    smocc::ui::update();
    smocc::ui::main_menu::update();
    smocc::ui::info::update();
    smocc::ui::game_over::update();
    smocc::ui::score_record::update();
    smocc::ui::buffs::update();
    smocc::game::update();
    smocc::player::update();
    smocc::enemies::update();
    smocc::bots::update();
    smocc::bullets::update();
    smocc::explosions::update();
    smocc::buffs::update();

    SDL_RenderPresent(_renderer);
    SDL_Delay(_GAME_LOOP_MINIMUM_FRAME_TIME_MILLISECONDS);
}

} // namespace smocc