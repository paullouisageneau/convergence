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

#include "src/networking.hpp"

#include "net/websocket.hpp"

#include <random>
#include <chrono>

namespace convergence
{

using net::WebSocket;
using pla::to_hex;

Networking::Networking(const string &url)
{
	using clock = std::chrono::high_resolution_clock;
	using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
	random_bytes_engine rbe;
	rbe.seed(clock::now().time_since_epoch()/std::chrono::milliseconds(1));
	std::generate(mLocalId.begin(), mLocalId.end(), std::ref(rbe));

	std::cout << "Local peer is " << to_hex(mLocalId) << std::endl;
	mMessageBus = std::make_shared<MessageBus>(mLocalId);
	mMessageBus->registerOmniscientListener(this);
	
	connectWebSocket(url);
}

Networking::~Networking(void)
{
	mMessageBus->unregisterOmniscientListener(this);
}

identifier Networking::localId(void) const
{
	return mLocalId;
}

shared_ptr<MessageBus> Networking::messageBus(void) const
{
	return mMessageBus;
}

void Networking::onPeer(const identifier &id)
{
	std::cout << "Discovered peer " << to_hex(id) << std::endl;
	auto peering = createPeering(id);
	peering->connect();
}

void Networking::onMessage(const Message &message)
{
	if(message.type == Message::Description) {
		const identifier &id = message.source;
		std::cout << "Incoming peer " << to_hex(id) << std::endl;
		createPeering(id);
	}
}

void Networking::connectWebSocket(const string &url)
{
	auto webSocket = std::make_shared<WebSocket>(url);
	webSocket->onOpen([this, webSocket]() {
		std::cout << "WebSocket opened" << std::endl;
		mMessageBus->addChannel(webSocket, MessageBus::Priority::Default);
	});
	
	webSocket->onError([this](const string &error) {
		std::cerr << "WebSocket error: " << error << std::endl;
	});
	
	webSocket->onClosed([this, webSocket]() {
		std::cerr << "WebSocket closed" << std::endl;
		mMessageBus->removeChannel(webSocket);
	});
}

shared_ptr<Peering> Networking::createPeering(const identifier &id)
{
	auto it = mPeerings.find(id);
	if(it != mPeerings.end()) return it->second;
	
	auto peering = std::make_shared<Peering>(id, mMessageBus);
	mPeerings[id] = peering;
	return peering;
}

}

