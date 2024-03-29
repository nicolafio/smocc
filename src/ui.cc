#include "smocc.h"
#include "ui.h"
#include "game.h"
#include "colors.h"

#include <iostream>
#include <filesystem>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

#define INCONSOLATA_LGC_FONT_PATH "inconsolata-lgc/regular.ttf"
#define INCONSOLATA_LGC_BOLD_FONT_PATH "inconsolata-lgc/bold.ttf"

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
using namespace smocc;
namespace fs = std::filesystem;

SDL_Cursor* arrowCursor;
SDL_Cursor* handCursor;

TTF_Font* titleFont;
TTF_Font* font;

SDL_Color foregroundColor = SMOCC_FOREGROUND_COLOR;
SDL_Color backgroundColor = SMOCC_BACKGROUND_COLOR;

SDL_Color btnBorderColor = {
    foregroundColor.r,
    foregroundColor.g,
    foregroundColor.b,
    MENU_BTN_BORDER_OPACITY
};

SDL_Color black = {0, 0, 0, 255};

SDL_Texture* titleTexture;
SDL_Texture* playBtnTextTexture;
SDL_Texture* playBtnTextHoverTexture;
SDL_Texture* infoBtnTextTexture;
SDL_Texture* infoBtnTextHoverTexture;

int titleWidth, titleHeight;

bool mainMenuVisible;

SDL_Cursor* createSystemCursor(SDL_SystemCursor cursor)
{
    SDL_Cursor* newCursor = SDL_CreateSystemCursor(cursor);

    if (!newCursor)
    {
        cerr << "Failed to create system cursor: " << SDL_GetError() << endl;
        exit(1);
    }

    return newCursor;
}

bool isPointInRect(int x, int y, SDL_Rect* rect)
{
    return x >= rect->x && x <= rect->x + rect->w &&
           y >= rect->y && y <= rect->y + rect->h;
}

bool isMouseInRect(SDL_Rect* rect)
{
    int mouseX, mouseY;

    SDL_GetMouseState(&mouseX, &mouseY);

    return isPointInRect(mouseX, mouseY, rect);
}

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

SDL_Texture* createText(TTF_Font* font, char const* text, SDL_Color color) {
    SDL_Renderer* renderer = smocc::getRenderer();

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

SDL_Rect getTextureSize(SDL_Texture* texture)
{
    SDL_Rect size;
    size.x = 0;
    size.y = 0;

    if (SDL_QueryTexture(texture, NULL, NULL, &size.w, &size.h))
    {
        cerr << "Failed to query texture width and height: ";
        cerr << SDL_GetError() << endl;
        exit(1);
    }

    return size;
}

int getTextureHeight(SDL_Texture* texture)
{
    int height;

    if (SDL_QueryTexture(texture, NULL, NULL, NULL, &height))
    {
        cerr << "Failed to query texture height: " << SDL_GetError() << endl;
        exit(1);
    }

    return height;
}

void setDrawColor(SDL_Color* color)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    int r = color->r;
    int g = color->g;
    int b = color->b;
    int a = color->a;

    if (SDL_SetRenderDrawColor(renderer, r, g, b, a))
    {
        cerr << "Failed to set render draw color: " << SDL_GetError() << endl;
        exit(1);
    }
}

void setDrawBlendMode(SDL_BlendMode blendMode)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_SetRenderDrawBlendMode(renderer, blendMode))
    {
        cerr << "Failed to set render draw blend mode: " << SDL_GetError() << endl;
        exit(1);
    }
}

void drawRect(SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderDrawRect(renderer, rect))
    {
        cerr << "Failed to render rectangle: " << SDL_GetError() << endl;
        exit(1);
    }
}

void fillRect(SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderFillRect(renderer, rect))
    {
        cerr << "Failed to fill rectangle: " << SDL_GetError() << endl;
        exit(1);
    }
}

void renderTexture(SDL_Texture* texture, SDL_Rect* rect)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    if (SDL_RenderCopy(renderer, texture, NULL, rect))
    {
        cerr << "Failed to render texture: " << SDL_GetError() << endl;
        exit(1);
    }
}

