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
#include <vector>

namespace convergence {

const size_t MaxLightsCount = 16;

Light::Light(vec4 color, float power, vec3 position)
    : mColor(std::move(color)), mPower(std::move(power)), mPosition(std::move(position)),
      mDepthCubeMap(std::make_shared<DepthCubeMap>(2048)) {}

vec4 Light::color() const { return mColor; }

float Light::power() const { return mPower; }

vec3 Light::position() const { return mPosition; }

void Light::setPosition(vec3 position) { mPosition = position; }

void Light::bindDepth(int face) { mDepthCubeMap->bindFramebuffer(face); }

void Light::unbindDepth() { mDepthCubeMap->unbindFramebuffer(); }

Light::Collection::Collection() { mLights.reserve(MaxLightsCount); }

void Light::Collection::add(sptr<Light> light) {
	if (mLights.size() < MaxLightsCount)
		mLights.emplace_back(std::move(light));
}

const std::vector<sptr<Light>> &Light::Collection::vector() const { return mLights; }

int Light::Collection::count() const { return int(mLights.size()); }

std::vector<vec3> Light::Collection::positions() const {
	std::vector<vec3> result(mLights.size());
	std::transform(mLights.begin(), mLights.end(), result.begin(),
	               [](const auto &light) { return light->mPosition; });
	return result;
}

std::vector<vec4> Light::Collection::colors() const {
	std::vector<vec4> result(mLights.size());
	std::transform(mLights.begin(), mLights.end(), result.begin(),
	               [](const auto &light) { return light->mColor; });
	return result;
}

std::vector<float> Light::Collection::powers() const {
	std::vector<float> result(mLights.size());
	std::transform(mLights.begin(), mLights.end(), result.begin(),
	               [](const auto &light) { return light->mPower; });
	return result;
}

std::vector<sptr<Texture>> Light::Collection::depthCubeMaps() const {
	std::vector<sptr<Texture>> result(mLights.size());
	std::transform(mLights.begin(), mLights.end(), result.begin(),
	               [](const auto &light) { return light->mDepthCubeMap; });
	return result;
}

} // namespace convergence
