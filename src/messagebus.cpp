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

namespace convergence
{

using pla::binary;
using pla::to_hex;
using pla::BinaryFormatter;

MessageBus::MessageBus(const identifier &localId) :
	mLocalId(localId)
{

}

MessageBus::~MessageBus(void) 
{
	
}

identifier MessageBus::localId(void) const
{
	return mLocalId;
}

void MessageBus::addRoute(const identifier &id, shared_ptr<Channel> channel, Priority priority)
{
	mRoutes[id][priority] = channel;
}

void MessageBus::removeRoute(const identifier &id, shared_ptr<Channel> channel)
{
	auto it = mRoutes.find(id);
	if(it != mRoutes.end())
	{
		auto &map = it->second;
		auto jt = map.begin();
		while(jt != map.end())
		{
			if(jt->second == channel)
			{
				map.erase(jt);
				if(map.empty()) mRoutes.erase(it);
				break;
			}
			++jt;
		}
	}
}

void MessageBus::addChannel(shared_ptr<Channel> channel)
{
	mChannels.insert(channel);
	channel->onMessage([this, channel](const binary &data) {
		Message message(data);

		if(!message.source.empty()) {
			addRoute(message.source, channel, Priority::Default);
		}

		if(message.type == Message::List)
		{
			identifier peerId;
			BinaryFormatter formatter(message.payload);
			while(formatter >> peerId) 
			{
				if(!peerId.isNull() && peerId != mLocalId)
				{
					addRoute(peerId, channel, Priority::Default);
					
					for(auto listener : mOmniscientListeners)
					{
						listener->onPeer(peerId);
					}
				}
			}
		}
		else {
			route(message);
		}
	});

	Message message(Message::Join);
	message.source = mLocalId;
	channel->send(message);
}

void MessageBus::removeChannel(shared_ptr<Channel> channel)
{
	auto it = mChannels.find(channel);
	if(it != mChannels.end())
	{
		mChannels.erase(it);
		channel->onMessage([](const binary &data) {});
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
	for(auto it = mListeners.begin(); it != mListeners.end(); ++it)
	{
		if(it->first != mLocalId)
		{
			message.destination = it->first;
			route(message);
		}
	}
	
	message.destination.clear();
}

void MessageBus::dispatch(const Message &message)
{
	// Dispatch locally
	for(auto listener : mOmniscientListeners)
	{
		listener->onMessage(message);
	}
	auto range = mListeners.equal_range(message.source);
	if(range.first != range.second)
	{
		for(auto it = range.first; it != range.second; ++it)
		{
			Listener *listener = it->second;
			listener->onMessage(message);
		}
	}
	else {
		//std::cout << "No listener for " << to_hex(message.source)  << std::endl;
	}
}

void MessageBus::registerListener(const identifier &remoteId, Listener *listener)
{
	mListeners.insert(std::make_pair(remoteId, listener));
}

void MessageBus::unregisterListener(const identifier &remoteId, Listener *listener)
{
	auto range = mListeners.equal_range(remoteId);
	auto it = range.first;
	while(it != range.second)
	{
		if(it->second == listener) 
		{
			mListeners.erase(it);
			break;
		}
		++it;
	}
}

void MessageBus::registerOmniscientListener(Listener *listener)
{
	mOmniscientListeners.insert(listener);
}

void MessageBus::unregisterOmniscientListener(Listener *listener)
{
	mOmniscientListeners.erase(listener);
}

void MessageBus::route(Message &message)
{
	if(message.destination == mLocalId || message.destination.isNull())
	{
		dispatch(message);
	}
	else {
		auto it = mRoutes.find(message.destination);
		if(it != mRoutes.end() && !it->second.empty())
		{
			// Take route with highest priority
			shared_ptr<Channel> channel = it->second.rbegin()->second;
			channel->send(message);
		}
		else {
			std::cout << "No route for " << to_hex(message.destination)  << std::endl;
		}
	}
}

}