SDL_Rect computeMenuButtonRect(
    SDL_Rect* windowRect,
    SDL_Texture* textTexture,
    int yPosition
) {
    SDL_Rect buttonRect;
    buttonRect.w = MENU_BTN_WIDTH_PIXELS;
    buttonRect.h = getTextureHeight(textTexture) + 2 * MENU_BTN_PADDING_PIXELS;
    buttonRect.x = (windowRect->w - buttonRect.w) / 2;
    buttonRect.y = yPosition;

    return buttonRect;
}

void renderMenuButton(
    SDL_Rect* buttonRect,
    SDL_Texture* textTexture,
    SDL_Texture* hoverTextTexture
) {
    SDL_Renderer* renderer = smocc::getRenderer();

    bool hover = isMouseInRect(buttonRect);

    SDL_Rect textRect = getTextureSize(textTexture);

    textRect.x = buttonRect->x + (buttonRect->w - textRect.w) / 2;
    textRect.y = buttonRect->y + MENU_BTN_PADDING_PIXELS;

    setDrawColor(&btnBorderColor);
    setDrawBlendMode(SDL_BLENDMODE_BLEND);
    drawRect(buttonRect);

    if (!hover)
    {
        renderTexture(textTexture, &textRect);
    }

    if (hover)
    {
        setDrawColor(&foregroundColor);
        fillRect(buttonRect);
        renderTexture(hoverTextTexture, &textRect);
    }
}

void ui::init(int argc, char* argv[])
{
    arrowCursor = createSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    handCursor = createSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    mainMenuVisible = true;

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

    titleTexture = createText(titleFont, TITLE, foregroundColor);
    playBtnTextTexture = createText(font, PLAY_BUTTON_TEXT, foregroundColor);
    playBtnTextHoverTexture = createText(font, PLAY_BUTTON_TEXT, backgroundColor);
    infoBtnTextTexture = createText(font, INFO_BUTTON_TEXT, foregroundColor);
    infoBtnTextHoverTexture = createText(font, INFO_BUTTON_TEXT, backgroundColor);

    SDL_QueryTexture(titleTexture, NULL, NULL, &titleWidth, &titleHeight);
}

void ui::update()
{
    SDL_Window* window = smocc::getWindow();

    SDL_Rect windowRect;
    windowRect.x = 0;
    windowRect.y = 0;

    SDL_GetWindowSize(window, &windowRect.w, &windowRect.h);

    if (!mainMenuVisible) return;

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

    int playBtnYPosition = titleRect.y + titleRect.h + TITLE_MARGINS_PIXELS;

    SDL_Rect playBtnRect = computeMenuButtonRect(
        &windowRect,
        playBtnTextTexture,
        playBtnYPosition
    );

    int infoBtnYPosition = playBtnRect.y + playBtnRect.h + MENU_BTN_MARGIN_PIXELS;

    SDL_Rect infoBtnRect = computeMenuButtonRect(
        &windowRect,
        infoBtnTextTexture,
        infoBtnYPosition
    );

    renderTexture(titleTexture, &titleRect);
    renderMenuButton(&playBtnRect, playBtnTextTexture, playBtnTextHoverTexture);
    renderMenuButton(&infoBtnRect, infoBtnTextTexture, infoBtnTextHoverTexture);

    bool playBtnHovering = isMouseInRect(&playBtnRect);
    bool infoBtnHovering = isMouseInRect(&infoBtnRect);
    bool hovering = playBtnHovering || infoBtnHovering;
    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    bool pressingLeftButton = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);

    if (hovering)
    {
        SDL_SetCursor(handCursor);
    }

    if (!hovering)
    {
        SDL_SetCursor(arrowCursor);
    }

    if (pressingLeftButton && playBtnHovering)
    {
        SDL_SetCursor(arrowCursor);
        mainMenuVisible = false;
        game::begin();
    }
}

void ui::cleanup()
{
    SDL_DestroyTexture(infoBtnTextHoverTexture);
    SDL_DestroyTexture(infoBtnTextTexture);
    SDL_DestroyTexture(playBtnTextHoverTexture);
    SDL_DestroyTexture(playBtnTextTexture);
    SDL_DestroyTexture(titleTexture);
    TTF_CloseFont(titleFont);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_FreeCursor(handCursor);
    SDL_FreeCursor(arrowCursor);
}