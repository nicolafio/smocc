/*

rng.h: Random number generation for SMOCC

Copyright (C) 2024 Nicola Fiori (JD342)

This file is part of SMOCC, licensed under GNU General Public License 3.0.

*/

namespace smocc::rng
{

// Rolls a random number between 0 and 1.
double roll();

int rollInt(int min, int max);

} // namespace smocc::rng