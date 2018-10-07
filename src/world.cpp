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

namespace convergence
{

using pla::to_hex;

World::World(sptr<MessageBus> messageBus) :
	mMessageBus(messageBus)
{
	mMessageBus->registerOmniscientListener(this);
	
	unsigned seed(time(NULL));
	
	mLedger = std::make_shared<Ledger>();
	mTerrain = std::make_shared<Terrain>(seed);

	mLocalPlayer = std::make_shared<LocalPlayer>(mMessageBus);
	mPlayers[mLocalPlayer->id()] = mLocalPlayer;
}

World::~World(void)
{
	mMessageBus->unregisterOmniscientListener(this);
}

sptr<Player> World::localPlayer(void) const
{
	return mLocalPlayer;
}

sptr<Terrain> World::terrain(void) const
{
	return mTerrain;
}

void World::update(double time)
{
	mTerrain->update(time);
		
	for(const auto &p : mPlayers)
	{
		sptr<Player> player = p.second;
		player->update(mTerrain, time);
	}
}

int World::draw(Context &context)
{
	int count = 0;

	count+= mTerrain->draw(context);

	for(const auto &p : mPlayers)
	{
		sptr<Player> player = p.second;
		count+= player->draw(context);
	}
	
	return count;
}

void World::onPeer(const identifier &id)
{
	std::cout << "New player: " << to_hex(id) << std::endl;
	mPlayers[id] = std::make_shared<Player>(mMessageBus, id);
}

void World::onMessage(const Message &message)
{
	// TODO
}

}

