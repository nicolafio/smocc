#include "ui.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <SDL_ttf.h>

#define INCONSOLATA_LGC_FONT_PATH "inconsolata-lgc/regular.ttf"
#define INCONSOLATA_LGC_BOLD_FONT_PATH "inconsolata-lgc/bold.ttf"

#define FOREGROUND_COLOR {17, 17, 17, 255}

#define TITLE "SMOCC"
#define TITLE_FONT_SIZE_PIXELS 40
#define TITLE_MARGINS_PIXELS 80

#define BODY_FONT_SIZE_PIXELS 13

#define UI_WIDTH_PERCENTAGE 90
#define UI_HEIGHT_PERCENTAGE 90

#define MENU_BTN_WIDTH_PIXELS 400
#define MENU_BTN_PADDING_PIXELS 15
#define MENU_BTN_MARGIN_PIXELS 6
#define MENU_BTN_BORDER_OPACITY 25

#define PLAY_BUTTON_TEXT "Play"
#define INFO_BUTTON_TEXT "Info"

using namespace std;
namespace fs = std::filesystem;

TTF_Font* titleFont;
TTF_Font* font;

SDL_Color foregroundColor = FOREGROUND_COLOR;

SDL_Color btnBorderColor = {
    foregroundColor.r,
    foregroundColor.g,
    foregroundColor.b,
    MENU_BTN_BORDER_OPACITY
};

SDL_Color black = {0, 0, 0, 255};

SDL_Texture* titleTexture;
SDL_Texture* helloWorldTexture;
SDL_Texture* playBtnTextTexture;
SDL_Texture* infoBtnTextTexture;

int titleWidth, titleHeight;

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

SDL_Texture* createText(
    SDL_Renderer* renderer,
    TTF_Font* font,
    char const* text,
    SDL_Color color
) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);

    if (!surface)
    {
        cerr << "Failed to render text: " << TTF_GetError() << endl;
        exit(1);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (!texture)
    {
        cerr << "Failed to create text texture: " << SDL_GetError() << endl;
        exit(1);
    }

    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND))
    {
        cerr << "Failed to set texture blend mode: " << SDL_GetError() << endl;
        exit(1);
    }

    SDL_FreeSurface(surface);
    return texture;
}

SDL_Rect renderMenuButton(
    SDL_Renderer* renderer,
    SDL_Rect* windowRect,
    SDL_Texture* textTexture,
    int yPosition
) {
    int textWidth, textHeight;

    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

    SDL_Rect buttonRect;
    buttonRect.w = MENU_BTN_WIDTH_PIXELS;
    buttonRect.h = textHeight + 2 * MENU_BTN_PADDING_PIXELS;
    buttonRect.x = (windowRect->w - buttonRect.w) / 2;
    buttonRect.y = yPosition;

    SDL_Rect textRect;
    textRect.w = textWidth;
    textRect.h = textHeight;
    textRect.x = (windowRect->w - textRect.w) / 2;
    textRect.y = buttonRect.y + MENU_BTN_PADDING_PIXELS;

    int r = btnBorderColor.r;
    int g = btnBorderColor.g;
    int b = btnBorderColor.b;
    int a = btnBorderColor.a;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawRect(renderer, &buttonRect);

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    return buttonRect;
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

    font = openFont(fontPath, BODY_FONT_SIZE_PIXELS);
    titleFont = openFont(boldFontPath, TITLE_FONT_SIZE_PIXELS);

    titleTexture = createText(renderer, titleFont, TITLE, foregroundColor);
    playBtnTextTexture = createText(renderer, font, PLAY_BUTTON_TEXT, foregroundColor);
    infoBtnTextTexture = createText(renderer, font, INFO_BUTTON_TEXT, foregroundColor);

    SDL_QueryTexture(titleTexture, NULL, NULL, &titleWidth, &titleHeight);
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
    titleRect.w = titleWidth;
    titleRect.h = titleHeight;
    titleRect.x = (windowRect.w - titleRect.w) / 2;
    titleRect.y = uiRect.y + TITLE_MARGINS_PIXELS;

    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);

    int playBtnYPosition = titleRect.y + titleRect.h + TITLE_MARGINS_PIXELS;

    SDL_Rect playBtnRect = renderMenuButton(
        renderer,
        &windowRect,
        playBtnTextTexture,
        playBtnYPosition
    );

    int infoBtnYPosition = playBtnRect.y + playBtnRect.h + MENU_BTN_MARGIN_PIXELS;

    SDL_Rect infoBtnRect = renderMenuButton(
        renderer,
        &windowRect,
        infoBtnTextTexture,
        infoBtnYPosition
    );
}

void ui::cleanup()
{
    SDL_DestroyTexture(infoBtnTextTexture);
    SDL_DestroyTexture(playBtnTextTexture);
    SDL_DestroyTexture(titleTexture);
    TTF_CloseFont(titleFont);
    TTF_CloseFont(font);
    TTF_Quit();
}