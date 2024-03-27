#include <iostream>
#include <SDL.h>

using namespace std;

#include "main.h"
#include "ui.h"

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 720
#define GAME_LOOP_MINIMUM_FRAME_TIME_MILLISECONDS 8

SDL_Window   *window;
SDL_Renderer *renderer;
bool         quit = false;

int main(int argc, char* argv[])
{
    init(argc, argv);

    atexit(cleanup);

    SDL_Event e;

    while (!quit)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    	SDL_RenderClear(renderer);

        while (!quit && SDL_PollEvent(&e))
        {
            event(&e);
        }

        update();

        SDL_RenderPresent(renderer);
        SDL_Delay(GAME_LOOP_MINIMUM_FRAME_TIME_MILLISECONDS);
    }
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

    ui::init(argc, argv, renderer);
}

void event(SDL_Event *e)
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
    ui::update(renderer, window);
}

void cleanup()
{
    ui::cleanup();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}