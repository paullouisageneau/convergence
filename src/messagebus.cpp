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

#include "src/messagebus.hpp"

#include "pla/binaryformatter.hpp"

#include <list>
#include <random>
#include <chrono>

namespace convergence
{

using pla::binary;
using pla::to_hex;
using pla::BinaryFormatter;

MessageBus::MessageBus(void)
{
	using clock = std::chrono::high_resolution_clock;
	using random_bytes_engine =
	    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
	random_bytes_engine rbe;
	rbe.seed(clock::now().time_since_epoch()/std::chrono::milliseconds(1));
	std::generate(mLocalId.begin(), mLocalId.end(), [&rbe]() { return byte(rbe()); });

	std::cout << "Local identifier: " << to_hex(mLocalId) << std::endl;
}

MessageBus::~MessageBus(void) {}

identifier MessageBus::localId(void) const
{
	return mLocalId;
}

void MessageBus::addRoute(const identifier &id, shared_ptr<Channel> channel, Priority priority)
{
	std::lock_guard<std::mutex> lock(mRoutesMutex);
	mRoutes[id][priority] = channel;
}

void MessageBus::removeRoute(const identifier &id, shared_ptr<Channel> channel)
{
	std::lock_guard<std::mutex> lock(mRoutesMutex);
	auto it = mRoutes.find(id);
	if(it != mRoutes.end())
	{
		auto &map = it->second;
		auto jt = map.begin();
		while(jt != map.end())
		{
			if(jt->second == channel) jt = map.erase(jt);
			else ++jt;
		}
		if(map.empty()) mRoutes.erase(it);
	}
}

void MessageBus::removeAllRoutes(shared_ptr<Channel> channel)
{
	std::lock_guard<std::mutex> lock(mRoutesMutex);
	auto it = mRoutes.begin();
	while(it != mRoutes.end())
	{
		auto &map = it->second;
		auto jt = map.begin();
		while(jt != map.end())
		{
			if(jt->second == channel) jt = map.erase(jt);
			else ++jt;
		}
		if(map.empty()) it = mRoutes.erase(it);
		else ++it;
	}
}

void MessageBus::addChannel(shared_ptr<Channel> channel, Priority priority)
{
	channel->onMessage(
	    [this, channel, priority](const binary &data) {
		    // This can be called on non-main thread
		    Message message(data);

		    if (!message.source.isNull()) {
			    addRoute(message.source, channel, priority);
		    }

		    if (message.type == Message::List) {
			    identifier peerId;
			    BinaryFormatter formatter(message.payload);
			    while (formatter >> peerId) {
				    if (!peerId.isNull() && peerId != mLocalId) {
					    addRoute(peerId, channel, priority);
					    dispatchPeer(peerId);
				    }
			    }
		    } else {
			    route(message);
		    }
	    },
	    [](const string &data) {
		    // Ignore
	    });

	Message message(Message::Join);
	message.source = mLocalId;
	channel->send(message);

	std::lock_guard<std::mutex> lock(mChannelsMutex);
	mChannels.insert(channel);
}

void MessageBus::removeChannel(shared_ptr<Channel> channel)
{
	removeAllRoutes(channel);

	std::lock_guard<std::mutex> lock(mChannelsMutex);
	auto it = mChannels.find(channel);
	if(it != mChannels.end())
	{
		mChannels.erase(it);
		channel->onMessage([](const binary &data) {}, [](const string &data) {});
	}
}

void MessageBus::send(Message &message)
{
	message.source = mLocalId;
	if(!message.destination.isNull()) route(message);
	else broadcast(message);
}

void MessageBus::broadcast(Message &message)
{
	message.source = mLocalId;

	// Broadcast to remote ids that have local listeners
	std::list<identifier> destinations;
	{
		std::lock_guard<std::mutex> lock(mListenersMutex);
		binary last;
		auto it = mListeners.begin();
		while(it != mListeners.end())
		{
			if (last == it->first) {
				++it;
				continue;
			}

			last = it->first;

			if(auto l = it->second.lock())
			{
				if(it->first != mLocalId) destinations.push_back(it->first);
				++it;
			}
			else {
				it = mListeners.erase(it);
			}
		}
	}

	for(auto d : destinations)
	{
		message.destination = d;
		route(message);
	}

	message.destination.clear();
}

void MessageBus::dispatch(const Message &message)
{
	std::list<shared_ptr<Listener>> listeners;
	{
		std::lock_guard<std::mutex> lock(mListenersMutex);
		auto range = mTypeListeners.equal_range(message.type);
		auto it = range.first;
		while(it != range.second)
		{
			if(auto l = it->second.lock())
			{
				listeners.push_back(l);
				++it;
			}
			else {
				it = mTypeListeners.erase(it);
			}
		}
	}

	for(auto l : listeners) l->onMessage(message);

	listeners.clear();
	{
		std::lock_guard<std::mutex> lock(mListenersMutex);
		auto range = mListeners.equal_range(message.source);
		auto it = range.first;
		while(it != range.second)
		{
			if(auto l = it->second.lock())
			{
				listeners.push_back(l);
				++it;
			}
			else {
				it = mListeners.erase(it);
			}
		}
	}

	for(auto l : listeners) l->onMessage(message);
}

void MessageBus::dispatchPeer(const identifier &id)
{
	std::list<shared_ptr<Listener>> listeners;
	{
		std::lock_guard<std::mutex> lock(mListenersMutex);
		auto it = mTypeListeners.begin();
		while(it != mTypeListeners.end())
		{
			if(auto l = it->second.lock())
			{
				listeners.push_back(l);
				++it;
			}
			else {
				it = mTypeListeners.erase(it);
			}
		}
	}

	for(auto l : listeners) l->onPeer(id);
}

void MessageBus::registerTypeListener(Message::Type type, weak_ptr<Listener> listener)
{
	std::lock_guard<std::mutex> lock(mListenersMutex);
	mTypeListeners.insert(std::make_pair(type, listener));
}

void MessageBus::registerListener(const identifier &remoteId, weak_ptr<Listener> listener)
{
	std::lock_guard<std::mutex> lock(mListenersMutex);
	mListeners.insert(std::make_pair(remoteId, listener));
}

void MessageBus::route(Message &message)
{
	if(message.destination == mLocalId || message.destination.isNull())
	{
		dispatch(message);
	}
	else {
		shared_ptr<Channel> channel = findRoute(message.destination);
		if (channel)
			channel->send(message);
	}
}

shared_ptr<Channel> MessageBus::findRoute(const identifier &remoteId)
{
	std::lock_guard<std::mutex> lock(mRoutesMutex);
	auto it = mRoutes.find(remoteId);
	if(it != mRoutes.end() && !it->second.empty())
	{
		// Choose route with highest priority
		return it->second.rbegin()->second;
	}

	std::cout << "No route for " << to_hex(remoteId)  << std::endl;
	return nullptr;
}

void MessageBus::AsyncListener::onMessage(const Message &message)
{
	std::lock_guard<std::mutex> lock(mQueueMutex);
	mQueue.push(message);
}

bool MessageBus::AsyncListener::readMessage(Message &message)
{
	std::lock_guard<std::mutex> lock(mQueueMutex);
	if(!mQueue.empty())
	{
		message = mQueue.front();
		mQueue.pop();
		return true;
	}

	return false;
}

}

