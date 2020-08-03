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

#ifndef CONVERGENCE_FIREFLY_H
#define CONVERGENCE_FIREFLY_H

#include "src/entity.hpp"
#include "src/include.hpp"

namespace convergence {

class Firefly : public Entity {
public:
	Firefly(sptr<MessageBus> messageBus, identifier id);
	virtual ~Firefly();

	virtual float getRadius() const;
	virtual vec3 getSpeed() const;

	virtual void collect(LightCollection &lights);
	virtual void update(sptr<Collidable> terrain, double time);
	virtual int draw(const Context &context);

protected:
	sptr<Object> mObject;
};

} // namespace convergence

#endif
