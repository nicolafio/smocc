/*

ui.cc: User interface implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <filesystem>
#include <iostream>
#include <string>

#include "colors.h"
#include "game.h"
#include "gfx.h"
#include "smocc.h"
#include "ui.h"

using namespace std;
using namespace smocc;

namespace fs = std::filesystem;

namespace smocc::ui
{

const char* _INCONSOLATA_LGC_FONT_PATH = "inconsolata-lgc/regular.ttf";
const char* _INCONSOLATA_LGC_BOLD_FONT_PATH = "inconsolata-lgc/bold.ttf";

const char* _TITLE = "SMOCC";
const int _TITLE_FONT_SIZE_PIXELS = 40;
const int _TITLE_MARGINS_PIXELS = 80;

const int _BODY_FONT_SIZE_PIXELS = 13;

const int _UI_WIDTH_PERCENTAGE = 90;
const int _UI_HEIGHT_PERCENTAGE = 90;

const int _MENU_BTN_WIDTH_PIXELS = 400;
const int _MENU_BTN_PADDING_PIXELS = 15;
const int _MENU_BTN_MARGIN_PIXELS = 6;
const int _MENU_BTN_BORDER_OPACITY = 25;

const char* _PLAY_TEXT = "Play";
const char* _INFO_TEXT = "Info";

using namespace std;
using namespace smocc;
namespace fs = std::filesystem;

SDL_Cursor* _arrowCursor;
SDL_Cursor* _handCursor;

TTF_Font* _titleFont;
TTF_Font* _font;

SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;
SDL_Color _BG_COLOR = SMOCC_BACKGROUND_COLOR;

SDL_Color _BTN_BORDER_COLOR = {_FG_COLOR.r, _FG_COLOR.g, _FG_COLOR.b,
                               _MENU_BTN_BORDER_OPACITY};

SDL_Color _BLACK = {0, 0, 0, 255};

SDL_Texture* _titleTexture;
SDL_Texture* _playBtnTextTexture;
SDL_Texture* _playBtnTextHoverTexture;
SDL_Texture* _infoBtnTextTexture;
SDL_Texture* _infoBtnTextHoverTexture;

int _titleWidth, _titleHeight;

bool _mainMenuVisible;

SDL_Rect _computeMenuButtonRect(SDL_Rect* windowRect, SDL_Texture* textTexture,
                                int yPosition)
{
    SDL_Rect rect;
    rect.w = _MENU_BTN_WIDTH_PIXELS;
    rect.h = gfx::getTextureHeight(textTexture) + 2 * _MENU_BTN_PADDING_PIXELS;
    rect.x = (windowRect->w - rect.w) / 2;
    rect.y = yPosition;

    return rect;
}

void _renderMenuButton(SDL_Rect* buttonRect, SDL_Texture* textTexture,
                       SDL_Texture* hoverTextTexture)
{
    SDL_Renderer* renderer = smocc::getRenderer();

    bool hover = gfx::isMouseInRect(buttonRect);

    SDL_Rect textRect = gfx::getTextureSize(textTexture);

    textRect.x = buttonRect->x + (buttonRect->w - textRect.w) / 2;
    textRect.y = buttonRect->y + _MENU_BTN_PADDING_PIXELS;

    gfx::setDrawColor(&_BTN_BORDER_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::drawRect(buttonRect);

    if (!hover)
    {
        gfx::renderTexture(textTexture, &textRect);
    }

    if (hover)
    {
        gfx::setDrawColor(&_FG_COLOR);
        gfx::fillRect(buttonRect);
        gfx::renderTexture(hoverTextTexture, &textRect);
    }
}

void init(int argc, char* argv[])
{
    _arrowCursor = gfx::createSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    _handCursor = gfx::createSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    _mainMenuVisible = true;

    if (TTF_Init())
    {
        cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << endl;
        exit(1);
    }

    fs::path programPath(argv[0]);
    fs::path programDir = programPath.remove_filename();
    fs::path fontPath = programDir / _INCONSOLATA_LGC_FONT_PATH;
    fs::path boldFontPath = programDir / _INCONSOLATA_LGC_BOLD_FONT_PATH;

    _font = gfx::openFont(fontPath, _BODY_FONT_SIZE_PIXELS);
    _titleFont = gfx::openFont(boldFontPath, _TITLE_FONT_SIZE_PIXELS);

    _titleTexture = gfx::createText(_titleFont, _TITLE, _FG_COLOR);
    _playBtnTextTexture = gfx::createText(_font, _PLAY_TEXT, _FG_COLOR);
    _playBtnTextHoverTexture = gfx::createText(_font, _PLAY_TEXT, _BG_COLOR);
    _infoBtnTextTexture = gfx::createText(_font, _INFO_TEXT, _FG_COLOR);
    _infoBtnTextHoverTexture = gfx::createText(_font, _INFO_TEXT, _BG_COLOR);

    SDL_QueryTexture(_titleTexture, NULL, NULL, &_titleWidth, &_titleHeight);
}

void update()
{
    SDL_Window* window = smocc::getWindow();

    SDL_Rect windowRect;
    windowRect.x = 0;
    windowRect.y = 0;

    SDL_GetWindowSize(window, &windowRect.w, &windowRect.h);

    if (!_mainMenuVisible) return;

    SDL_Rect uiRect;
    uiRect.w = windowRect.w * _UI_WIDTH_PERCENTAGE / 100;
    uiRect.h = windowRect.h * _UI_HEIGHT_PERCENTAGE / 100;
    uiRect.x = (windowRect.w - uiRect.w) / 2;
    uiRect.y = (windowRect.h - uiRect.h) / 2;

    SDL_Rect titleRect;
    titleRect.w = _titleWidth;
    titleRect.h = _titleHeight;
    titleRect.x = (windowRect.w - titleRect.w) / 2;
    titleRect.y = uiRect.y + _TITLE_MARGINS_PIXELS;

    int playBtnYPosition = titleRect.y + titleRect.h + _TITLE_MARGINS_PIXELS;

    SDL_Rect playBtnRect = _computeMenuButtonRect(
        &windowRect, _playBtnTextTexture, playBtnYPosition);

    int infoBtnYPosition =
        playBtnRect.y + playBtnRect.h + _MENU_BTN_MARGIN_PIXELS;

    SDL_Rect infoBtnRect = _computeMenuButtonRect(
        &windowRect, _infoBtnTextTexture, infoBtnYPosition);

    gfx::renderTexture(_titleTexture, &titleRect);

    _renderMenuButton(&playBtnRect, _playBtnTextTexture,
                      _playBtnTextHoverTexture);

    _renderMenuButton(&infoBtnRect, _infoBtnTextTexture,
                      _infoBtnTextHoverTexture);

    bool playBtnHovering = gfx::isMouseInRect(&playBtnRect);
    bool infoBtnHovering = gfx::isMouseInRect(&infoBtnRect);
    bool hovering = playBtnHovering || infoBtnHovering;
    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    bool pressingLeftButton = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);

    if (hovering)
    {
        SDL_SetCursor(_handCursor);
    }

    if (!hovering)
    {
        SDL_SetCursor(_arrowCursor);
    }

    if (pressingLeftButton && playBtnHovering)
    {
        SDL_SetCursor(_arrowCursor);
        _mainMenuVisible = false;
        game::begin();
    }
}

} // namespace smocc::ui
