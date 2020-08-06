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

#include "src/firefly.hpp"

#include "pla/program.hpp"
#include "pla/shader.hpp"

namespace convergence {

using pla::FragmentShader;
using pla::Program;
using pla::Sphere;
using pla::VertexShader;

Firefly::Firefly(sptr<MessageBus> messageBus, identifier id) : Entity(messageBus, std::move(id)) {

	mLight = std::make_shared<Light>(vec4(1.f, 0.9f, 0.6f, 1.f), 16.f);

	auto program = std::make_shared<Program>(std::make_shared<VertexShader>("shader/color.vect"),
	                                         std::make_shared<FragmentShader>("shader/color.frag"));
	mObject = std::make_shared<Sphere>(64, program);
}

Firefly::~Firefly() {}

float Firefly::getRadius() const { return 0.5f; }

vec3 Firefly::getSpeed() const { return Entity::getSpeed(); }

void Firefly::collect(Light::Collection &lights) { lights.add(mLight); }

void Firefly::update(sptr<Collidable> terrain, double time) {
	Entity::update(terrain, time);
	mLight->setPosition(getPosition());
}

int Firefly::draw(const Context &context) {
	int count = 0;
	if (!context.overrideProgram()) {
		Context subContext = context.transform(getTransform());
	    count += mObject->draw(subContext);
	}
	return count;
}

} // namespace convergence
