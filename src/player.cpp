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

#include "src/player.hpp"
#include "src/factory.hpp"

#include "pla/binaryformatter.hpp"
#include "pla/program.hpp"
#include "pla/shader.hpp"

namespace convergence {

using pla::BinaryFormatter;
using pla::FragmentShader;
using pla::Program;
using pla::VertexShader;

Player::Player(sptr<MessageBus> messageBus, identifier id)
    : Entity(messageBus, std::move(id)), mYaw(0.f), mPitch(0.f), mWalkSpeed(0.f), mAction(0.f),
      mIsJumping(false) {

	auto program = std::make_shared<Program>(std::make_shared<VertexShader>("shader/color.vect"),
	                                         std::make_shared<FragmentShader>("shader/color.frag"));

	float cube_vertices[] = {
	    -1.0, -1.0, 1.0,  1.0, -1.0, 1.0,  1.0, 1.0, 1.0,  -1.0, 1.0, 1.0,
	    -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0,
	};

	Object::index_t cube_indices[] = {
	    0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
	    4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3,
	};

	mObject = std::make_shared<Object>(cube_indices, 12 * 3, cube_vertices, 8 * 3, program);
	mTool = Factory("pickaxe", 1.f / 32.f, program).build();
}

Player::~Player(void) {}

bool Player::isJumping(void) const { return mIsJumping; }

void Player::pivot(float yaw, float pitch) {
	mYaw = yaw;
	mPitch = pitch;

	mTransform = glm::translate(getPosition());
	mTransform = glm::rotate(mTransform, Pi / 2.f, vec3(1.f, 0.f, 0.f));
	mTransform = glm::rotate(mTransform, mYaw, vec3(0.f, 1.f, 0.f));
	mTransform = glm::rotate(mTransform, mPitch, vec3(1.f, 0.f, 0.f));
}

void Player::walk(float speed) { mWalkSpeed = speed; }

void Player::jump(void) {
	if (mIsOnGround)
		mIsJumping = true;
}

void Player::jolt(float force) {
	if (isOnGround())
		accelerate(vec3(0.f, 0.f, std::abs(force)));
}

void Player::action(double frame) { mAction = frame; }

void Player::pick(sptr<Entity> entity) {
	if (!entity->isPicked()) {
		mPicked = entity;
		entity->setPicked(true);
	}
}

void Player::release() {
	if (mPicked) {
		mPicked->setPicked(false);
		mPicked->accelerate(getDirection() * 10.f);
		mPicked = nullptr;
	}
}

bool Player::isPicking() const { return mPicked != nullptr; }

float Player::getRadius() const { return 1.f; }

vec3 Player::getSpeed() const {
	return Entity::getSpeed() + vec3(std::sin(-mYaw), std::cos(-mYaw), 0.f) * mWalkSpeed;
}

void Player::update(sptr<Collidable> terrain, double time) {
	if (isOnGround() && mIsJumping)
		accelerate(vec3(0.f, 0.f, 10.f));

	Entity::update(terrain, time);

	if (isOnGround())
		mIsJumping = false;

	float t = mAction >= 0. ? (mAction < .9 ? (1. + mAction) / 1.8 : 1. - (mAction - .9) / 0.10)
	                        : (1. + mAction) / 1.8;

	mat4 handTransform = getTransform();
	handTransform = glm::translate(handTransform, vec3(0.45f, -0.6f - mPitch * 0.1f, -1.f));
	handTransform = glm::rotate(handTransform, mPitch * 0.1f + Pi * 0.75f, vec3(1, 0, 0));
	handTransform = glm::rotate(handTransform, -Pi / 8, vec3(0, 0, 1));
	handTransform = glm::rotate(handTransform, t * Pi / 2, vec3(1.f, -0.f, 0.3f));
	handTransform = glm::translate(handTransform, vec3(0.f, -0.4f, 0.f));

	if (mPicked)
		mPicked->setTransform(std::move(handTransform));
}

int Player::draw(const Context &context) {
	int count = 0;
	// count += mObject->draw(subContext);

	if (mPicked)
		mPicked->draw(context);

	return count;
}

void Player::handleCollision(const vec3 &normal) { mSpeed = vec3(0.f, 0.f, 0.f); }

void Player::processMessage(const Message &message) {
	BinaryFormatter formatter(message.payload);

	switch (message.type) {
	case Message::EntityControl: {
		float32_t yaw = 0.f;
		float32_t pitch = 0.f;
		formatter >> yaw >> pitch;
		pivot(yaw, pitch);

		float32_t speed = 0.f;
		formatter >> speed;
		mWalkSpeed = speed;

		uint32_t flags = 0;
		formatter >> flags;
		break;
	}

	default:
		Entity::processMessage(message);
		break;
	}
}

void Player::sendControl() const {
	Message message(Message::EntityControl);
	BinaryFormatter formatter;
	formatter << float32_t(mYaw) << float32_t(mPitch);
	formatter << float32_t(mWalkSpeed);
	formatter << uint32_t(0); // flags
	message.payload = formatter.data();
	mMessageBus->send(message);
}

} // namespace convergence
