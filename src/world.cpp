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
#include "src/firefly.hpp"

#include <map>

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

	mEntities[identifier()] = std::make_shared<Firefly>(mMessageBus, identifier());

	// Factory factory("pickaxe", 1.f / 32.f, program);
	// mObjects[identifier()] = factory.build();
}

World::~World() {}

sptr<Terrain> World::terrain() const { return mTerrain; }

sptr<Player> World::localPlayer() const { return mLocalPlayer; }

sptr<Player> World::createPlayer(identifier id, Entity::Init init) {
	auto player = std::make_shared<Player>(mMessageBus, std::move(id));
	mMessageBus->registerListener(id, player);
	player->transform(init.transform);
	return player;
}

sptr<Entity> World::createEntity(identifier id, Entity::Init init) {
	// TODO: factory
	auto entity = std::make_shared<Firefly>(mMessageBus, std::move(id));
	mMessageBus->registerListener(id, entity);
	entity->transform(init.transform);
	return entity;
}

void World::localPick() {
	vec3 position = mLocalPlayer->getPosition();
	std::multimap<float, shared_ptr<Entity>> ordered;
	for (const auto &[id, entity] : mEntities) {
		float distance = glm::distance(entity->getPosition(), position);
		ordered.emplace(distance, entity);
	}

	for (const auto &[distance, entity] : ordered) {
		if (!entity->isPicked()) {
			if (distance <= 2.f)
				mLocalPlayer->pick(entity);
		}
		break;
	}
}

void World::collect(Light::Collection &lights) {
	for (auto &[id, player] : mPlayers)
		player->collect(lights);

	for (auto &[id, entity] : mEntities)
		entity->collect(lights);
}

void World::update(double time) {
	Message message;
	while (readMessage(message))
		processMessage(message);

	mTerrain->update(time);

	for (auto &[id, player] : mPlayers)
		player->update(mTerrain, time);

	for (auto &[id, entity] : mEntities)
		entity->update(mTerrain, time);
}

int World::draw(Context &context) {
	int count = 0;
	count += mTerrain->draw(context);

	for (const auto &[id, player] : mPlayers)
		count += player->draw(context);

	for (const auto &[id, entity] : mEntities)
		if (!entity->isPicked())
			count += entity->draw(context);

	return count;
}

void World::processMessage(const Message &message) {
	if (!message.source.isNull()) {
		const identifier &id = message.source;
		if (mPlayers.find(id) == mPlayers.end()) {
			std::cout << "New player: " << to_hex(id) << std::endl;
			// TODO
			mPlayers[id] = createPlayer(id, {});

			mTerrain->broadcast();
		}
	}
}

} // namespace convergence
