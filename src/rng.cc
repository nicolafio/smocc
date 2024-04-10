/*

rng.cc: Random number generation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of the SMOCC, licensed under the terms of the GNU General
Public License 3.0.

*/

#include <random>

using namespace std;

random_device _rd;
mt19937 _gen(_rd());
uniform_real_distribution<double> _rng(0.0, 1.0);

namespace smocc::rng
{

double roll()
{
    return _rng(_gen);
}

}