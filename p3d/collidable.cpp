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

#include "p3d/collidable.hpp"
#include "p3d/intersection.hpp"

namespace pla
{

bool Collidable::collide(const vec3 &pos, const vec3 &move, float radius, vec3 *result, vec3 *intersection, vec3 *normal, int reclevel)
{
	if(reclevel <= 0) return false;

	vec3 tmpIntersection;
	float t1 = intersect(pos, move, radius, &tmpIntersection);

	// If collision
	if(t1 <= 1.f)
	{
		if(intersection) *intersection = tmpIntersection;
		if(normal) *normal = glm::normalize(pos + move*t1 - tmpIntersection);
		if(!result) return true;	// no further computation expected

		vec3 v = move*(t1 - Epsilon);

		// If we already are going through
		float d = glm::distance(pos, tmpIntersection);
		if(d < radius)
		{
			vec3 n = glm::normalize(pos-tmpIntersection);
			v+= n*(radius - d)*(1.f + Epsilon);	// fix position
		}

		// Sliding plane
		vec3 slideOrigin = tmpIntersection;
		vec3 slideNormal = tmpIntersection-pos;

		// Project destination on sliding plane
		vec3 destination = pos+move;
		float t2 = intersectPlane(destination, slideNormal, slideOrigin, slideNormal);
		vec3 newdestination = destination+slideNormal*t2;

		vec3 newmove = newdestination-tmpIntersection;
		vec3 newpos = pos+v;

		vec3 r;
		if(reclevel > 0)
		{
			r = newmove;
			collide(newpos, newmove, radius, &r, NULL, NULL, reclevel-1);	// next collisions
		}

		*result = v+r;
		return true;
	}
	else 
	{
		if(result) *result=move;
		return false;
	}
}

}

