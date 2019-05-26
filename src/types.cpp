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

#include "src/types.hpp"

namespace convergence {

int3::int3(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}

int3::int3(const vec3 &v)
    : x(int(std::floor(double(v.x)))), y(int(std::floor(double(v.y)))),
      z(int(std::floor(double(v.z)))) {}

bool int3::operator==(const int3 &i) const { return (x == i.x && y == i.y && z == i.z); }

bool int3::operator!=(const int3 &i) const { return (x != i.x || y != i.y || z != i.z); }

int3 int3::operator-(void) const { return int3(-x, -y, -z); }

int3 int3::operator+(const int3 &i) const { return int3(x + i.x, y + i.y, z + i.z); }

int3 int3::operator-(const int3 &i) const { return int3(x - i.x, y - i.y, z - i.z); }

int3 int3::operator*(int i) const { return int3(x * i, y * i, z * i); }

int3 int3::operator/(int i) const { return int3(x / i, y / i, z / i); }

int3::operator vec3(void) const { return vec3(x, y, z); }

int4::int4(int _x, int _y, int _z, int _w) : x(_x), y(_y), z(_z), w(_w) {}

int4::int4(const int3 &i, int _w) : x(i.x), y(i.y), z(i.z), w(_w) {}

int4::int4(const vec4 &v)
    : x(int(std::floor(double(v.x)))), y(int(std::floor(double(v.y)))),
      z(int(std::floor(double(v.z)))), w(int(std::floor(double(v.w)))) {}

bool int4::operator==(const int4 &i) const {
	return (x == i.x && y == i.y && z == i.z && w == i.w);
}

bool int4::operator!=(const int4 &i) const {
	return (x != i.x || y != i.y || z != i.z || w != i.w);
}

int4 int4::operator-(void) const { return int4(-x, -y, -z, -w); }

int4 int4::operator+(const int4 &i) const { return int4(x + i.x, y + i.y, z + i.z, w + i.w); }

int4 int4::operator-(const int4 &i) const { return int4(x - i.x, y - i.y, z - i.z, w - i.w); }

int4 int4::operator*(int i) const { return int4(x * i, y * i, z * i, w * i); }

int4 int4::operator/(int i) const { return int3(x / i, y / i, z / i); }

int4::operator int3(void) const { return int3(x, y, z); }

int4::operator vec4(void) const { return vec4(x, y, z, w); }

int84::int84(int8_t _x, int8_t _y, int8_t _z, int8_t _w) : x(_x), y(_y), z(_z), w(_w) {}

int84::int84(const vec4 &v)
    : x(int8_t(std::floor(v.x))), y(int8_t(std::floor(v.y))), z(int8_t(std::floor(v.z))),
      w(int8_t(std::floor(v.w))) {}

int84::int84(const vec3 &v)
    : x(int8_t(std::floor(v.x))), y(int8_t(std::floor(v.y))), z(int8_t(std::floor(v.z))), w(0) {}

int84 int84::normalize(void) const {
	const int ix(x);
	const int iy(y);
	const int iz(z);
	const int iw(w);
	const int norm = (ix * ix + iy * iy + iz * iz + iw * iw);
	if (norm) {
		const float inv = 127.f / std::sqrt(float(norm));
		return int84(int(ix * inv), int(iy * inv), int(iz * inv), int(iw * inv));
	} else
		return *this;
}

bool int84::operator==(const int84 &i) const {
	return (x == i.x && y == i.y && z == i.z && w == i.w);
}

bool int84::operator!=(const int84 &i) const {
	return (x != i.x || y != i.y || z != i.z || w != i.w);
}

int84 int84::operator-(void) const { return int84(-x, -y, -z, -w); }

int84 int84::operator+(const int84 &i) const { return int84(x + i.x, y + i.y, z + i.z, w + i.w); }

int84 int84::operator-(const int84 &i) const { return int84(x - i.x, y - i.y, z - i.z, w - i.w); }

int84 int84::operator*(int i) const { return int84(x * i, y * i, z * i, w * i); }

int84 int84::operator/(int i) const { return int84(x / i, y / i, z / i); }

int84::operator vec4(void) const { return vec4(x, y, z, w); }

} // namespace convergence

