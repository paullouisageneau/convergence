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

namespace convergence
{

Player::Player(const identifier &id) : mId(id)
{
	mYaw = 0.f;
	mPitch = 0.f;
	mSpeed = 0.f;
	mIsOnGround = false;
	
	mPosition = vec3(5.f, 5.f, 50.f);
}

Player::~Player(void)
{

}

void Player::rotate(float yaw, float pitch)
{
	mYaw = yaw;
	mPitch = pitch;
}

void Player::jump(void)
{
	if(mIsOnGround) mIsJumping = true;
}

void Player::setSpeed(float speed)
{
	mSpeed = speed;
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

}

