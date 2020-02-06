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

#include "src/world.hpp"
#include "src/factory.hpp"

namespace convergence {

using pla::to_hex;

World::World(sptr<MessageBus> messageBus) : mMessageBus(messageBus) {
	mStore = std::make_shared<Store>(mMessageBus);
	mMessageBus->registerTypeListener(Message::Store, mStore);
	mMessageBus->registerTypeListener(Message::Request, mStore);

	unsigned seed = 130;
	mTerrain = std::make_shared<Terrain>(mMessageBus, mStore, seed);
	mMessageBus->registerTypeListener(Message::TerrainRoot, mTerrain);
	mMessageBus->registerTypeListener(Message::TerrainUpdate, mTerrain);

	mLocalPlayer = std::make_shared<LocalPlayer>(mMessageBus);
	mMessageBus->registerListener(mLocalPlayer->id(), mLocalPlayer);
	mPlayers[mLocalPlayer->id()] = mLocalPlayer;

	auto program = std::make_shared<Program>(std::make_shared<VertexShader>("shader/color.vect"),
	                                         std::make_shared<FragmentShader>("shader/color.frag"));

	program->bindAttribLocation(0, "position");
	program->bindAttribLocation(1, "normal");
	program->bindAttribLocation(2, "color");
	program->link();

	Factory factory("res/pickaxe.png", 1.f / 32.f, program);
	mObjects[identifier()] = factory.build();
}

World::~World(void) {}

sptr<Player> World::localPlayer(void) const { return mLocalPlayer; }

sptr<Terrain> World::terrain(void) const { return mTerrain; }

void World::update(double time) {
	Message message;
	while (readMessage(message))
		processMessage(message);

	mTerrain->update(time);

	for (auto &[id, player] : mPlayers)
		player->update(mTerrain, time);
}

int World::draw(Context &context) {
	int count = 0;
	count += mTerrain->draw(context);

	for (const auto &[id, player] : mPlayers)
		count += player->draw(context);

	for (const auto &[id, object] : mObjects)
		count += object->draw(context);

	return count;
}

void World::processMessage(const Message &message) {
	if (!message.source.isNull()) {
		const identifier &id = message.source;
		if (mPlayers.find(id) == mPlayers.end()) {
			std::cout << "New player: " << to_hex(id) << std::endl;
			mPlayers[id] = createPlayer(id);

			mTerrain->broadcast();
		}
	}
}

shared_ptr<Player> World::createPlayer(const identifier &id) {
	auto player = std::make_shared<Player>(mMessageBus, id);
	mMessageBus->registerListener(id, player);
	return player;
}

} // namespace convergence
