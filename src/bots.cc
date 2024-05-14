/*

bots.cc: Friendly bots implementation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

// FIXME: buggy and laggy!

#include <cassert>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include "bots.h"
#include "buffs.h"
#include "bullets.h"
#include "colors.h"
#include "enemies.h"
#include "game.h"
#include "gfx.h"
#include "player.h"
#include "rng.h"
#include "smocc.h"

using namespace std;

namespace smocc::bots
{

using enum buffs::BuffType;

const unsigned int _WAYPOINT_CHECK_RADIUS = enemies::MAX_ENEMY_RADIUS * 2;
const unsigned int _WAYPOINTS_PER_BOT = 8;

const double _POI_SPEED = 0.4;
const double _POI_PRIORITY_FACTOR = 0.4; // 0.0 to 1.0

const double _PLAYER_HEAT_FACTOR = 0.1;
const double _BOT_HEAT_FACTOR = 0.3;
const double _WORLD_EDGES_HEAT_FACTOR = 0.1;
const double _ENEMY_HEAT_FACTOR = 1.0;

const double _AIM_FULL_ROTATION_SPEED_MILLISECONDS = 1000;

const double _AIM_ROTATION_RADIANS_PER_MILLISECOND =
    2 * M_PI / _AIM_FULL_ROTATION_SPEED_MILLISECONDS;

SDL_Color _BOT_COLOR = SMOCC_FOREGROUND_COLOR;

struct Aim
{
    double x;
    double y;
};

struct PointOfInterest
{
    double x;
    double y;
    double sppedX;
    double speedY;
    double targetX;
    double targetY;
};

struct Bot
{
    unsigned int index;
    unsigned long long bulletSourceID;

    double x;
    double y;
    bool active;
    bool reset;
    PointOfInterest poi;
    Aim aim;
};

void _reset();

void _activateBot(Bot& bot);
void _updateBot(Bot& bot);
void _resetBot(Bot& bot);
void _updateBotPosition(Bot& bot);
void _updateBotPointOfInterest(Bot& bot);
void _updateBotAim(Bot& bot);
pair<unsigned int, unsigned int> _findBestWaypoint(Bot& bot);
const enemies::Enemy* findBestTarget(Bot& bot);
void getDirectionToAim(
    Bot& bot, const enemies::Enemy& target, double* aimX, double* aimY
);
double getTargetPriority(Bot& bot, const enemies::Enemy& enemy);
bool _segmentIntersectsAnyEnemy(double x1, double y1, double x2, double y2);
bool _segmentIntersectsAnySelectEnemies(
    double x1, double y1, double x2, double y2,
    function<bool(const enemies::Enemy&)> selector
);

double _maxDistance;
bool _buffWasActive;

Bot _bots[BOTS_COUNT];
bool _resetDone;

void init()
{
    for (int i = 0; i < BOTS_COUNT; i++)
    {
        _bots[i].index = i;
    }

    _reset();
}

void update()
{
    if (!game::isRunning())
    {
        if (!_resetDone) _reset();
        return;
    }

    _resetDone = false;

    SDL_Window* window = smocc::getWindow();
    int ww, wh;

    SDL_GetWindowSize(window, &ww, &wh);

    _maxDistance = gfx::distance(0, 0, ww, wh);

    bool buffIsAcive = buffs::isActive(FRIENDLY_BOTS);
    bool buffTurnedActive = buffIsAcive && !_buffWasActive;
    bool buffTurnedInactive = !buffIsAcive && _buffWasActive;

    if (buffTurnedActive)
        for (int i = 0; i < BOTS_COUNT; i++)
            _activateBot(_bots[i]);

    if (buffTurnedInactive)
        for (int i = 0; i < BOTS_COUNT; i++)
            _bots[i].active = false;

    if (buffIsAcive)
    {
        for (int i = 0; i < BOTS_COUNT; i++)
        {
            if (_bots[i].active)
                _updateBot(_bots[i]);
            else if (!_bots[i].reset)
                _resetBot(_bots[i]);
        }
    }

    _buffWasActive = buffIsAcive;
}

void location(unsigned int botIndex, double* x, double* y)
{
    assert(botIndex < BOTS_COUNT);

    // Shouldn't retrieve info of an inactive bot. Bot may not be initialized.
    assert(_bots[botIndex].active);

    *x = _bots[botIndex].x;
    *y = _bots[botIndex].y;
}

void deactivate(unsigned int botIndex)
{
    assert(botIndex < BOTS_COUNT);

    _bots[botIndex].active = false;
}

void _reset()
{
    _buffWasActive = false;

    for (int i = 0; i < BOTS_COUNT; i++)
    {
        _bots[i].active = false;
        if (!_bots[i].reset) _resetBot(_bots[i]);
    }
}

void _activateBot(Bot& bot)
{
    SDL_Window* window = smocc::getWindow();
    int ww, wh;

    SDL_GetWindowSize(window, &ww, &wh);

    double aimRotationRadians = M_PI * rng::roll();

    double px = rng::roll() * ww;
    double py = rng::roll() * wh;
    double ptx = rng::roll() * ww;
    double pty = rng::roll() * wh;
    double pdx, pdy;

    gfx::direction(px, py, ptx, pty, &pdx, &pdy);

    bot.active = true;
    bot.x = player::getXPosition();
    bot.y = player::getYPosition();
    bot.poi.x = px;
    bot.poi.y = py;
    bot.poi.targetX = ptx;
    bot.poi.targetY = pty;
    bot.poi.sppedX = _POI_SPEED * pdx;
    bot.poi.speedY = _POI_SPEED * pdy;
    bot.aim.x = cos(aimRotationRadians);
    bot.aim.y = -sin(aimRotationRadians);

    bot.bulletSourceID = bullets::createSource();
}

void _updateBot(Bot& bot)
{
    bot.reset = false;

    _updateBotPosition(bot);
    _updateBotPointOfInterest(bot);
    _updateBotAim(bot);

    bullets::setSourcePosition(bot.bulletSourceID, bot.x, bot.y);
    bullets::setSourceDirection(bot.bulletSourceID, bot.aim.x, bot.aim.y);

    gfx::setDrawColor(&_BOT_COLOR);
    gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    gfx::fillCircle(bot.x, bot.y, BOT_CIRCLE_RADIUS);
}

void _resetBot(Bot& bot)
{
    bullets::deleteSource(bot.bulletSourceID);

    bot.reset = true;
}

void _updateBotPosition(Bot& bot)
{
    unsigned int deltaTimeMilliseconds = game::getDeltaTimeMilliseconds();
    pair<unsigned int, unsigned int> bestWaypoint = _findBestWaypoint(bot);

    double wx = bestWaypoint.first;
    double wy = bestWaypoint.second;
    double dx, dy;

    gfx::direction(bot.x, bot.y, wx, wy, &dx, &dy);

    double botPositionChange = BOT_SPEED * deltaTimeMilliseconds;

    if (botPositionChange > gfx::distance(bot.x, bot.y, wx, wy))
    {
        bot.x = wx;
        bot.y = wy;
    }
    else
    {
        bot.x += dx * botPositionChange;
        bot.y += dy * botPositionChange;
    }
}

void _updateBotPointOfInterest(Bot& bot)
{
    unsigned int deltaTimeMilliseconds = game::getDeltaTimeMilliseconds();

    double px = bot.poi.x;
    double py = bot.poi.y;
    double ptx = bot.poi.targetX;
    double pty = bot.poi.targetY;
    double poiPositionChange = _POI_SPEED * deltaTimeMilliseconds;

    if (poiPositionChange > gfx::distance(px, py, ptx, pty))
    {
        SDL_Window* window = smocc::getWindow();
        int ww, wh;

        SDL_GetWindowSize(window, &ww, &wh);

        px = ptx;
        py = pty;
        ptx = rng::roll() * ww;
        pty = rng::roll() * wh;

        double pdx, pdy;

        gfx::direction(px, py, ptx, pty, &pdx, &pdy);

        bot.poi.x = px;
        bot.poi.y = py;
        bot.poi.targetX = ptx;
        bot.poi.targetY = pty;
        bot.poi.sppedX = _POI_SPEED * pdx;
        bot.poi.speedY = _POI_SPEED * pdy;
    }
    else
    {
        bot.poi.x += bot.poi.sppedX * deltaTimeMilliseconds;
        bot.poi.y += bot.poi.speedY * deltaTimeMilliseconds;
    }

    // DEBUG

    // SDL_Color blue = {0, 0, 255, (Uint8)(255 * .5)};
    // gfx::setDrawColor(&blue);
    // gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    // gfx::fillCircle(bot.poi.x, bot.poi.y, 5);

    // / DEBUG
}

void _updateBotAim(Bot& bot)
{
    const enemies::Enemy* target = findBestTarget(bot);

    if (target == nullptr) return;

    double deltaTimeMilliseconds = game::getDeltaTimeMilliseconds();

    double dx, dy;   // current aim direction
    double tdx, tdy; // target aim direction

    dx = bot.aim.x;
    dy = bot.aim.y;

    getDirectionToAim(bot, *target, &tdx, &tdy);

    // 🪄 magic https://stackoverflow.com/a/3461533
    bool targetIsOverLeft = dx * tdx - dy * tdy > 0;

    double rotation = targetIsOverLeft ? -1 : 1;

    rotation *= _AIM_ROTATION_RADIANS_PER_MILLISECOND * deltaTimeMilliseconds;

    gfx::rotate(dx, dy, rotation, &dx, &dy);

    bool targetWasOverLeft = targetIsOverLeft;
    targetIsOverLeft = dx * tdx - dy * tdy > 0;

    bool sideFlipped = targetWasOverLeft != targetIsOverLeft;

    if (sideFlipped)
    {
        dx = tdx;
        dy = tdy;
    }

    bot.aim.x = dx;
    bot.aim.y = dy;
}

pair<unsigned int, unsigned int> _findBestWaypoint(Bot& bot)
{
    struct circle
    {
        double x, y, r;
    };

    double x = bot.x;
    double y = bot.y;
    double wcr = _WAYPOINT_CHECK_RADIUS;

    double coldestHeat = std::numeric_limits<double>::infinity();
    double playerX = player::getXPosition();
    double playerY = player::getYPosition();
    double playerRadius = player::PLAYER_CIRCLE_RADIUS;
    double playerDistance = gfx::distance(bot.x, bot.y, playerX, playerY);
    bool isPlayerClose = playerDistance < wcr;

    pair<unsigned int, unsigned int> bestWaypoint = {bot.x, bot.y};
    vector<circle> closeEntities;

    if (gfx::circlesOverlap(x, y, wcr, playerX, playerY, playerRadius))
        closeEntities.push_back({playerX, playerY, playerRadius});

    enemies::forEach(
        [&](auto e)
        {
            if (gfx::circlesOverlap(x, y, wcr, e.x, e.y, e.radius))
                closeEntities.push_back({e.x, e.y, e.radius});
        }
    );

    for (int i = 0; i < BOTS_COUNT; i++)
    {
        if (i == bot.index) continue;
        double bx = _bots[i].x, by = _bots[i].y, br = BOT_CIRCLE_RADIUS;
        if (gfx::circlesOverlap(x, y, wcr, bx, by, br))
            closeEntities.push_back({bx, by, br});
    }

    double waypointDistance = _WAYPOINT_CHECK_RADIUS;

    for (circle e : closeEntities)
    {
        double d = gfx::distance(x, y, e.x, e.y) - e.r;
        waypointDistance = min(waypointDistance, d);
    }

    SDL_Window* window = smocc::getWindow();
    int ww, wh;

    SDL_GetWindowSize(window, &ww, &wh);

    double edgeDistance = gfx::distancePointToRectOutline(x, y, 0, 0, ww, wh);

    waypointDistance = min(waypointDistance, edgeDistance);

    // DEBUG

    // double heatCellWidth = (double)ww / _WAYPOINT_GRID_COLUMNS;
    // double heatCellHeight = (double)wh / _WAYPOINT_GRID_ROWS;
    // double heatMap[_WAYPOINT_GRID_COLUMNS][_WAYPOINT_GRID_ROWS];
    // double maxHeat = 0;

    // for (int c = 0; c < _WAYPOINT_GRID_COLUMNS; c++)
    //     for (int r = 0; r < _WAYPOINT_GRID_ROWS; r++)
    //         heatMap[c][r] = std::numeric_limits<double>::infinity();

    // / DEBUG

    auto checkWaypoint = [&](double wx, double wy)
    {
        if (wx < 0 || wy < 0 || wx > ww || wy > wh) return;

        double distance = gfx::distance(x, y, wx, wy);

        if (distance > _WAYPOINT_CHECK_RADIUS) return;

        bool blocked = false;

        for (circle e : closeEntities)
        {
            if (gfx::segmentIntersectsCircle(x, y, wx, wy, e.x, e.y, e.r))
            {
                blocked = true;
                break;
            }
        }

        if (blocked) return;

        double heat = 0.1;

        for (circle e : closeEntities)
        {
            double d = gfx::distance(wx, wy, e.x, e.y) - e.r;
            heat += _WAYPOINT_CHECK_RADIUS / d;
        }

        double poiDist = gfx::distance(wx, wy, bot.poi.x, bot.poi.y);
        double t = poiDist / _maxDistance;
        double poiFactor = lerp(1.0 - _POI_PRIORITY_FACTOR, 1.0, t);

        heat *= poiFactor;

        // DEBUG

        // heatMap[c][r] = heat;

        // if (heat != numeric_limits<double>::infinity())
        //     maxHeat = max(maxHeat, heat);

        // / DEBUG

        if (heat < coldestHeat)
        {
            coldestHeat = heat;
            bestWaypoint = {wx, wy};
        }
    };

    for (int i = 0; i < _WAYPOINTS_PER_BOT; i++)
    {
        double t = (double)i / (double)_WAYPOINTS_PER_BOT;
        double angle = t * 2 * M_PI;
        double wx = x + waypointDistance * cos(angle);
        double wy = y + waypointDistance * -sin(angle);
        checkWaypoint(wx, wy);
    }

    // for (double i = 0; i < _WAYPOINT_CHECK_RADIUS; i++)
    //     for (double j = 0; j < _WAYPOINT_CHECK_RADIUS; j++)
    //         for (int k : {-1, 1})
    //             for (int l : {-1, 1})
    //             {
    //                 double wx = x + i * k;
    //                 double wy = y + j * l;

    //                 checkWaypoint(wx, wy);
    //             }

    // for (int c = 0; c < _WAYPOINT_GRID_COLUMNS; c++)
    //     for (int r = 0; r < _WAYPOINT_GRID_ROWS; r++)
    //     {
    //         double wx = _waypointX[c][r];
    //         double wy = _waypointY[c][r];
    //         double distance = gfx::distance(bot.x, bot.y, wx, wy);

    //         if (distance > _WAYPOINT_CHECK_RADIUS) continue;

    //         bool blocked = false;

    //         for (circle e : closeEntities)
    //         {
    //             if (gfx::segmentIntersectsCircle(x, y, wx, wy, e.x, e.y,
    //             e.r))
    //             {
    //                 blocked = true;
    //                 break;
    //             }
    //         }

    //         if (blocked) continue;

    //         double heat = 0.1;

    //         for (circle e : closeEntities)
    //         {
    //             double d = gfx::distance(wx, wy, e.x, e.y) - e.r;
    //             heat += _WAYPOINT_CHECK_RADIUS / d;
    //         }

    //         double poiDist = gfx::distance(wx, wy, bot.poi.x, bot.poi.y);
    //         double t = poiDist / _maxDistance;
    //         double poiFactor = lerp(1.0 - _POI_PRIORITY_FACTOR, 1.0, t);

    //         heat *= poiFactor;

    //         // DEBUG

    //         // heatMap[c][r] = heat;

    //         // if (heat != numeric_limits<double>::infinity())
    //         //     maxHeat = max(maxHeat, heat);

    //         // / DEBUG

    //         if (heat < coldestHeat)
    //         {
    //             coldestHeat = heat;
    //             bestWaypoint = {wx, wy};
    //         }
    //     }

    // DEBUG

    // for (int c = 0; c < _WAYPOINT_GRID_COLUMNS; c++)
    //     for (int r = 0; r < _WAYPOINT_GRID_ROWS; r++)
    //     {
    //         double wx = _waypointX[c][r];
    //         double wy = _waypointY[c][r];
    //         double alpha = 0;

    //         if (heatMap[c][r] != numeric_limits<double>::infinity())
    //             alpha = 1 - heatMap[c][r] / maxHeat;

    //         Uint8 alpha8 = (Uint8)(255 * alpha * 0.5);

    //         if (alpha8 == 0) continue;

    //         SDL_Color color = {0, 0, 0, alpha8};

    //         SDL_Rect rect = {
    //             (int)(wx - heatCellWidth / 2), (int)(wy - heatCellHeight /
    //             2), (int)heatCellWidth, (int)heatCellHeight
    //         };

    //         gfx::setDrawColor(&color);
    //         gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    //         SDL_RenderFillRect(smocc::getRenderer(), &rect);
    //     }

    // SDL_Color green = {0, 255, 0, 100};

    // gfx::setDrawColor(&green);
    // gfx::setDrawBlendMode(SDL_BLENDMODE_BLEND);
    // gfx::fillCircle(bestWaypoint.first, bestWaypoint.second, 5);

    // / DEBUG

    return bestWaypoint;
}

const enemies::Enemy* findBestTarget(Bot& bot)
{
    const enemies::Enemy* bestTarget = nullptr;
    double bestPriority = 0;

    enemies::forEach(
        [&](auto e)
        {
            double priority = getTargetPriority(bot, e);

            if (priority > bestPriority)
            {
                bestPriority = priority;
                bestTarget = &e;
            }
        }
    );

    return bestTarget;
}

void getDirectionToAim(
    Bot& bot, const enemies::Enemy& target, double* dx, double* dy
)
{
    // Finds the corrent orientation to take so that the fired bullets
    // will rendezvous with the target.

    // https://stackoverflow.com/a/2249237

    auto sqr = [](auto x) { return x * x; };

    double bulletSpeed = bullets::BULLET_SPEED;

    double a = sqr(target.xSpeed) + sqr(target.ySpeed) - sqr(bulletSpeed);
    double b = 2 * (target.xSpeed * (target.x - bot.x) +
                    target.ySpeed * (target.y - bot.y));
    double c = sqr(target.x - bot.x) + sqr(target.y - bot.y);
    double disc = sqr(b) - 4 * a * c;

    if (disc < 0)
    {
        // Projectile cann never arrive at target in time. Just aim at target.
        gfx::direction(bot.x, bot.y, target.x, target.y, dx, dy);
        return;
    }

    double t1 = (-b + sqrt(disc)) / (2 * a);
    double t2 = (-b - sqrt(disc)) / (2 * a);

    if (t1 < 0) t1 = std::numeric_limits<double>::infinity();
    if (t2 < 0) t2 = std::numeric_limits<double>::infinity();

    double t = min(t1, t2);

    double aimX = (target.x + target.xSpeed * t) - bot.x;
    double aimY = (target.y + target.ySpeed * t) - bot.y;

    gfx::unit(aimX, aimY, dx, dy);
}

double getTargetPriority(Bot& bot, const enemies::Enemy& enemy)
{
    double px = player::getXPosition();
    double py = player::getYPosition();
    double bx = bot.x;
    double by = bot.y;
    double ex = enemy.x;
    double ey = enemy.y;
    double eh = enemy.health;

    auto notEnemy = [&](auto e) { return e.id != enemy.id; };

    if (_segmentIntersectsAnySelectEnemies(bx, by, ex, ey, notEnemy)) return 0;

    double minHealth = enemies::MIN_ENEMY_HEALTH;
    double maxHealth = enemies::MAX_ENEMY_HEALTH;

    double botDistanceFactor = _maxDistance / gfx::distance(bx, by, ex, ey) / 2;
    double playerDistanceFactor = _maxDistance / gfx::distance(px, py, ex, ey);
    double distanceFactor = max(botDistanceFactor, playerDistanceFactor);
    double healthFactor = 1 - gfx::inverseLerp(minHealth, maxHealth, eh) * 0.3;

    return distanceFactor * healthFactor;
}

bool _segmentIntersectsAnyEnemy(double x1, double y1, double x2, double y2)
{
    return _segmentIntersectsAnySelectEnemies(
        x1, y1, x2, y2, [](auto _) { return true; }
    );
}

bool _segmentIntersectsAnySelectEnemies(
    double x1, double y1, double x2, double y2,
    function<bool(const enemies::Enemy&)> selector
)
{
    bool s = false;

    enemies::forEach(
        [&](auto e)
        {
            if (!selector(e)) return;

            double ex = e.x;
            double ey = e.y;
            double er = e.radius;

            s = s || gfx::segmentIntersectsCircle(x1, y1, x2, y2, ex, ey, er);
        }
    );

    return s;
}

} // namespace smocc::bots
