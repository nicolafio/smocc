#include <SDL.h>

#include "../colors.h"
#include "../game.h"
#include "../gfx.h"
#include "../ui.h"
#include "info.h"
#include "menu_btn.h"
#include "text.h"

namespace smocc::ui::main_menu
{

const char* _TITLE = "SMOCC";
const char* _PLAY_TEXT = "Play";
const char* _INFO_TEXT = "Info";

const unsigned int _TITLE_FONT_SIZE_PIXELS = 40;
const unsigned int _TITLE_MARGINS_PIXELS = 80;

SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;
SDL_Color _BG_COLOR = SMOCC_BACKGROUND_COLOR;

SDL_Texture* _title;
SDL_Texture* _playText;
SDL_Texture* _playHoverText;
SDL_Texture* _infoText;
SDL_Texture* _infoHoverText;

bool _mainMenuVisible;

int _titleWidth, _titleHeight;

void init()
{
    _mainMenuVisible = true;

    _title = ui::text::get(_TITLE, _TITLE_FONT_SIZE_PIXELS);
    _playText = ui::text::get(_PLAY_TEXT);
    _playHoverText = ui::text::get(_PLAY_TEXT, _BG_COLOR);
    _infoText = ui::text::get(_INFO_TEXT);
    _infoHoverText = ui::text::get(_INFO_TEXT, _BG_COLOR);

    SDL_QueryTexture(_title, NULL, NULL, &_titleWidth, &_titleHeight);
}

void show()
{
    _mainMenuVisible = true;
}

void update()
{
    if (!_mainMenuVisible) return;

    SDL_Rect uiRect = ui::rect();

    SDL_Rect titleRect;
    titleRect.w = _titleWidth;
    titleRect.h = _titleHeight;
    titleRect.x = uiRect.x + (uiRect.w - titleRect.w) / 2;
    titleRect.y = uiRect.y + _TITLE_MARGINS_PIXELS;

    int playBtnYPosition = titleRect.y + titleRect.h + _TITLE_MARGINS_PIXELS;

    SDL_Rect playBtnRect = menu_btn::rect(_playText, playBtnYPosition);

    int infoBtnYPosition =
        playBtnRect.y + playBtnRect.h + menu_btn::MARGIN_PIXELS;

    SDL_Rect infoBtnRect = menu_btn::rect(_infoText, infoBtnYPosition);

    gfx::renderTexture(_title, &titleRect);

    menu_btn::draw(&playBtnRect, _playText, _playHoverText);
    menu_btn::draw(&infoBtnRect, _infoText, _infoHoverText);

    bool playBtnHovering = gfx::mouseInRect(&playBtnRect);
    bool infoBtnHovering = gfx::mouseInRect(&infoBtnRect);
    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    bool pressingLeftMouseButton = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
    bool hovering = playBtnHovering || infoBtnHovering;

    setPointerStyle(hovering ? HAND : ARROW);

    if (pressingLeftMouseButton && playBtnHovering)
    {
        setPointerStyle(ARROW);
        _mainMenuVisible = false;
        game::begin();
    }

    if (pressingLeftMouseButton && infoBtnHovering)
    {
        setPointerStyle(ARROW);
        _mainMenuVisible = false;
        ui::info::show();
    }
}

} // namespace smocc::ui::main_menu