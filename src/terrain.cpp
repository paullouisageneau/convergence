/***************************************************************************
 *   Copyright (C) 2015-2016 by Paul-Louis Ageneau                         *
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

#include "src/world.hpp"

namespace convergence
{

Terrain::Terrain(unsigned int seed) :
	mSurface(seed)
{

}

Terrain::~Terrain(void)
{

}

void Terrain::update(double time)
{
	mSurface.update(time);
}

int Terrain::draw(const Context &context)
{
	return mSurface.draw(context);
}

float Terrain::intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection)
{
	return mSurface.intersect(pos, move, radius, intersection);
}

void Terrain::build(const vec3 &p, int weight)
{
	mSurface.addWeight(p, weight, 0);
}

void Terrain::dig(const vec3 &p, int weight, float radius)
{
	mSurface.addWeight(p, weight, radius, -1);
}

}
