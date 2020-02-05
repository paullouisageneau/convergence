/***************************************************************************
 *   Copyright (C) 2015-2020 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_VOLUME_H
#define CONVERGENCE_VOLUME_H

#include "src/include.hpp"
#include "src/types.hpp"

#include "pla/collidable.hpp"
#include "pla/context.hpp"
#include "pla/mesh.hpp"

#include <vector>

using pla::Context;
using pla::Mesh;

namespace convergence {

class Volume : public Mesh {
public:
	Volume();
	virtual ~Volume();

	struct Material {
		uint84 ambient;
		uint84 diffuse;
		uint8_t smoothness;

		Material operator+(const Material &m) const;
		Material operator*(float f) const;
	};

	int polygonize(const vec3 &position, const uint8_t *weights, const Material *materials,
	               const int3 &size);

protected:
	struct Attribs {
		vec3 vert;
		int84 grad;
		Material mat;

		Attribs operator+(const Attribs &m) const;
		Attribs operator*(float f) const;
	};

	template <typename T> inline static T interpolate(const T &a, const T &b, float t) {
		return a * (1.f - t) + b * t;
	}

	inline static size_t getIndex(const int3 &pos, const int3 &size) {
		return (pos.x * size.y + pos.y) * size.z + pos.z;
	}

private:
	struct GeometryArrays {
		std::vector<vec3> vertices;
		std::vector<int84> normals;
		std::vector<uint84> ambient;
		std::vector<uint84> diffuse;
		std::vector<uint8_t> smoothness;
		std::vector<index_t> indices;
	};

	virtual void computeGradients(const uint8_t *weights, int84 *grads, const int3 &size);
	virtual Attribs interpolateAttribs(const Attribs &a, const Attribs &b, uint8_t wa, uint8_t wb);

	int polygonizeCell(const int3 &c, const vec3 &position, const uint8_t *weights,
	                   const int84 *grads, const Material *mats, const int3 &size,
	                   GeometryArrays &arrays);

	static uint16_t EdgeTable[256];
	static int8_t TriangleTable[256][16];
};

} // namespace convergence

#endif
