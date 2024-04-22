/*

text.h: Text utility for the SMOCC UI

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <filesystem>
#include <iostream>
#include <unordered_map>

#include <SDL.h>
#include <SDL_ttf.h>

#include "../colors.h"
#include "../gfx.h"
#include "text.h"

using namespace std;
namespace fs = std::filesystem;

namespace smocc::ui::text
{

const char* _REGULAR_FONT_RELATIVE_PATH = "inconsolata-lgc/regular.ttf";
const char* _BOLD_FONT_RELATIVE_PATH = "inconsolata-lgc/bold.ttf";

fs::path _programDir;
fs::path _regularFontPath;
fs::path _boldFontPath;

SDL_Color _FG_COLOR = SMOCC_FOREGROUND_COLOR;

typedef unordered_map<string, SDL_Texture*> _textMemoL1;
typedef unordered_map<unsigned int, _textMemoL1> _textMemoL2;
typedef unordered_map<unsigned long, _textMemoL2> _textMemoL3;
unordered_map<int, _textMemoL3> _textMemo;

typedef unordered_map<int, TTF_Font*> _fontMemoL1;
unordered_map<unsigned int, _fontMemoL1> _fontMemo;

TTF_Font* _getFont(FontStyle style, unsigned int size);

unsigned long _colorToInt(SDL_Color color)
{
    return color.r << 24 | color.g << 16 | color.b << 8 | color.a;
}

void init(int argc, char* argv[])
{
    if (TTF_Init())
    {
        cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << endl;
        exit(1);
    }

    _programDir = fs::path(argv[0]).remove_filename();
    _regularFontPath = _programDir / _REGULAR_FONT_RELATIVE_PATH;
    _boldFontPath = _programDir / _BOLD_FONT_RELATIVE_PATH;
}

SDL_Texture* get(const char* str, unsigned int size, SDL_Color c, FontStyle st)
{
    auto k1 = static_cast<int>(st);
    auto k2 = _colorToInt(c);
    auto k3 = size;
    auto k4 = string(str);

    if (!_textMemo.contains(k1)) _textMemo[k1] = _textMemoL3();
    if (!_textMemo[k1].contains(k2)) _textMemo[k1][k2] = _textMemoL2();
    if (!_textMemo[k1][k2].contains(k3)) _textMemo[k1][k2][k3] = _textMemoL1();
    if (_textMemo[k1][k2][k3].contains(k4)) return _textMemo[k1][k2][k3][k4];

    TTF_Font* font = _getFont(st, size);
    SDL_Texture* texture = gfx::text(font, str, c);

    _textMemo[k1][k2][k3][k4] = texture;

    return texture;
}

SDL_Texture* get(const char* str)
{
    return get(str, REGULAR_FONT_SIZE_PIXELS, _FG_COLOR, REGULAR);
}

SDL_Texture* get(const char* str, unsigned int fontSize)
{
    return get(str, fontSize, _FG_COLOR, REGULAR);
}

SDL_Texture* get(const char* str, SDL_Color color)
{
    return get(str, REGULAR_FONT_SIZE_PIXELS, color, REGULAR);
}

SDL_Texture* get(const char* str, FontStyle style)
{
    return get(str, REGULAR_FONT_SIZE_PIXELS, _FG_COLOR, style);
}

SDL_Texture* get(unsigned int number)
{
    return get(to_string(number).c_str());
}

TTF_Font* _getFont(FontStyle style, unsigned int size)
{
    auto k1 = static_cast<int>(style);
    auto k2 = size;

    if (!_fontMemo.contains(k1)) _fontMemo[k1] = _fontMemoL1();
    if (_fontMemo[k1].contains(k2)) return _fontMemo[k1][k2];

    fs::path fontPath;

    if (style == REGULAR) fontPath = _regularFontPath;
    if (style == BOLD) fontPath = _boldFontPath;

    TTF_Font* font = gfx::font(fontPath, size);

    _fontMemo[k1][k2] = font;

    return font;
}

} // namespace smocc::ui::text
