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

#include "p3d/include.hpp"

namespace pla
{

// Intersection subroutines
float intersectPlane(const vec3 &p, const vec3 &direction, const vec3 &planeOrigin, const vec3 &planeNormal);
float intersectSphere(const vec3 &p, vec3 direction, const vec3 &centre, float radius);
bool isInTriangle(const vec3 &p, const vec3 &p1, const vec3 &p2, const vec3 &p3);
vec3 closestPointOnTriangle(const vec3 &p, const vec3 &p1,const vec3 &p2, const vec3 &p3);
vec3 closestPointOnSegment(const vec3 &p, const vec3 &p1, const vec3 &p2);
float intersectBox(const vec3 &p, const vec3 &move, float radius, const vec3 &min, const vec3 &max, vec3 *intersection);
float intersectFace(const vec3 &p, const vec3 &move, float radius, const vec3 &p1, const vec3 &p2, const vec3 &p3, const vec3 &p4, vec3 *intersection);
float intersectFace(const vec3 &p, const vec3 &move, float radius, const vec3 &p1, const vec3 &p2, const vec3 &p3, vec3 *intersection);

}
