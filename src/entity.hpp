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

#ifndef CONVERGENCE_ENTITY_H
#define CONVERGENCE_ENTITY_H

#include "src/include.hpp"
#include "src/light.hpp"
#include "src/messagebus.hpp"

#include "pla/collidable.hpp"
#include "pla/context.hpp"
#include "pla/object.hpp"

namespace convergence {

using pla::Collidable;
using pla::Context;
using pla::Object;

class Entity : public MessageBus::AsyncListener {
public:
	Entity(sptr<MessageBus> messageBus, identifier id);
	virtual ~Entity();

	identifier id() const;
	vec3 getPosition() const;
	vec3 getDirection() const;
	mat4 getTransform() const;

	void setTransform(mat4 m);
	void transform(const mat4 &m);
	void accelerate(const vec3 &v);

	bool isOnGround() const;

	bool isPicked() const { return mIsPicked; }
	void setPicked(bool picked) { mIsPicked = picked; }

	virtual float getRadius() const;
	virtual vec3 getSpeed() const;

	virtual void collect(Light::Collection &lights);
	virtual void update(sptr<Collidable> terrain, double time);
	virtual int draw(const Context &context);

protected:
	virtual void handleCollision(const vec3 &normal);
	virtual void processMessage(const Message &message);
	void sendTransform() const;

	sptr<MessageBus> mMessageBus;
	identifier mId;
	mat4 mTransform;
	vec3 mSpeed;
	bool mIsOnGround;

	bool mIsPicked = false;
};

} // namespace convergence

#endif
