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

#include "pla/intersection.hpp"

namespace pla
{

float intersectPlane(const vec3 &p, const vec3 &direction, const vec3 &planeOrigin, const vec3 &planeNormal)
{
	/*
	n.p = D
	n.p(t) = D
	n.(s+d*t) = D	// s=source d=direction
	(n.s) + (n.d)*t = D
	*/

	float d = -glm::dot(planeNormal, planeOrigin);
	return -(glm::dot(planeNormal, p)+d)/glm::dot(planeNormal, direction);
}

float intersectSphere(const vec3 &p, vec3 direction, const vec3 &centre, float radius)
{
	float len = glm::length(direction);
	direction/= len;

	vec3 dst = p - centre;
	float b = glm::dot(dst, direction);
	float c = glm::length2(dst) - radius*radius;
	float d = b*b - c;

	if(d < 0.f) return std::numeric_limits<float>::infinity();
	float t = -b-std::sqrt(d);
	if(t < 0.f) return std::numeric_limits<float>::infinity();
	return t/len;
}

bool isInTriangle(const vec3 &p, const vec3 &p1, const vec3 &p2, const vec3 &p3)
{
	vec3 v1 = glm::normalize(p - p1);
	vec3 v2 = glm::normalize(p - p2);
	vec3 v3 = glm::normalize(p - p3);
	
	float total = std::acos(glm::dot(v1, v2));
	total+= std::acos(glm::dot(v2, v3));
	total+= std::acos(glm::dot(v3, v1)); 

	if(std::abs(total-2*Pi) <= Epsilon) return true;
	else return false;
}

vec3 closestPointOnTriangle(const vec3 &p, const vec3 &p1,const vec3 &p2, const vec3 &p3)
{
	vec3 nearest[3];
	nearest[0] = closestPointOnSegment(p, p1, p2);
	nearest[1] = closestPointOnSegment(p, p2, p3);
	nearest[2] = closestPointOnSegment(p, p3, p1);

	float dist[3];
	for(int i=0; i<3; ++i)
		dist[i] = distance(p, nearest[i]);

	if(dist[0] < dist[1])
	{
		if(dist[2]<dist[0]) return nearest[2];
		else return nearest[0];
	}
	else {
		if(dist[2]<dist[1]) return nearest[2];
		else return nearest[1];
	}
}

vec3 closestPointOnSegment(const vec3 &p, const vec3 &p1, const vec3 &p2)
{
	vec3 c = p - p1;
	vec3 v = glm::normalize(p2 - p1);
	float d = glm::distance(p1, p2);
	float t = glm::dot(v, c);

	if(t < 0.f) return p1;
	if(t > d) return p2;
	
	return p1 + glm::normalize(v)*t;
}

float intersectBox(const vec3 &p, const vec3 &move, float radius, const vec3 &min, const vec3 &max, vec3 *intersection)
{
	float t[6];
	vec3 inter[6];
	
	// OPTI: intersection is computed even if argument is NULL
	// OPTI: on a cube, there is only two intersections, and only one in the right direction
	t[0] = intersectFace(p, move, radius, vec3(min.x, min.y, min.z), vec3(max.x, min.y, min.z), vec3(max.x, max.y, min.z), vec3(min.x, max.y, min.z), &inter[0]);
	t[1] = intersectFace(p, move, radius, vec3(min.x, min.y, max.z), vec3(max.x, min.y, max.z), vec3(max.x, max.y, max.z), vec3(min.x, max.y, max.z), &inter[1]);
	t[2] = intersectFace(p, move, radius, vec3(min.x, min.y, min.z), vec3(min.x, max.y, max.z), vec3(min.x, max.y, max.z), vec3(min.x, min.y, max.z), &inter[2]);
	t[3] = intersectFace(p, move, radius, vec3(max.x, min.y, min.z), vec3(max.x, max.y, min.z), vec3(max.x, max.y, max.z), vec3(max.x, min.y, max.z), &inter[3]);
	t[4] = intersectFace(p, move, radius, vec3(min.x, min.y, min.z), vec3(max.x, min.y, min.z), vec3(max.x, min.y, max.z), vec3(min.x, min.y, max.z), &inter[4]);
	t[5] = intersectFace(p, move, radius, vec3(min.x, max.y, min.z), vec3(max.x, max.y, min.z), vec3(max.x, max.y, max.z), vec3(min.x, max.y, max.z), &inter[5]);
	
	float tmin = std::numeric_limits<float>::infinity();
	for(int i=0; i<6; ++i)
		if(t[i] <= tmin)
		{
			tmin = t[i];
			if(intersection) *intersection = inter[i];
		}

	return tmin;
}

float intersectFace(const vec3 &p, const vec3 &move, float radius, const vec3 &p1, const vec3 &p2, const vec3 &p3, const vec3 &p4, vec3 *intersection)
{
	vec3 inter1, inter2;
	float t1 = intersectFace(p, move, radius, p1, p2, p3, &inter1);
	float t2 = intersectFace(p, move, radius, p1, p3, p4, &inter2);

	if(t1 < t2)
	{
		if(intersection) *intersection = inter1;
		return t1;
	}
	
	if(intersection) *intersection = inter2;
	return t2;
}

float intersectFace(const vec3 &p, const vec3 &move, float radius, const vec3 &p1, const vec3 &p2, const vec3 &p3, vec3 *intersection)
{
	vec3 normal = glm::normalize(glm::cross(p1-p2, p1-p3));	// normale au plan de la face

	// NB: p1 sert d'origine pour le plan
	float planeDist = intersectPlane(p, -normal, p1, normal);	// distance de la source au plan de la face
	if(planeDist < 0.f) return std::numeric_limits<float>::infinity();

	vec3 planeintersection;

	// si la sphere est entrée dans le plan
	if(std::abs(planeDist) <= radius)
	{
		planeintersection = p - normal*planeDist;
	}
	else {
		vec3 ellipsoidintersection = p - normal*radius;
		float t = intersectPlane(ellipsoidintersection, move, p1, normal);
		planeintersection = ellipsoidintersection + move*t;	// calcule l'intersection avec le plan
	}

	vec3 polygonintersection;
	if(isInTriangle(planeintersection, p1, p2, p3)) polygonintersection = planeintersection;
	else polygonintersection = closestPointOnTriangle(planeintersection, p1, p2, p3);
	
	if(intersection) *intersection = polygonintersection;

	// si on est déjà en train de passer à travers un polygone
	if(glm::distance(p, polygonintersection) < radius) return 0.f;

	// contre-intersection avec la sphere
	return intersectSphere(polygonintersection, -move, p, radius);
}

}
