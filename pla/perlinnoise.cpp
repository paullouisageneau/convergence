/***************************************************************************
 *   Copyright (C) 2006-2016 by Paul-Louis Ageneau                         *
 *   paul-louis (at) ageneau (dot) org                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "pla/perlinnoise.hpp"

#include <iostream>
#include <random>
#include <algorithm>

namespace pla
{

PerlinNoise::PerlinNoise(unsigned int seed)
{
	// Compute permutation vector from seed
	const unsigned int m = 2147483648;
	const unsigned int a = 1103515245;
	const unsigned int c = 12345;
	p.resize(256);
	std::iota(p.begin(), p.end(), 0);
	for(unsigned int i = 255; i > 0; --i) 
	{
		seed = (seed*a + c) % m;
		std::swap(p[i], p[seed % (i+1)]);
	}
	
	p.insert(p.end(), p.begin(), p.end());
}

double PerlinNoise::noise(double x, double y, double z) const
{
	// Find the unit cube that contains the point
	int X = int(floor(x)) & 0xFF;
	int Y = int(floor(y)) & 0xFF;
	int Z = int(floor(z)) & 0xFF;

	// Find relative x, y, z of point in cube
	x-= floor(x);
	y-= floor(y);
	z-= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x-1, y, z)), lerp(u, grad(p[AB], x, y-1, z), grad(p[BB], x-1, y-1, z))),	lerp(v, lerp(u, grad(p[AA+1], x, y, z-1), grad(p[BA+1], x-1, y, z-1)), lerp(u, grad(p[AB+1], x, y-1, z-1),	grad(p[BB+1], x-1, y-1, z-1))));
	return (res + 1.0)/2.0;
}

double PerlinNoise::fade(double t) const
{ 
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) const
{ 
	return a + t * (b - a); 
}

double PerlinNoise::grad(int hash, double x, double y, double z) const
{
	int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ? x : y,
		   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

}
