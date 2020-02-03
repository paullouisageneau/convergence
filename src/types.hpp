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

#include <limits>

namespace convergence {

#pragma pack(push, 1)
template <typename T> struct integer3 {
	T x, y, z;

	struct hash {
		std::size_t operator()(const integer3<T> &p) const noexcept {
			std::size_t seed = 0;
			hash_combine(seed, p.x);
			hash_combine(seed, p.y);
			hash_combine(seed, p.z);
			return seed;
		}
	};

	integer3(T _x = 0, T _y = 0, T _z = 0);
	integer3(const vec3 &v);

	bool operator==(const integer3<T> &i) const;
	bool operator!=(const integer3<T> &i) const;
	integer3<T> operator-() const;
	integer3<T> operator+(const integer3<T> &i) const;
	integer3<T> operator-(const integer3<T> &i) const;
	integer3<T> operator*(int i) const;
	integer3<T> operator/(int i) const;
	integer3<T> operator*(float f) const;
	integer3<T> operator/(float f) const;

	integer3<T> &operator+=(const integer3<T> &i) { return *this = *this + i; }
	integer3<T> &operator-=(const integer3<T> &i) { return *this = *this - i; }
	integer3<T> &operator*=(int i) { return *this = *this * i; }
	integer3<T> &operator/=(int i) { return *this = *this / i; }
	integer3<T> &operator*=(float f) { return *this = *this * f; }
	integer3<T> &operator/=(float f) { return *this = *this / f; }

	operator vec3() const;

	integer3<T> normalize() const;
};

template <typename T> struct integer4 {
	T x, y, z, w;

	struct hash {
		std::size_t operator()(const integer4<T> &p) const noexcept {
			std::size_t seed = 0;
			hash_combine(seed, p.x);
			hash_combine(seed, p.y);
			hash_combine(seed, p.z);
			hash_combine(seed, p.w);
			return seed;
		}
	};

	integer4(T _x = 0, T _y = 0, T _z = 0, T _w = 0);
	integer4(const integer3<T> &i, T _w = 0);
	integer4(const vec4 &v);

	bool operator==(const integer4<T> &i) const;
	bool operator!=(const integer4<T> &i) const;
	integer4<T> operator-() const;
	integer4<T> operator+(const integer4<T> &i) const;
	integer4<T> operator-(const integer4<T> &i) const;
	integer4<T> operator*(int i) const;
	integer4<T> operator/(int i) const;
	integer4<T> operator*(float f) const;
	integer4<T> operator/(float f) const;

	integer4<T> &operator+=(const integer4<T> &i) { return *this = *this + i; }
	integer4<T> &operator-=(const integer4<T> &i) { return *this = *this - i; }
	integer4<T> &operator*=(int i) { return *this = *this * i; }
	integer4<T> &operator/=(int i) { return *this = *this / i; }
	integer4<T> &operator*=(float f) { return *this = *this * f; }
	integer4<T> &operator/=(float f) { return *this = *this / f; }

	operator integer3<T>() const;
	operator vec4() const;

	integer4<T> normalize() const;
};
#pragma pack(pop)

using int3 = integer3<int>;
using uint3 = integer3<unsigned int>;
using int4 = integer4<int>;
using uint4 = integer3<unsigned int>;
using int84 = integer4<int8_t>;
using uint84 = integer4<uint8_t>;

template <typename T> integer3<T>::integer3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

template <typename T>
integer3<T>::integer3(const vec3 &v)
    : x(int(std::floor(double(v.x)))), y(int(std::floor(double(v.y)))),
      z(int(std::floor(double(v.z)))) {}

template <typename T> bool integer3<T>::operator==(const integer3<T> &i) const {
	return (x == i.x && y == i.y && z == i.z);
}

template <typename T> bool integer3<T>::operator!=(const integer3<T> &i) const {
	return (x != i.x || y != i.y || z != i.z);
}

template <typename T> integer3<T> integer3<T>::operator-() const { return {T(-x), T(-y), T(-z)}; }

template <typename T> integer3<T> integer3<T>::operator+(const integer3<T> &i) const {
	return {T(x + i.x), T(y + i.y), T(z + i.z)};
}

template <typename T> integer3<T> integer3<T>::operator-(const integer3<T> &i) const {
	return {T(x - i.x), T(y - i.y), T(z - i.z)};
}

template <typename T> integer3<T> integer3<T>::operator*(int i) const {
	return {T(x * i), T(y * i), T(z * i)};
}

template <typename T> integer3<T> integer3<T>::operator/(int i) const {
	return {T(x / i), T(y / i), T(z / i)};
}

template <typename T> integer3<T> integer3<T>::operator*(float f) const {
	return vec3(x, y, z) * f;
}

template <typename T> integer3<T> integer3<T>::operator/(float f) const {
	return vec3(x, y, z) / f;
}

template <typename T> integer3<T>::operator vec3() const { return vec3(x, y, z); }

template <typename T> integer3<T> integer3<T>::normalize() const {
	const int ix(x);
	const int iy(y);
	const int iz(z);
	const int norm = (ix * ix + iy * iy + iz * iz);
	if (norm) {
		const float inv = float(std::numeric_limits<T>::max()) / std::sqrt(float(norm));
		return {T(ix * inv), T(iy * inv), T(iz * inv)};
	} else
		return *this;
}

template <typename T> integer4<T>::integer4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}

