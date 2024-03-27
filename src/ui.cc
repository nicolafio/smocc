#include "ui.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <SDL_ttf.h>

#define INCONSOLATA_LGC_FONT_PATH "inconsolata-lgc/regular.ttf"
#define INCONSOLATA_LGC_BOLD_FONT_PATH "inconsolata-lgc/bold.ttf"

#define TITLE "SMOCC"
#define TITLE_FONT_SIZE_PIXELS 40
#define TITLE_MARGINS_PIXELS 80

#define UI_WIDTH_PERCENTAGE 90
#define UI_HEIGHT_PERCENTAGE 90

using namespace std;
namespace fs = std::filesystem;

TTF_Font* titleFont;
TTF_Font* font;
SDL_Color black = {0, 0, 0, 255};
SDL_Surface* titleSurface;
SDL_Texture* titleTexture;
SDL_Surface* helloWorldSurface;
SDL_Texture* helloWorldTexture;

TTF_Font* openFont(fs::path& fontPath, int size)
{
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), size);

    if (!font)
    {
        cerr << "Failed to open font: " << TTF_GetError() << endl;
        exit(1);
    }

    return font;
}

void makeText(
    SDL_Renderer* renderer,
    TTF_Font* font,
    char const* text,
    SDL_Color color,
    SDL_Surface** surface,
    SDL_Texture** texture
) {
    *surface = TTF_RenderText_Solid(font, text, color);
    *texture = SDL_CreateTextureFromSurface(renderer, *surface);
}

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
    fs::path boldFontPath = programDir / INCONSOLATA_LGC_BOLD_FONT_PATH;

    font = openFont(fontPath, 24);
    titleFont = openFont(boldFontPath, TITLE_FONT_SIZE_PIXELS);

    makeText(renderer, titleFont, TITLE, black, &titleSurface, &titleTexture);
    makeText(renderer, font, "Hello, world!", black, &helloWorldSurface, &helloWorldTexture);
}

void ui::update(SDL_Renderer* renderer, SDL_Window* window)
{
    SDL_Rect windowRect;
    windowRect.x = 0;
    windowRect.y = 0;

    SDL_GetWindowSize(window, &windowRect.w, &windowRect.h);

    SDL_Rect uiRect;
    uiRect.w = windowRect.w * UI_WIDTH_PERCENTAGE / 100;
    uiRect.h = windowRect.h * UI_HEIGHT_PERCENTAGE / 100;
    uiRect.x = (windowRect.w - uiRect.w) / 2;
    uiRect.y = (windowRect.h - uiRect.h) / 2;

    SDL_Rect titleRect;
    titleRect.w = titleSurface->w;
    titleRect.h = titleSurface->h;
    titleRect.x = (windowRect.w - titleRect.w) / 2;
    titleRect.y = uiRect.y + TITLE_MARGINS_PIXELS;

    SDL_Rect helloWorldRect;
    helloWorldRect.w = helloWorldSurface->w;
    helloWorldRect.h = helloWorldSurface->h;
    helloWorldRect.x = (windowRect.w - helloWorldRect.w) / 2;
    helloWorldRect.y = titleRect.y + titleRect.h + TITLE_MARGINS_PIXELS;

    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    SDL_RenderCopy(renderer, helloWorldTexture, NULL, &helloWorldRect);
}

void ui::cleanup()
{
    SDL_DestroyTexture(titleTexture);
    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(helloWorldTexture);
    SDL_FreeSurface(helloWorldSurface);
    TTF_CloseFont(font);
    TTF_Quit();
}