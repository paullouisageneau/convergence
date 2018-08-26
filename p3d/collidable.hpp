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

#ifndef P3D_COLLIDABLE_H
#define P3D_COLLIDABLE_H

#include "p3d/include.hpp"

namespace pla
{

class Collidable
{
public:
	virtual float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL) = 0;
	virtual bool collide(const vec3 &pos, const vec3 &move, float radius, vec3 *result = NULL, vec3 *intersection = NULL, vec3 *normal = NULL, int reclevel = 10);
};

}

#endif
