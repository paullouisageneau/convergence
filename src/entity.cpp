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

#include "src/entity.hpp"

#include "pla/binaryformatter.hpp"

namespace convergence {

using pla::BinaryFormatter;
using pla::FragmentShader;
using pla::Program;
using pla::VertexShader;

Entity::Entity(sptr<MessageBus> messageBus, identifier id)
    : mMessageBus(messageBus), mId(std::move(id)), mIsOnGround(false) {
	mTransform = mat4(1.f);
	mSpeed = vec3(0.f);
}

Entity::~Entity(void) {}

identifier Entity::id(void) const { return mId; }

vec3 Entity::getPosition(void) const { return glm::vec3(mTransform[3]); }

vec3 Entity::getDirection(void) const { return mTransform * vec4(0.f, 0.f, -1.f, 0.f); }

mat4 Entity::getTransform(void) const { return mTransform; }

void Entity::setTransform(mat4 m) { mTransform = std::move(m); }

void Entity::transform(const mat4 &m) { mTransform *= m; }

void Entity::accelerate(const vec3 &v) { mSpeed += v; }

bool Entity::isOnGround(void) const { return mIsOnGround; }

void Entity::collect(LightCollection &lights) {
	// Dummy
}

void Entity::update(sptr<Collidable> terrain, double time) {
	Message message;
	while (readMessage(message))
		processMessage(message);

	mSpeed -= vec3(0.f, 0.f, 10.f) * float(time); // gravity
	mIsOnGround = false;

	vec3 vect = getSpeed() * float(time);
	vec3 newvect, intersection, normal;
	if (terrain->collide(getPosition(), vect, getRadius(), &newvect, &intersection, &normal)) {
		vect = newvect;
		if (normal.z > 0.f)
			mIsOnGround = true;
		handleCollision(normal);
	}

	mTransform = glm::translate(vect) * mTransform;
}

float Entity::getRadius() const { return 1.f; }

vec3 Entity::getSpeed() const { return mSpeed; }

int Entity::draw(const Context &context) {
	// Dummy
	return 0;
}

void Entity::handleCollision(const vec3 &normal) {
	mSpeed -= normal * glm::dot(normal, mSpeed);
	mSpeed *= 0.999f;
}

void Entity::processMessage(const Message &message) {
	BinaryFormatter formatter(message.payload);

	switch (message.type) {

	case Message::EntityTransform: {
		float32_t x = 0.f, y = 0.f, z = 0.f;
		float32_t qx = 0.f, qy = 0.f, qz = 0.f, qw = 0.f;
		formatter >> x >> y >> z;
		formatter >> qx >> qy >> qz >> qw;
		setTransform(glm::translate(vec3(x, y, z)) * glm::mat4_cast(quat(qx, qy, qz, qw)));
		break;
	}

	case Message::EntitySpeed: {
		float32_t x = 0.f, y = 0.f, z = 0.f;
		formatter >> x >> y >> z;
		mSpeed = vec3(x, y, z);
		break;
	}

	default:
		// Ignore
		break;
	}
}

void Entity::sendTransform() const {
	Message message(Message::EntityTransform);
	BinaryFormatter formatter;
	vec3 p = glm::vec3(mTransform[3]);
	quat q = glm::quat_cast(mTransform);
	formatter << float32_t(p.x) << float32_t(p.y) << float32_t(p.z);
	formatter << float32_t(q.x) << float32_t(q.y) << float32_t(q.z) << float32_t(q.w);
	message.payload = formatter.data();
	mMessageBus->send(message);
}

} // namespace convergence
