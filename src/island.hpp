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

#ifndef COELI_ISLAND_H
#define COELI_ISLAND_H

#include "src/surface.hpp"

namespace convergence
{

using pla::vec3;
using pla::vec4;
template<typename T> using sptr = std::shared_ptr<T>;

class Island : public Collidable
{
public:
	Island(unsigned int seed);
	~Island(void);

	void update(double time);
	int draw(const Context &context);
	float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL);

	void build(const vec3 &p, int weight);
	void dig(const vec3 &p, int weight, float radius);

protected:
	Surface mSurface;
};

}

#endif
