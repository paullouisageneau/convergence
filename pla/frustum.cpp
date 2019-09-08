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

#include "pla/frustum.hpp"

namespace pla {

Frustum::Frustum(const mat4 &proj) {
	// Right plane
	mPlane[0] = vec4(proj[0][3] - proj[0][0], proj[1][3] - proj[1][0], proj[2][3] - proj[2][0],
	                 proj[3][3] - proj[3][0]);

	// Left plane
	mPlane[1] = vec4(proj[0][3] + proj[0][0], proj[1][3] + proj[1][0], proj[2][3] + proj[2][0],
	                 proj[3][3] + proj[3][0]);

	// Bottom plane
	mPlane[2] = vec4(proj[0][3] + proj[0][1], proj[1][3] + proj[1][1], proj[2][3] + proj[2][1],
	                 proj[3][3] + proj[3][1]);

	// Top plane
	mPlane[3] = vec4(proj[0][3] - proj[0][1], proj[1][3] - proj[1][1], proj[2][3] - proj[2][1],
	                 proj[3][3] - proj[3][1]);

	// Far plane
	mPlane[4] = vec4(proj[0][3] - proj[0][2], proj[1][3] - proj[1][2], proj[2][3] - proj[2][2],
	                 proj[3][3] - proj[3][2]);

	// Near plane
	mPlane[5] = vec4(proj[0][3] + proj[0][2], proj[1][3] + proj[1][2], proj[2][3] + proj[2][2],
	                 proj[3][3] + proj[3][2]);

	// Normalize
	for (int i = 0; i < 6; ++i)
		mPlane[i] /= glm::length(vec3(mPlane[i]));
}

Frustum::~Frustum(void) {}

bool Frustum::testPoint(const vec3 &p) const {
	for (int i = 0; i < 6; ++i)
		if (planeDistance(i, p) <= 0.f)
			return false;

	return true;
}

bool Frustum::testSphere(const vec3 &center, float radius) const {
	for (int i = 0; i < 6; ++i)
		if (planeDistance(i, center) <= -radius)
			return false;

	return true;
}

bool Frustum::testBox(const vec3 &p1, const vec3 &p2) const {
	for (int i = 0; i < 6; ++i) {
		if (planeDistance(i, vec3(p2.x, p2.y, p2.z)) > 0.f)
			continue;
		if (planeDistance(i, vec3(p1.x, p2.y, p2.z)) > 0.f)
			continue;
		if (planeDistance(i, vec3(p2.x, p1.y, p2.z)) > 0.f)
			continue;
		if (planeDistance(i, vec3(p1.x, p1.y, p2.z)) > 0.f)
			continue;
		if (planeDistance(i, vec3(p2.x, p2.y, p1.z)) > 0.f)
			continue;
		if (planeDistance(i, vec3(p1.x, p2.y, p1.z)) > 0.f)
			continue;
		if (planeDistance(i, vec3(p2.x, p1.y, p1.z)) > 0.f)
			continue;
		if (planeDistance(i, vec3(p1.x, p1.y, p1.z)) > 0.f)
			continue;
		return false;
	}

	return true;
}

float Frustum::planeDistance(int plane, const vec3 &p) const {
	return glm::dot(mPlane[plane], vec4(p, 1.f));
}

} // namespace pla
