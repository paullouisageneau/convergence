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

using pla::BinaryFormatter;

namespace convergence
{

Player::Player(sptr<MessageBus> messageBus, const identifier &id) :
	mMessageBus(messageBus),
	mId(id)
{
	mMessageBus->registerListener(mId, this);
	
	mYaw = mPitch = 0.f;
	mSpeed = 0.f;
	mGravity = 0.f;
	mIsOnGround = false;
	mIsJumping = false;
	
	mPosition = vec3(0.f, 0.f, 0.f);
}

Player::~Player(void)
{
	mMessageBus->unregisterListener(mId, this);
}

identifier Player::id(void) const
{
	return mId;
}

vec3 Player::getPosition(void) const
{
	return mPosition;
}

vec3 Player::getDirection(void) const
{
	return vec3(std::sin(-mYaw), std::cos(-mYaw), 0.f) * std::cos(mPitch) + vec3(0.f, 0.f, std::sin(mPitch));
}

mat4 Player::getTransform(void) const
{
	mat4 matrix = mat4(1.0f);
	matrix = glm::translate(matrix, mPosition);
	matrix = glm::rotate(matrix, Pi/2, vec3(1, 0, 0));
	matrix = glm::rotate(matrix, mYaw, vec3(0, 1, 0));
	matrix = glm::rotate(matrix, mPitch, vec3(1, 0, 0));
	return matrix;
}

bool Player::isOnGround(void) const
{
	return mIsOnGround;
}

bool Player::isJumping(void) const
{
	return mIsJumping;
}

void Player::rotate(float yaw, float pitch)
{
	mYaw = yaw;
	mPitch = pitch;
}

void Player::move(float speed)
{
	mSpeed = speed;
}

void Player::jump(void)
{
	if(mIsOnGround) mIsJumping = true;
}

void Player::update(sptr<Collidable> terrain, double time)
{
	vec3 move(0.f);
	mGravity+= 10.f*time;
	if(mIsOnGround && mIsJumping) mGravity-= 10.f;
	move.z-= mGravity*time;
	
	vec3 dir = vec3(std::sin(-mYaw), std::cos(-mYaw), 0.f);
	move+= dir*float(time)*mSpeed;
	
	mIsOnGround = false;
	vec3 newmove, intersection, normal;
	if(terrain->collide(mPosition - vec3(0.f, 0.5f, 0.f), move, 1.f, &newmove, &intersection, &normal))
	{
		move = newmove;
		
		if(normal.z > 0.f)
		{
			mIsOnGround = true;
			mIsJumping = false;
			mGravity = 0.f;
		}
	}

	mPosition+= move;
}

int Player::draw(const Context &context)
{
	return 0;
}

void Player::onMessage(const Message &message)
{
	BinaryFormatter formatter(message.payload);
	
	switch(message.type)
	{
		case Message::PlayerPosition:
		{
			float32_t x, y, z;
			formatter >> x >> y >> z;
			mPosition = vec3(x, y, z);
		}
		
		case Message::PlayerControl:
		{
			float32_t yaw = 0.f;
			float32_t pitch = 0.f;
			formatter >> yaw >> pitch;
			rotate(yaw, pitch);
			
			float32_t speed = 0.f;
			formatter >> speed;
			move(speed);
			
			uint32_t flags = 0;
			formatter >> flags;
			if(flags & 0x1) jump();
		}
	}
}

}

