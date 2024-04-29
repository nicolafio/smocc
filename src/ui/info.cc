/*

ui/info.cc: Info view for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

#include <cmath>
#include <util.h>
#include <vector>

#include "../colors.h"
#include "../gfx.h"
#include "../ui.h"
#include "info.h"
#include "main_menu.h"
#include "menu_btn.h"
#include "text.h"

using namespace std;

namespace smocc::ui::info
{

using enum ui::text::FontStyle;

unsigned int _EMPTY_LINE_HEIGHT_PIXELS = 20;
unsigned int _BACK_BTN_PADDING_PIXELS = 5;
unsigned int _TITLE_FONT_SIZE = 25;

bool _infoVisible = false;
unsigned int _viewHeight;
unsigned int _viewWidth;
unsigned int _contentHeight;
unsigned int _buttonWidth;
unsigned int _buttonHeight;

vector<vector<SDL_Texture*>> _content;
vector<unsigned int> _lineHeights;

SDL_Texture* _btnText;
SDL_Texture* _btnTextHover;

void init()
{
    _infoVisible = false;

    _btnText = text::get("Main menu");
    _btnTextHover = text::get("Main menu", SMOCC_BACKGROUND_COLOR);

    auto title = [](const char* s) { return text::get(s, _TITLE_FONT_SIZE); };
    auto t = [](const char* s) { return text::get(s); };
    auto b = [](const char* s) { return text::get(s, BOLD); };

    _content = {
        {title("SMOCC")},
        {},
        {t("Game developed by "), b("Nicola Fiori (JD342)"), t(" using "),
         b("C++"), t(" and "), b("SDL2"), t(".")},
        {},
        {t("The source code of SMOCC can be found at "), b("nicolaf.io/smocc")},
        {}
    };

    _viewHeight = 0;
    _viewWidth = 0;

    for (auto& line : _content)
    {
        unsigned int lineHeight = 0;
        unsigned int lineWidth = 0;

        if (line.empty()) lineHeight = _EMPTY_LINE_HEIGHT_PIXELS;

        for (auto& text : line)
        {
            lineHeight = max(lineHeight, gfx::textureHeight(text));
            lineWidth += gfx::textureWidth(text);
        }

        _viewHeight += lineHeight;
        _viewWidth = max(_viewWidth, lineWidth);
        _lineHeights.push_back(lineHeight);
    }

    _contentHeight = _viewHeight;

    _buttonWidth = gfx::textureWidth(_btnText) + 2 * _BACK_BTN_PADDING_PIXELS;
    _buttonHeight = gfx::textureHeight(_btnText) + 2 * _BACK_BTN_PADDING_PIXELS;

    _viewHeight += _buttonHeight;
}

void show()
{
    _infoVisible = true;
}

void update()
{
    if (!_infoVisible) return;

    SDL_Rect uiRect = ui::rect();

    unsigned int viewX = uiRect.x + (uiRect.w - _viewWidth) / 2;
    unsigned int viewY = uiRect.y + (uiRect.h - _viewHeight) / 2;

    auto n = _content.size();

    for (unsigned int i = 0, lineY = viewY; i < n; lineY += _lineHeights[i++])
    {
        unsigned int textX = viewX;

        for (int j = 0; j < _content[i].size(); j++)
        {
            unsigned int lineHeight = _lineHeights[i];
            unsigned int textHeight = gfx::textureHeight(_content[i][j]);
            unsigned int textWidth = gfx::textureWidth(_content[i][j]);
            unsigned int textY = lineY + lineHeight - textHeight;

            gfx::renderTexture(_content[i][j], textX, textY);

            textX += textWidth;
        }
    }

    SDL_Rect btnRect;

    btnRect.w = _buttonWidth;
    btnRect.h = _buttonHeight;
    btnRect.x = viewX + _viewWidth - _buttonWidth;
    btnRect.y = viewY + _contentHeight;

    menu_btn::draw(&btnRect, _btnText, _btnTextHover);

    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    bool pressingLeftMouseKey = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);

    bool hoveringBtn = gfx::mouseInRect(&btnRect);
    bool clickingBtn = hoveringBtn && pressingLeftMouseKey;

    ui::setPointerStyle(hoveringBtn ? HAND : ARROW);

    if (clickingBtn)
    {
        ui::setPointerStyle(ARROW);
        _infoVisible = false;
        main_menu::show();
    }
}

} // namespace smocc::ui::info
