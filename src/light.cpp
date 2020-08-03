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

#include "src/light.hpp"

#include <algorithm>

namespace convergence {

using std::vector;

const size_t MaxLightsCount = 16;

LightCollection::LightCollection() { mLights.reserve(MaxLightsCount); }

int LightCollection::count() const { return int(mLights.size()); }

void LightCollection::add(Light light) {
	if (mLights.size() < MaxLightsCount)
		mLights.emplace_back(std::move(light));
}

vector<vec3> LightCollection::positions() const {
	vector<vec3> result(mLights.size());
	std::transform(mLights.begin(), mLights.end(), result.begin(),
	               [](const auto &light) { return light.position; });
	return result;
}

vector<vec4> LightCollection::colors() const {
	vector<vec4> result(mLights.size());
	std::transform(mLights.begin(), mLights.end(), result.begin(),
	               [](const auto &light) { return light.color; });
	return result;
}

vector<float> LightCollection::powers() const {
	vector<float> result(mLights.size());
	std::transform(mLights.begin(), mLights.end(), result.begin(),
	               [](const auto &light) { return light.power; });
	return result;
}

} // namespace convergence
