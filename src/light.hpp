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

#include "pla/cubemap.hpp"
#include "pla/texture.hpp"

namespace convergence {

using pla::DepthCubeMap;
using pla::Texture;

class Light {
public:
	Light(vec4 color = vec4(1.f), float power = 1.f, vec3 position = vec3(0.f));

	vec4 color() const;
	float power() const;

	vec3 position() const;
	void setPosition(vec3 position);

	void bindDepth(int face);
	void unbindDepth();

	class Collection {
	public:
		Collection();

		void add(sptr<Light> light);
		const std::vector<sptr<Light>> &vector() const;
		int count() const;

		std::vector<vec3> positions() const;
		std::vector<vec4> colors() const;
		std::vector<float> powers() const;
		std::vector<sptr<Texture>> depthCubeMaps() const;

	private:
		std::vector<sptr<Light>> mLights;
	};

private:
	vec4 mColor;
	float mPower;
	vec3 mPosition;

	sptr<DepthCubeMap> mDepthCubeMap;
};

} // namespace convergence

#endif
