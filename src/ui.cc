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

const int _GAME_OVER_TEXT_MARGIN = 20;

const char* _PLAY_TEXT = "Play";
const char* _INFO_TEXT = "Info";

const char* _GAME_OVER_TEXT = "Game Over";
const char* _TRY_AGAIN_TEXT = "Try again";
const char* _BACK_TO_MAIN_MENU_TEXT = "Back to main menu";

using namespace std;
using namespace smocc;
namespace fs = std::filesystem;

SDL_Cursor* _arrowCursor;
SDL_Cursor* _handCursor;

TTF_Font* _titleFont;
TTF_Font* _font;
TTF_Font* _boldFont;

SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;
SDL_Color _BG_COLOR = SMOCC_BACKGROUND_COLOR;

SDL_Color _BTN_BORDER_COLOR = {_FG_COLOR.r, _FG_COLOR.g, _FG_COLOR.b,
                               _MENU_BTN_BORDER_OPACITY};

SDL_Texture* _titleTexture;
SDL_Texture* _playBtnTextTexture;
SDL_Texture* _playBtnTextHoverTexture;
SDL_Texture* _infoBtnTextTexture;
SDL_Texture* _infoBtnTextHoverTexture;

SDL_Texture* _gameOverTextTexture;
SDL_Texture* _tryAgainTextTexture;
SDL_Texture* _tryAgainTextHoverTexture;
SDL_Texture* _backToMainMenuTextTexture;
SDL_Texture* _backToMainMenuTextHoverTexture;

SDL_Rect _uiRect;

int _titleWidth, _titleHeight;
int _gameOverViewHeight;
int _gameOverTextWidth, _gameOverTextHeight;

bool _mainMenuVisible;
bool _gameOverVisible;
bool _hoveringOverUIButton;
bool _pressingLeftMouseButton;

void _updateMainMenu();
void _updateGameOver();

SDL_Rect _computeMenuButtonRect(SDL_Texture* textTexture, int yPosition)
{
    SDL_Rect rect;
    rect.w = _MENU_BTN_WIDTH_PIXELS;
    rect.h = gfx::getTextureHeight(textTexture) + 2 * _MENU_BTN_PADDING_PIXELS;
    rect.x = _uiRect.x + (_uiRect.w - rect.w) / 2;
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
    _gameOverVisible = false;
    _hoveringOverUIButton = false;

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
    _boldFont = gfx::openFont(boldFontPath, _BODY_FONT_SIZE_PIXELS);
    _titleFont = gfx::openFont(boldFontPath, _TITLE_FONT_SIZE_PIXELS);

    _titleTexture = gfx::createText(_titleFont, _TITLE, _FG_COLOR);
    _playBtnTextTexture = gfx::createText(_font, _PLAY_TEXT, _FG_COLOR);
    _playBtnTextHoverTexture = gfx::createText(_font, _PLAY_TEXT, _BG_COLOR);
    _infoBtnTextTexture = gfx::createText(_font, _INFO_TEXT, _FG_COLOR);
    _infoBtnTextHoverTexture = gfx::createText(_font, _INFO_TEXT, _BG_COLOR);

    _gameOverTextTexture =
        gfx::createText(_boldFont, _GAME_OVER_TEXT, _FG_COLOR);
    _tryAgainTextTexture = gfx::createText(_font, _TRY_AGAIN_TEXT, _FG_COLOR);
    _tryAgainTextHoverTexture =
        gfx::createText(_font, _TRY_AGAIN_TEXT, _BG_COLOR);
    _backToMainMenuTextTexture =
        gfx::createText(_font, _BACK_TO_MAIN_MENU_TEXT, _FG_COLOR);
    _backToMainMenuTextHoverTexture =
        gfx::createText(_font, _BACK_TO_MAIN_MENU_TEXT, _BG_COLOR);

    SDL_QueryTexture(_titleTexture, NULL, NULL, &_titleWidth, &_titleHeight);

    SDL_QueryTexture(_gameOverTextTexture, NULL, NULL, &_gameOverTextWidth,
                     &_gameOverTextHeight);

    _gameOverViewHeight = _GAME_OVER_TEXT_MARGIN;
    _gameOverViewHeight += _gameOverTextHeight;
    _gameOverViewHeight += _GAME_OVER_TEXT_MARGIN;
    _gameOverViewHeight += _MENU_BTN_PADDING_PIXELS;
    _gameOverViewHeight += gfx::getTextureHeight(_tryAgainTextTexture);
    _gameOverViewHeight += _MENU_BTN_PADDING_PIXELS;
    _gameOverViewHeight += _MENU_BTN_MARGIN_PIXELS;
    _gameOverViewHeight += _MENU_BTN_PADDING_PIXELS;
    _gameOverViewHeight += gfx::getTextureHeight(_backToMainMenuTextTexture);
    _gameOverViewHeight += _MENU_BTN_PADDING_PIXELS;
}

