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

#ifndef PLA_PERLIN_H
#define PLA_PERLIN_H

#include "pla/include.hpp"
#include "pla/linalg.hpp"

#include <vector>

namespace pla {

class PerlinNoise {
public:
	PerlinNoise(unsigned int seed, int period = 256);
	double generate(const dvec3 &v, int octaves = 1) const;

private:
	double noise(double x, double y, double z, int m) const;
	double fade(double t) const;
	double lerp(double t, double a, double b) const;
	double grad(int hash, double x, double y, double z) const;
	int hash(int i) const;

	const int mPeriod;
	const std::vector<int> mPerm;
};

} // namespace pla

#endif
