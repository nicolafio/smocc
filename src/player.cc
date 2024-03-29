#include <iostream>
#include <SDL.h>

#include "smocc.h"
#include "player.h"
#include "game.h"
#include "colors.h"

using namespace std;
using namespace smocc;

#define PLAYER_SPEED 0.3f

bool spawned;
double xPosition;
double yPosition;
bool firstUpdate;
Uint64 lastUpdateTimeMilliseconds;

SDL_Color playerColor = SMOCC_FOREGROUND_COLOR;

void player::init()
{
    spawned = false;
}

void player::spawn()
{
    cout << "Player spawned!" << endl;
    spawned = true;

    SDL_Window* window = smocc::getWindow();
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    xPosition = w / 2;
    yPosition = h / 2;

    lastUpdateTimeMilliseconds = -1;
}

void player::update()
{
    if (!spawned)
    {
        return;
    }

    if (!game::isRunning())
    {
        spawned = false;
        return;
    }

    Uint64 now = SDL_GetTicks64();
    Uint64 deltaTimeMilliseconds = 0;

    if (lastUpdateTimeMilliseconds != -1)
    {
        deltaTimeMilliseconds = now - lastUpdateTimeMilliseconds;
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
    {
        yPosition -= PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
    {
        yPosition += PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
    {
        xPosition -= PLAYER_SPEED * deltaTimeMilliseconds;
    }

    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
    {
        xPosition += PLAYER_SPEED * deltaTimeMilliseconds;
    }

    SDL_Window* window = smocc::getWindow();

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    int minX = 3;
    int minY = 3;
    int maxX = windowWidth - 3;
    int maxY = windowHeight - 3;

    if (xPosition < minX) xPosition = minX;
    if (xPosition > maxX) xPosition = maxX;
    if (yPosition < minY) yPosition = minY;
    if (yPosition > maxY) yPosition = maxY;

    lastUpdateTimeMilliseconds = now;

    SDL_Renderer* renderer = smocc::getRenderer();

    SDL_Rect playerRect;
    playerRect.w = 6;
    playerRect.h = 6;
    playerRect.x = round(xPosition) - 3;
    playerRect.y = round(yPosition) - 3;

    int r = playerColor.r;
    int g = playerColor.g;
    int b = playerColor.b;
    int a = playerColor.a;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &playerRect);
}

void player::cleanup()
{

}
