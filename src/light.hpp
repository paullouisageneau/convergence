/***************************************************************************
 *   Copyright (C) 2017-2020 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_LIGHT_H
#define CONVERGENCE_LIGHT_H

#include "src/include.hpp"

namespace convergence {

struct Light {
	vec3 position;
	vec4 color;
	float power = 1.f;
};

class LightCollection {
public:
	LightCollection();

	void add(Light light);

	int count() const;

	std::vector<vec3> positions() const;
	std::vector<vec4> colors() const;
	std::vector<float> powers() const;

private:
	std::vector<Light> mLights;
};

} // namespace convergence

#endif
