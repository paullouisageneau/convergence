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

#include "src/entity.hpp"
#include "src/include.hpp"

namespace convergence {

class Player : public Entity {
public:
	Player(sptr<MessageBus> messageBus, identifier id);
	virtual ~Player();

	bool isJumping() const;

	void pivot(float yaw, float pitch);
	void walk(float speed);
	void jump();
	void jolt(float force);
	void action(double frame);
	void pick(sptr<Entity> entity);
	void release();
	bool isPicking() const;

	virtual float getRadius() const;
	virtual vec3 getSpeed() const;

	virtual void update(sptr<Collidable> terrain, double time);
	virtual int draw(const Context &context);

protected:
	virtual void handleCollision(const vec3 &normal);
	virtual void processMessage(const Message &message);
	void sendControl() const;

	float mYaw, mPitch;
	float mWalkSpeed;
	float mAction;
	bool mIsJumping;

	sptr<Object> mObject, mTool;
	sptr<Entity> mPicked;
};

} // namespace convergence

#endif
