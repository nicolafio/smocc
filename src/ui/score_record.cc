#include "../game.h"
#include "../gfx.h"
#include "../ui.h"

#include "game_over.h"
#include "score_record.h"
#include "text.h"

namespace smocc::ui::score_record
{

const char* _SCORE_TEXT = "Score: ";
const char* _RECORD_TEXT = "Record: ";

const unsigned int _TEXT_MARGIN_PIXELS = 20;

SDL_Texture* _scoreText;
SDL_Texture* _recordText;

void init()
{
    _scoreText = ui::text::get(_SCORE_TEXT);
    _recordText = ui::text::get(_RECORD_TEXT);
}

void update()
{
    bool visible = game::isRunning() || game_over::isVisible();

    if (!visible) return;

    unsigned int score = game::getScore();
    unsigned int record = game::getRecord();

    SDL_Texture* _scoreNumber = ui::text::get(score);
    SDL_Texture* _recordNumber = ui::text::get(record);

    SDL_Rect uiRect = ui::rect();

    int scoreTextWidth = gfx::textureWidth(_scoreText);
    int recordTextWidth = gfx::textureWidth(_recordText);
    int scoreNumberWidth = gfx::textureWidth(_scoreNumber);
    int recordNumberWidth = gfx::textureWidth(_recordNumber);

    int scoreAndRecordWidth = 0;
    scoreAndRecordWidth += scoreTextWidth + scoreNumberWidth;
    scoreAndRecordWidth += _TEXT_MARGIN_PIXELS;
    scoreAndRecordWidth += recordTextWidth + recordNumberWidth;

    SDL_Rect scoreTextRect = gfx::textureSize(_scoreText);
    scoreTextRect.x = uiRect.x + (uiRect.w - scoreAndRecordWidth) / 2;
    scoreTextRect.y = uiRect.y;

    SDL_Rect scoreNumberRect = gfx::textureSize(_scoreNumber);
    scoreNumberRect.x = scoreTextRect.x + scoreTextRect.w;
    scoreNumberRect.y = uiRect.y;

    SDL_Rect recordTextRect = gfx::textureSize(_recordText);
    recordTextRect.x =
        scoreNumberRect.x + scoreNumberRect.w + _TEXT_MARGIN_PIXELS;
    recordTextRect.y = uiRect.y;

    SDL_Rect recordNumberRect = gfx::textureSize(_recordNumber);
    recordNumberRect.x = recordTextRect.x + recordTextRect.w;
    recordNumberRect.y = uiRect.y;

    gfx::renderTexture(_scoreText, &scoreTextRect);
    gfx::renderTexture(_scoreNumber, &scoreNumberRect);
    gfx::renderTexture(_recordText, &recordTextRect);
    gfx::renderTexture(_recordNumber, &recordNumberRect);
}

} // namespace smocc::ui::score_record