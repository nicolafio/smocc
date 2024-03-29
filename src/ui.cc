/*

ui.cc: User interface implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <iostream>
#include <filesystem>
#include <string>

#include "smocc.h"
#include "gfx.h"
#include "game.h"
#include "colors.h"
#include "ui.h"

using namespace std;
using namespace smocc;

namespace fs = std::filesystem;

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

SDL_Color fgColor = SMOCC_FOREGROUND_COLOR;
SDL_Color bgColor = SMOCC_BACKGROUND_COLOR;

SDL_Color btnBorderColor = {
    fgColor.r,
    fgColor.g,
    fgColor.b,
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

SDL_Rect computeMenuButtonRect(
    SDL_Rect* windowRect,
    SDL_Texture* textTexture,
    int yPosition
) {
    SDL_Rect rect;
    rect.w = MENU_BTN_WIDTH_PIXELS;
    rect.h = gfx::getTextureHeight(textTexture) + 2 * MENU_BTN_PADDING_PIXELS;
    rect.x = (windowRect->w - rect.w) / 2;
    rect.y = yPosition;

    return rect;
}

void renderMenuButton(
    SDL_Rect* buttonRect,
    SDL_Texture* textTexture,
    SDL_Texture* hoverTextTexture
) {
    SDL_Renderer* renderer = smocc::getRenderer();

    bool hover = gfx::isMouseInRect(buttonRect);

    SDL_Rect textRect = gfx::getTextureSize(textTexture);

    textRect.x = buttonRect->x + (buttonRect->w - textRect.w) / 2;
    textRect.y = buttonRect->y + MENU_BTN_PADDING_PIXELS;

    gfx::setDrawColor(&btnBorderColor);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::drawRect(buttonRect);

    if (!hover)
    {
        gfx::renderTexture(textTexture, &textRect);
    }

    if (hover)
    {
        gfx::setDrawColor(&fgColor);
        gfx::fillRect(buttonRect);
        gfx::renderTexture(hoverTextTexture, &textRect);
    }
}

void ui::init(int argc, char* argv[])
{
    arrowCursor = gfx::createSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    handCursor = gfx::createSystemCursor(SDL_SYSTEM_CURSOR_HAND);
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

    font = gfx::openFont(fontPath, BODY_FONT_SIZE_PIXELS);
    titleFont = gfx::openFont(boldFontPath, TITLE_FONT_SIZE_PIXELS);

    titleTexture = gfx::createText(titleFont, TITLE, fgColor);
    playBtnTextTexture = gfx::createText(font, PLAY_BUTTON_TEXT, fgColor);
    playBtnTextHoverTexture = gfx::createText(font, PLAY_BUTTON_TEXT, bgColor);
    infoBtnTextTexture = gfx::createText(font, INFO_BUTTON_TEXT, fgColor);
    infoBtnTextHoverTexture = gfx::createText(font, INFO_BUTTON_TEXT, bgColor);

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

    gfx::renderTexture(titleTexture, &titleRect);
    renderMenuButton(&playBtnRect, playBtnTextTexture, playBtnTextHoverTexture);
    renderMenuButton(&infoBtnRect, infoBtnTextTexture, infoBtnTextHoverTexture);

    bool playBtnHovering = gfx::isMouseInRect(&playBtnRect);
    bool infoBtnHovering = gfx::isMouseInRect(&infoBtnRect);
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