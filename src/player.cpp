/***************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                         *
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

#include "pla/binaryformatter.hpp"
#include "pla/program.hpp"
#include "pla/shader.hpp"

namespace convergence {

using pla::BinaryFormatter;
using pla::FragmentShader;
using pla::Program;
using pla::VertexShader;

Player::Player(sptr<MessageBus> messageBus, const identifier &id)
    : mMessageBus(messageBus), mId(id) {
	mYaw = mPitch = 0.f;
	mSpeed = 0.f;
	mGravity = 0.f;
	mIsOnGround = false;
	mIsJumping = false;

	mPosition = vec3(0.f, 0.f, 0.f);

	auto program = std::make_shared<Program>(std::make_shared<VertexShader>("shader/color.vect"),
	                                         std::make_shared<FragmentShader>("shader/color.frag"));

	program->bindAttribLocation(0, "position");
	program->bindAttribLocation(1, "normal");
	program->bindAttribLocation(2, "color");
	program->link();

	float cube_vertices[] = {
	    -1.0, -1.0, 1.0,  1.0, -1.0, 1.0,  1.0, 1.0, 1.0,  -1.0, 1.0, 1.0,
	    -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0,
	};

	Object::index_t cube_indices[] = {
	    0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
	    4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3,
	};

	mObject = std::make_shared<Object>(cube_indices, 12 * 3, cube_vertices, 8 * 3, program);
	mObject->computeNormals(1);
}

Player::~Player(void) {}

identifier Player::id(void) const { return mId; }

vec3 Player::getPosition(void) const { return mPosition; }

vec3 Player::getDirection(void) const {
	return vec3(std::sin(-mYaw), std::cos(-mYaw), 0.f) * std::cos(mPitch) +
	       vec3(0.f, 0.f, std::sin(mPitch));
}

mat4 Player::getTransform(void) const {
	mat4 matrix = mat4(1.0f);
	matrix = glm::translate(matrix, mPosition);
	matrix = glm::rotate(matrix, Pi / 2, vec3(1, 0, 0));
	matrix = glm::rotate(matrix, mYaw, vec3(0, 1, 0));
	matrix = glm::rotate(matrix, mPitch, vec3(1, 0, 0));
	return matrix;
}

bool Player::isOnGround(void) const { return mIsOnGround; }

bool Player::isJumping(void) const { return mIsJumping; }

void Player::rotate(float yaw, float pitch) {
	mYaw = yaw;
	mPitch = pitch;
}

void Player::move(float speed) { mSpeed = speed; }

void Player::jump(void) {
	if (mIsOnGround)
		mIsJumping = true;
}

void Player::update(sptr<Collidable> terrain, double time) {
	Message message;
	while (readMessage(message))
		processMessage(message);

	vec3 move(0.f);
	mGravity += 10.f * time;
	if (mIsOnGround && mIsJumping)
		mGravity -= 10.f;
	move.z -= mGravity * time;

	vec3 dir = vec3(std::sin(-mYaw), std::cos(-mYaw), 0.f);
	move += dir * float(time) * mSpeed;

	mIsOnGround = false;
	vec3 newmove, intersection, normal;
	if (terrain->collide(mPosition - vec3(0.f, 0.5f, 0.f), move, 1.f, &newmove, &intersection,
	                     &normal)) {
		move = newmove;

		if (normal.z > 0.f) {
			mIsOnGround = true;
			mIsJumping = false;
			mGravity = 0.f;
		}
	}

	mPosition += move;
}

int Player::draw(const Context &context) {
	// TODO
	Context subContext = context;
	subContext.setUniform("transform", context.transform() * getTransform());
	return mObject->draw(subContext);
}

void Player::processMessage(const Message &message) {
	BinaryFormatter formatter(message.payload);

	switch (message.type) {
	case Message::PlayerPosition: {
		float32_t x = 0.f;
		float32_t y = 0.f;
		float32_t z = 0.f;
		formatter >> x >> y >> z;
		mPosition = vec3(x, y, z);
		break;
	}

	case Message::PlayerControl: {
		float32_t yaw = 0.f;
		float32_t pitch = 0.f;
		formatter >> yaw >> pitch;
		rotate(yaw, pitch);

		float32_t speed = 0.f;
		formatter >> speed;
		move(speed);

		float32_t gravity = 0.f;
		formatter >> gravity;
		mGravity = gravity;

		uint32_t flags = 0;
		formatter >> flags;
		break;
	}

	default:
		// Ignore
		break;
	}
}

} // namespace convergence
