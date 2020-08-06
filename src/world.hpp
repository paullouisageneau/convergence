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

#ifndef CONVERGENCE_WORLD_H
#define CONVERGENCE_WORLD_H

#include "src/include.hpp"
#include "src/light.hpp"
#include "src/localplayer.hpp"
#include "src/messagebus.hpp"
#include "src/player.hpp"
#include "src/store.hpp"
#include "src/terrain.hpp"

#include "pla/context.hpp"
#include "pla/object.hpp"

#include <map>

namespace convergence {

using pla::Context;
using pla::Object;

class World final : public MessageBus::AsyncListener {
public:
	World(shared_ptr<MessageBus> messageBus);
	~World();

	sptr<Terrain> terrain() const;
	sptr<Player> localPlayer() const;

	void localPick();

	void collect(Light::Collection &lights);
	void update(double time);
	int draw(Context &context);

private:
	void processMessage(const Message &message);
	shared_ptr<Player> createPlayer(const identifier &id);

	sptr<MessageBus> mMessageBus;
	sptr<Store> mStore;
	sptr<Terrain> mTerrain;
	sptr<LocalPlayer> mLocalPlayer;
	std::map<identifier, sptr<Player>> mPlayers;
	std::map<identifier, sptr<Entity>> mEntities; // Non-player entities
};
} // namespace convergence

#endif
