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

const unsigned int _WAYPOINT_GRID_COLUMNS = smocc::WINDOW_WIDTH / 10;
const unsigned int _WAYPOINT_GRID_ROWS = smocc::WINDOW_HEIGHT / 10;

const double _POI_SPEED = 0.1;
const double _POI_PRIORITY_FACTOR = 0.4; // 0.0 to 1.0

const double _PLAYER_HEAT_FACTOR = 0.1;
const double _BOT_HEAT_FACTOR = 0.3;
const double _WORLD_EDGES_HEAT_FACTOR = 0.5;
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

void _updateWaypoints();
void _updateHeatMap();
void _updateHeatPoint(unsigned int col, unsigned int row);
double _getPlayerHeat(double x, double y);
double _getWorldEdgesHeat(double x, double y);
double _getBotHeat(double x, double y, unsigned int botIndex);
double _getEnemyHeat(double x, double y, const enemies::Enemy& enemy);
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

// Heatmap used to determine the best waypoint to go to. Cooler (lower value)
// is better.
double _heatMap[_WAYPOINT_GRID_COLUMNS][_WAYPOINT_GRID_ROWS];
double _waypointX[_WAYPOINT_GRID_COLUMNS][_WAYPOINT_GRID_ROWS];
double _waypointY[_WAYPOINT_GRID_COLUMNS][_WAYPOINT_GRID_ROWS];

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
        _updateWaypoints();
        _updateHeatMap();

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

void _updateWaypoints()
{
    SDL_Window* window = smocc::getWindow();
    int ww, wh;

    SDL_GetWindowSize(window, &ww, &wh);

    for (int c = 0; c < _WAYPOINT_GRID_COLUMNS; c++)
        for (int r = 0; r < _WAYPOINT_GRID_ROWS; r++)
        {
            double tx = (0.5 + (double)c) / _WAYPOINT_GRID_COLUMNS;
            double ty = (0.5 + (double)r) / _WAYPOINT_GRID_ROWS;

            _waypointX[c][r] = tx * ww;
            _waypointY[c][r] = ty * wh;
        }
}

void _updateHeatMap()
{
    for (int c = 0; c < _WAYPOINT_GRID_COLUMNS; c++)
        for (int r = 0; r < _WAYPOINT_GRID_ROWS; r++)
            _updateHeatPoint(c, r);
}

void _updateHeatPoint(unsigned int col, unsigned int row)
{
    double x = _waypointX[col][row];
    double y = _waypointY[col][row];

    double* heat = &_heatMap[col][row];

    *heat = 0;

    double playerX = player::getXPosition();
    double playerY = player::getYPosition();
    double playerDistance = gfx::distance(x, y, playerX, playerY);
    double playerHeat = playerDistance / _maxDistance;

    *heat += _getPlayerHeat(x, y);
    *heat += _getWorldEdgesHeat(x, y);

    enemies::forEach([&](auto e) { *heat += _getEnemyHeat(x, y, e); });
}

double _getPlayerHeat(double x, double y)
{
    double playerX = player::getXPosition();
    double playerY = player::getYPosition();
    double distance = gfx::distance(x, y, playerX, playerY);

    return _PLAYER_HEAT_FACTOR * _maxDistance / distance;
}

double _getBotHeat(double x, double y, unsigned int botIndex)
{
    if (!_bots[botIndex].active) return 0;

    double botX = _bots[botIndex].x;
    double botY = _bots[botIndex].y;
    double distance = gfx::distance(x, y, botX, botY);

    return _BOT_HEAT_FACTOR * _maxDistance / distance;
}

double _getWorldEdgesHeat(double x, double y)
{
    SDL_Window* window = smocc::getWindow();
    int ww, wh;

    SDL_GetWindowSize(window, &ww, &wh);

    double maxEdgeDistance = min(ww, wh) / 2;
    double distance = gfx::distancePointToRectOutline(x, y, 0, 0, ww, wh);

    return _WORLD_EDGES_HEAT_FACTOR * _maxDistance / distance;
}

double _getEnemyHeat(double x, double y, const enemies::Enemy& enemy)
{
    double d = gfx::distance(x, y, enemy.x, enemy.y);

    if (d < enemy.radius) return std::numeric_limits<double>::infinity();

    double h = enemy.health;
    double s = enemy.speed;

    double distanceFactor = _maxDistance / d;
    double healthFactor = lerp(1.0, 10.0, h / enemies::MAX_ENEMY_HEALTH);
    double speedFactor = lerp(1.0, 10.0, s / enemies::MAX_ENEMY_SPEED);

    double base = distanceFactor * healthFactor * speedFactor;
    double power = max(1.0, 1 + 10 * speedFactor - 10);

    return _ENEMY_HEAT_FACTOR * pow(base, power);
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
    double coldestHeat = std::numeric_limits<double>::infinity();

    pair<unsigned int, unsigned int> bestWaypoint = {bot.x, bot.y};

    for (int c = 0; c < _WAYPOINT_GRID_COLUMNS; c++)
        for (int r = 0; r < _WAYPOINT_GRID_ROWS; r++)
        {
            double wx = _waypointX[c][r];
            double wy = _waypointY[c][r];

            if (_segmentIntersectsAnyEnemy(bot.x, bot.y, wx, wy)) continue;

            double heat = _heatMap[c][r];

            for (int i = 0; i < BOTS_COUNT; i++)
                if (i != bot.index) heat += _getBotHeat(wx, wy, i);

            double poiDist = gfx::distance(bot.x, bot.y, bot.poi.x, bot.poi.y);
            double t = 1 - (poiDist / _maxDistance);
            double minFactor = 1.0 - _POI_PRIORITY_FACTOR;
            double maxFactor = 1.0;
            double factor = lerp(minFactor, maxFactor, t);

            heat *= factor;

            if (heat < coldestHeat)
            {
                coldestHeat = heat;
                bestWaypoint = {c, r};
            }
        }

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
