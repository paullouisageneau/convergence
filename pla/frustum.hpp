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

#ifndef PLA_FRUSTUM_H
#define PLA_FRUSTUM_H

#include "pla/include.hpp"
#include "pla/linalg.hpp"

namespace pla {

// View frustum
class Frustum {
public:
	Frustum(const mat4 &proj);
	~Frustum(void);

	bool testPoint(const vec3 &p) const;
	bool testSphere(const vec3 &center, float radius) const;
	bool testBox(const vec3 &p1, const vec3 &p2) const;

private:
	float planeDistance(int plane, const vec3 &p) const; // relative distance

	vec4 mPlane[6];
};

} // namespace pla

#endif
