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
#include "src/messagebus.hpp"

#include "pla/context.hpp"
#include "pla/collidable.hpp"
#include "pla/object.hpp"

namespace convergence
{

using pla::Context;
using pla::Collidable;
using pla::Object;

class Player : public MessageBus::AsyncListener
{
public:
	Player(sptr<MessageBus> messageBus, const identifier &id);
	virtual ~Player(void);
	
	identifier id(void) const;
	vec3 getPosition(void) const;
	vec3 getDirection(void) const;
	mat4 getTransform(void) const;
	bool isOnGround(void) const;
	bool isJumping(void) const;
	
	void rotate(float yaw, float pitch);
	void move(float speed);
	void jump(void);
	
	virtual void update(sptr<Collidable> terrain, double time);
	virtual int draw(const Context &context);

protected:
	virtual void processMessage(const Message &message);
	
	sptr<MessageBus> mMessageBus;
	identifier mId;
	vec3 mPosition;
	float mYaw, mPitch;
	float mSpeed;
	float mGravity;
	bool mIsOnGround;
	bool mIsJumping;
	
	sptr<Object> mObject;
};

}

#endif

