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

#include "pla/mesh.hpp"

#include <vector>

using pla::Mesh;

namespace convergence {

class Volume : public Mesh {
public:
	struct Material {
		uint84 ambient;
		uint84 diffuse;
		uint8_t smoothness;

		Material operator+(const Material &m) const;
		Material operator*(float f) const;
	};

	Volume(int3 size, float scale);
	Volume(int3 size, float scale, const uint8_t *weights, const Material *mats,
	       const vec3 &pos = vec3(0.f, 0.f, 0.f));
	virtual ~Volume();

	int polygonize(const uint8_t *weights, const Material *mats,
	               const vec3 &pos = vec3(0.f, 0.f, 0.f));

protected:
	struct Attribs {
		vec3 vert;
		int84 grad;
		Material mat;

		Attribs operator+(const Attribs &m) const;
		Attribs operator*(float f) const;
	};

	virtual void computeGradients(const uint8_t *weights, int84 *grads);
	virtual Attribs interpolateAttribs(const Attribs &a, const Attribs &b, uint8_t wa, uint8_t wb);

	inline size_t getIndex(const int3 &pos) { return (pos.x * mSize.y + pos.y) * mSize.z + pos.z; }

	int3 mSize;
	float mScale;

private:
	struct GeometryArrays {
		std::vector<vec3> vertices;
		std::vector<int84> normals;
		std::vector<uint84> ambient;
		std::vector<uint84> diffuse;
		std::vector<uint8_t> smoothness;
		std::vector<index_t> indices;
	};
	int polygonizeCell(const int3 &c, const uint8_t *weights, const int84 *grads,
	                   const Material *mats, const vec3 &pos, GeometryArrays &arrays);

	template <typename T> inline static T interpolate(const T &a, const T &b, float t) {
		return a * (1.f - t) + b * t;
	}

	static uint16_t EdgeTable[256];
	static int8_t TriangleTable[256][16];
};

} // namespace convergence

#endif