template <typename T>
integer4<T>::integer4(const integer3<T> &i, T _w) : x(i.x), y(i.y), z(i.z), w(_w) {}

template <typename T>
integer4<T>::integer4(const vec4 &v)
    : x(int(std::floor(double(v.x)))), y(int(std::floor(double(v.y)))),
      z(int(std::floor(double(v.z)))), w(int(std::floor(double(v.w)))) {}

template <typename T> bool integer4<T>::operator==(const integer4<T> &i) const {
	return (x == i.x && y == i.y && z == i.z && w == i.w);
}

template <typename T> bool integer4<T>::operator!=(const integer4<T> &i) const {
	return (x != i.x || y != i.y || z != i.z || w != i.w);
}

template <typename T> integer4<T> integer4<T>::operator-() const {
	return {T(-x), T(-y), T(-z), T(-w)};
}

template <typename T> integer4<T> integer4<T>::operator+(const integer4<T> &i) const {
	return {T(x + i.x), T(y + i.y), T(z + i.z), T(w + i.w)};
}

template <typename T> integer4<T> integer4<T>::operator-(const integer4<T> &i) const {
	return {T(x - i.x), T(y - i.y), T(z - i.z), T(w - i.w)};
}

template <typename T> integer4<T> integer4<T>::operator*(int i) const {
	return {T(x * i), T(y * i), T(z * i), T(w * i)};
}

template <typename T> integer4<T> integer4<T>::operator/(int i) const {
	return {T(x / i), T(y / i), T(z / i)};
}

template <typename T> integer4<T> integer4<T>::operator*(float f) const {
	return vec4(x, y, z, w) * f;
}

template <typename T> integer4<T> integer4<T>::operator/(float f) const {
	return vec4(x, y, z, w) / f;
}

template <typename T> integer4<T>::operator integer3<T>() const { return {x, y, z}; }

template <typename T> integer4<T>::operator vec4() const { return vec4(x, y, z, w); }

template <typename T> integer4<T> integer4<T>::normalize() const {
	const int ix(x);
	const int iy(y);
	const int iz(z);
	const int iw(w);
	const int norm = (ix * ix + iy * iy + iz * iz + iw * iw);
	if (norm) {
		const float inv = float(std::numeric_limits<T>::max()) / std::sqrt(float(norm));
		return {T(ix * inv), T(iy * inv), T(iz * inv), T(iw * inv)};
	} else
		return *this;
}

} // namespace convergence

#endif
