/*

ui/game_over.cc: Game over view for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#include "../colors.h"
#include "../game.h"
#include "../gfx.h"
#include "../ui.h"
#include "main_menu.h"
#include "menu_btn.h"
#include "text.h"

namespace smocc::ui::game_over
{

const char* _GAME_OVER_TEXT = "Game Over";
const char* _TRY_AGAIN_TEXT = "Try again";
const char* _BACK_TO_MAIN_TEXT = "Back to main menu";

const unsigned int _GAME_OVER_TEXT_MARGIN_PIXELS = 20;

SDL_Color _BG_COLOR = SMOCC_BACKGROUND_COLOR;

SDL_Texture* _gameOverText;
SDL_Texture* _tryAgainText;
SDL_Texture* _tryAgainHoverText;
SDL_Texture* _backToMainText;
SDL_Texture* _backToMainHoverText;

unsigned int _gameOverViewHeight;
int _gameOverTextWidth, _gameOverTextHeight;

bool _gameOverVisible;

void init()
{
    _gameOverVisible = false;

    _gameOverText = text::get(_GAME_OVER_TEXT);
    _tryAgainText = text::get(_TRY_AGAIN_TEXT);
    _tryAgainHoverText = text::get(_TRY_AGAIN_TEXT, _BG_COLOR);
    _backToMainText = text::get(_BACK_TO_MAIN_TEXT);
    _backToMainHoverText = text::get(_BACK_TO_MAIN_TEXT, _BG_COLOR);

    SDL_QueryTexture(
        _gameOverText, NULL, NULL, &_gameOverTextWidth, &_gameOverTextHeight
    );

    _gameOverViewHeight = _GAME_OVER_TEXT_MARGIN_PIXELS;
    _gameOverViewHeight += _gameOverTextHeight;
    _gameOverViewHeight += _GAME_OVER_TEXT_MARGIN_PIXELS;
    _gameOverViewHeight += menu_btn::PADDING_PIXELS;
    _gameOverViewHeight += gfx::textureHeight(_tryAgainText);
    _gameOverViewHeight += menu_btn::PADDING_PIXELS;
    _gameOverViewHeight += menu_btn::MARGIN_PIXELS;
    _gameOverViewHeight += menu_btn::PADDING_PIXELS;
    _gameOverViewHeight += gfx::textureHeight(_backToMainText);
    _gameOverViewHeight += menu_btn::PADDING_PIXELS;
}

void show()
{
    _gameOverVisible = true;
}

bool isVisible()
{
    return _gameOverVisible;
}

void update()
{
    if (!_gameOverVisible) return;

    SDL_Rect uiRect = ui::rect();

    SDL_Rect gameOverViewRect;
    gameOverViewRect.w = menu_btn::WIDTH_PIXELS;
    gameOverViewRect.h = _gameOverViewHeight;
    gameOverViewRect.x = uiRect.x + (uiRect.w - gameOverViewRect.w) / 2;
    gameOverViewRect.y = uiRect.y + (uiRect.h - gameOverViewRect.h) / 2;

    SDL_Rect gameOverTextRect;
    gameOverTextRect.w = _gameOverTextWidth;
    gameOverTextRect.h = _gameOverTextHeight;
    gameOverTextRect.x = uiRect.x + (uiRect.w - gameOverTextRect.w) / 2;
    gameOverTextRect.y = gameOverViewRect.y + _GAME_OVER_TEXT_MARGIN_PIXELS;

    int tryAgainBtnYPosition =
        gameOverTextRect.y + gameOverTextRect.h + _GAME_OVER_TEXT_MARGIN_PIXELS;

    SDL_Rect tryAgainBtnRect =
        menu_btn::rect(_tryAgainText, tryAgainBtnYPosition);

    int backToMainMenuBtnYPosition =
        tryAgainBtnRect.y + tryAgainBtnRect.h + menu_btn::MARGIN_PIXELS;

    SDL_Rect backBtnRect =
        menu_btn::rect(_backToMainText, backToMainMenuBtnYPosition);

    gfx::renderTexture(_gameOverText, &gameOverTextRect);

    menu_btn::draw(&tryAgainBtnRect, _tryAgainText, _tryAgainHoverText);
    menu_btn::draw(&backBtnRect, _backToMainText, _backToMainHoverText);

    bool tryAgainBtnHovering = gfx::mouseInRect(&tryAgainBtnRect);
    bool backToMainMenuBtnHovering = gfx::mouseInRect(&backBtnRect);
    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    bool pressingLeftMouseButton = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
    bool hovering = tryAgainBtnHovering || backToMainMenuBtnHovering;

    ui::setPointerStyle(hovering ? HAND : ARROW);

    if (pressingLeftMouseButton && tryAgainBtnHovering)
    {
        _gameOverVisible = false;
        ui::setPointerStyle(ARROW);
        game::begin();
        return;
    }

    if (pressingLeftMouseButton && backToMainMenuBtnHovering)
    {
        ui::setPointerStyle(ARROW);
        _gameOverVisible = false;
        ui::main_menu::show();
        return;
    }
}

} // namespace smocc::ui::game_over