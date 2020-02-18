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

#include <algorithm>
#include <iostream>
#include <random>

namespace pla {

using pla::bounds;
using std::pow;

// Compute permutation vector from seed
std::vector<int> make_perm(unsigned int seed, int period) {
	std::vector<int> p(period);
	std::iota(p.begin(), p.end(), 0);

	// Compute permutation from seed
	const unsigned int m = 2147483648;
	const unsigned int a = 1103515245;
	const unsigned int c = 12345;
	for (unsigned int i = period - 1; i > 0; --i) {
		seed = (seed * a + c) % m;
		unsigned int r = seed / (m / (i + 1) + 1);
		std::swap(p[i], p[r]);
	}
	return p;
}

PerlinNoise::PerlinNoise(unsigned int seed, int period)
    : mPeriod(period), mPerm(make_perm(seed, period)) {}

double PerlinNoise::generate(const dvec3 &v, int octaves) const {
	int pw = 1;
	double value = 0.;
	double norm = 0.;
	for (int i = 0; i < octaves; ++i) {
		const double n = 0.5 / pw;
		value += n * noise(v.x * pw, v.y * pw, v.z * pw, mPeriod * pw);
		pw *= 2;
		norm += n;
	}
	return norm > 0. ? value / norm : 0.;
}

double PerlinNoise::noise(double x, double y, double z, int m) const {
	// Find the unit cube that contains the point
	int X = floor(x);
	int Y = floor(y);
	int Z = floor(z);

	// Find relative x, y, z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = hash(X) + Y;
	int AA = hash(A) + Z;
	int AB = hash(A + 1) + Z;
	int B = hash(X + 1) + Y;
	int BA = hash(B) + Z;
	int BB = hash(B + 1) + Z;

	// Add blended results from 8 corners of cube
	double res =
	    lerp(w,
	         lerp(v, lerp(u, grad(hash(AA), x, y, z), grad(hash(BA), x - 1, y, z)),
	              lerp(u, grad(hash(AB), x, y - 1, z), grad(hash(BB), x - 1, y - 1, z))),
	         lerp(v, lerp(u, grad(hash(AA + 1), x, y, z - 1), grad(hash(BA + 1), x - 1, y, z - 1)),
	              lerp(u, grad(hash(AB + 1), x, y - 1, z - 1),
	                   grad(hash(BB + 1), x - 1, y - 1, z - 1))));
	return (res + 1.) / 2.;
}

double PerlinNoise::fade(double t) const { return t * t * t * (t * (t * 6 - 15) + 10); }

double PerlinNoise::lerp(double t, double a, double b) const { return a + t * (b - a); }

double PerlinNoise::grad(int h, double x, double y, double z) const {
	h = h & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

int PerlinNoise::hash(int i) const {
	// Circumvent modulo implementation for negative values with an offset
	const unsigned offset = 0x80000000;
	return mPerm[unsigned(offset + i) % mPeriod];
}

} // namespace pla
