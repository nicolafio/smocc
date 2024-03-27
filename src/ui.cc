#include "ui.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <SDL_ttf.h>

#define INCONSOLATA_LGC_FONT_PATH "inconsolata-lgc/regular.ttf"

using namespace std;
namespace fs = std::filesystem;

TTF_Font* font;
SDL_Color black = {0, 0, 0, 255};
SDL_Surface* helloWorldSurface;
SDL_Texture* helloWorldTexture;

void ui::init(int argc, char* argv[], SDL_Renderer* renderer)
{
    if (TTF_Init())
    {
        cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << endl;
        exit(1);
    }

    fs::path programPath(argv[0]);
    fs::path programDir = programPath.remove_filename();
    fs::path fontPath = programDir / INCONSOLATA_LGC_FONT_PATH;

    font = TTF_OpenFont(fontPath.c_str(), 24);

    if (!font)
    {
        cerr << "Failed to open font: " << TTF_GetError() << endl;
        exit(1);
    }

    helloWorldSurface = TTF_RenderText_Solid(font, "Hello, world!", black);
    helloWorldTexture = SDL_CreateTextureFromSurface(renderer, helloWorldSurface);
}

void ui::update(SDL_Renderer* renderer, SDL_Window* window)
{
    SDL_Rect rect;
    rect.x = 100;
    rect.y = 100;
    rect.w = helloWorldSurface->w;
    rect.h = helloWorldSurface->h;

    SDL_RenderCopy(renderer, helloWorldTexture, NULL, &rect);
}

void ui::cleanup()
{
    SDL_DestroyTexture(helloWorldTexture);
    SDL_FreeSurface(helloWorldSurface);
    TTF_CloseFont(font);
    TTF_Quit();
}