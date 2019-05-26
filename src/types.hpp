/***************************************************************************
 *   Copyright (C) 2015-2019 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_TYPES_H
#define CONVERGENCE_TYPES_H

#include "src/include.hpp"

namespace convergence {

struct int3 {
	int3(int _x = 0, int _y = 0, int _z = 0);
	int3(const vec3 &v);

	bool operator==(const int3 &i) const;
	bool operator!=(const int3 &i) const;
	int3 operator-(void) const;
	int3 operator+(const int3 &i) const;
	int3 operator-(const int3 &i) const;
	int3 operator*(int i) const;
	int3 operator/(int i) const;

	int3 &operator+=(const int3 &i) { return *this = *this + i; }
	int3 &operator-=(const int3 &i) { return *this = *this - i; }
	int3 &operator*=(int i) { return *this = *this * i; }
	int3 &operator/=(int i) { return *this = *this / i; }

	int x, y, z;
};

struct int3_hash {
	std::size_t operator()(const int3 &p) const noexcept {
		std::size_t seed = 0;
		hash_combine(seed, p.x);
		hash_combine(seed, p.y);
		hash_combine(seed, p.z);
		return seed;
	}
};

struct int4 {
	int4(int _x = 0, int _y = 0, int _z = 0, int _w = 0);
	int4(const int3 &i, int _w = 0);
	int4(const vec4 &v);

	bool operator==(const int4 &i) const;
	bool operator!=(const int4 &i) const;
	int4 operator-(void) const;
	int4 operator+(const int4 &i) const;
	int4 operator-(const int4 &i) const;
	int4 operator*(int i) const;
	int4 operator/(int i) const;

	operator int3(void) const;

	int4 &operator+=(const int4 &i) { return *this = *this + i; }
	int4 &operator-=(const int4 &i) { return *this = *this - i; }
	int4 &operator*=(int i) { return *this = *this * i; }
	int4 &operator/=(int i) { return *this = *this / i; }

	int x, y, z, w;
};

struct int4_hash {
	std::size_t operator()(const int4 &p) const noexcept {
		std::size_t seed = 0;
		hash_combine(seed, p.x);
		hash_combine(seed, p.y);
		hash_combine(seed, p.z);
		hash_combine(seed, p.w);
		return seed;
	}
};

#pragma pack(push, 4) // Align on 4 bytes
struct int84 {
	int84(int8_t _x = 0, int8_t _y = 0, int8_t _z = 0, int8_t _w = 0);
	int84(const vec4 &v);
	int84(const vec3 &v);

	int84 normalize(void) const;

	bool operator==(const int84 &i) const;
	bool operator!=(const int84 &i) const;
	int84 operator-(void) const;
	int84 operator+(const int84 &i) const;
	int84 operator-(const int84 &i) const;
	int84 operator*(int i) const;
	int84 operator/(int i) const;

	int84 &operator+=(const int84 &i) { return *this = *this + i; }
	int84 &operator-=(const int84 &i) { return *this = *this - i; }
	int84 &operator*=(int i) { return *this = *this * i; }
	int84 &operator/=(int i) { return *this = *this / i; }

	int8_t x, y, z, w;
};
#pragma pack(pop)

} // namespace convergence

#endif
