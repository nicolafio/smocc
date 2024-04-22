/*

rng.cc: Random number generation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <cmath>
#include <random>

using namespace std;

namespace smocc::rng
{

random_device _rd;
mt19937 _gen(_rd());
uniform_real_distribution<double> _rng(0.0, 1.0);

double roll()
{
    return _rng(_gen);
}

int rollInt(int min, int max)
{
    double r = roll();
    int range = max - min + 1;
    int i = min + (int)(r * range);
    return std::min(i, max);
}

} // namespace smocc::rng