void update()
{
    SDL_Window* window = smocc::getWindow();

    SDL_Rect windowRect;
    windowRect.x = 0;
    windowRect.y = 0;

    SDL_GetWindowSize(window, &windowRect.w, &windowRect.h);

    _uiRect.w = windowRect.w * _UI_WIDTH_PERCENTAGE / 100;
    _uiRect.h = windowRect.h * _UI_HEIGHT_PERCENTAGE / 100;
    _uiRect.x = (windowRect.w - _uiRect.w) / 2;
    _uiRect.y = (windowRect.h - _uiRect.h) / 2;

    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);

    _pressingLeftMouseButton = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);

    _updateMainMenu();
    _updateGameOver();

    SDL_SetCursor(_hoveringOverUIButton ? _handCursor : _arrowCursor);
}

void showGameOver() { _gameOverVisible = true; }

void _updateMainMenu()
{
    if (!_mainMenuVisible) return;

    SDL_Rect titleRect;
    titleRect.w = _titleWidth;
    titleRect.h = _titleHeight;
    titleRect.x = _uiRect.x + (_uiRect.w - titleRect.w) / 2;
    titleRect.y = _uiRect.y + _TITLE_MARGINS_PIXELS;

    int playBtnYPosition = titleRect.y + titleRect.h + _TITLE_MARGINS_PIXELS;

    SDL_Rect playBtnRect =
        _computeMenuButtonRect(_playBtnTextTexture, playBtnYPosition);

    int infoBtnYPosition =
        playBtnRect.y + playBtnRect.h + _MENU_BTN_MARGIN_PIXELS;

    SDL_Rect infoBtnRect =
        _computeMenuButtonRect(_infoBtnTextTexture, infoBtnYPosition);

    gfx::renderTexture(_titleTexture, &titleRect);

    _renderMenuButton(&playBtnRect, _playBtnTextTexture,
                      _playBtnTextHoverTexture);

    _renderMenuButton(&infoBtnRect, _infoBtnTextTexture,
                      _infoBtnTextHoverTexture);

    bool playBtnHovering = gfx::isMouseInRect(&playBtnRect);
    bool infoBtnHovering = gfx::isMouseInRect(&infoBtnRect);

    _hoveringOverUIButton = playBtnHovering || infoBtnHovering;

    if (_pressingLeftMouseButton && playBtnHovering)
    {
        _hoveringOverUIButton = false;
        _mainMenuVisible = false;
        game::begin();
    }
}

void _updateGameOver()
{
    if (!_gameOverVisible) return;

    SDL_Rect gameOverViewRect;
    gameOverViewRect.w = _MENU_BTN_WIDTH_PIXELS;
    gameOverViewRect.h = _gameOverViewHeight;
    gameOverViewRect.x = _uiRect.x + (_uiRect.w - gameOverViewRect.w) / 2;
    gameOverViewRect.y = _uiRect.y + (_uiRect.h - gameOverViewRect.h) / 2;

    SDL_Rect gameOverTextRect;
    gameOverTextRect.w = _gameOverTextWidth;
    gameOverTextRect.h = _gameOverTextHeight;
    gameOverTextRect.x = _uiRect.x + (_uiRect.w - gameOverTextRect.w) / 2;
    gameOverTextRect.y = gameOverViewRect.y + _GAME_OVER_TEXT_MARGIN;

    int tryAgainBtnYPosition =
        gameOverTextRect.y + gameOverTextRect.h + _GAME_OVER_TEXT_MARGIN;

    SDL_Rect tryAgainBtnRect =
        _computeMenuButtonRect(_tryAgainTextTexture, tryAgainBtnYPosition);

    int backToMainMenuBtnYPosition =
        tryAgainBtnRect.y + tryAgainBtnRect.h + _MENU_BTN_MARGIN_PIXELS;

    SDL_Rect backToMainMenuBtnRect = _computeMenuButtonRect(
        _backToMainMenuTextTexture, backToMainMenuBtnYPosition);

    gfx::renderTexture(_gameOverTextTexture, &gameOverTextRect);

    _renderMenuButton(&tryAgainBtnRect, _tryAgainTextTexture,
                      _tryAgainTextHoverTexture);

    _renderMenuButton(&backToMainMenuBtnRect, _backToMainMenuTextTexture,
                      _backToMainMenuTextHoverTexture);

    bool tryAgainBtnHovering = gfx::isMouseInRect(&tryAgainBtnRect);
    bool backToMainMenuBtnHovering = gfx::isMouseInRect(&backToMainMenuBtnRect);

    _hoveringOverUIButton = tryAgainBtnHovering || backToMainMenuBtnHovering;

    if (_pressingLeftMouseButton && tryAgainBtnHovering)
    {
        cout << "Clicked 'Try again'!" << endl;

        _hoveringOverUIButton = false;
        _gameOverVisible = false;
        game::begin();
        return;
    }

    if (_pressingLeftMouseButton && backToMainMenuBtnHovering)
    {
        _hoveringOverUIButton = false;
        _gameOverVisible = false;
        _mainMenuVisible = true;
        return;
    }
}

} // namespace smocc::ui
