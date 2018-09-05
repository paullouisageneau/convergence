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

#ifndef CONVERGENCE_PLAYER_H
#define CONVERGENCE_PLAYER_H

#include "src/include.hpp"

#include "pla/context.hpp"
#include "pla/collidable.hpp"

namespace convergence
{

using pla::Context;
using pla::Collidable;

class Player
{
public:
	Player(const identifier &id);
	~Player(void);

	void rotate(float yaw, float pitch);
	void jump(void);
	void setSpeed(float speed);
	
	vec3 getPosition(void) const;
	vec3 getDirection(void) const;
	mat4 getTransform(void) const;
	bool isOnGround(void) const;
	bool isJumping(void) const;
	
	void update(sptr<Collidable> terrain, double time);
	int draw(const Context &context);

private:
	identifier mId;
	vec3 mPosition;
	float mYaw, mPitch;
	float mSpeed;
	float mGravity;
	bool mIsOnGround;
	bool mIsJumping;
};

}

#endif

