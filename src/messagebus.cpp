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


void MessageBus::addRoute(const identifier &id, shared_ptr<Channel> channel)
{
	mRoutes.insert(std::make_pair(id, channel));
}

void MessageBus::removeRoute(const identifier &id, shared_ptr<Channel> channel)
{
	auto range = mRoutes.equal_range(id);
	auto it = range.first;
	while(it != range.second)
	{
		if(it->second == channel) 
		{
			mRoutes.erase(it);
			break;
		}
		++it;
	}
}

void MessageBus::addChannel(shared_ptr<Channel> channel)
{
	mChannels.insert(channel);
	channel->onMessage([this, channel](const binary &data) {
		Message message(data);

		if(!message.source.empty()) {
			// TODO
			if(mRoutes.find(message.source) == mRoutes.end()) {
				addRoute(message.source, channel);
			}
		}

		if(message.type == 2)
		{
			identifier peerId;
			BinaryFormatter formatter(message.payload);
			while(formatter >> peerId) 
			{
				addRoute(peerId, channel);
				if(mPeerCallback) mPeerCallback(peerId);
			}
		}
		else {
			dispatch(message);
		}
	});

	Message message;
	message.type = 1;
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
	dispatch(message);
}

void MessageBus::onPeer(function<void(const identifier &id)> callback)
{
	mPeerCallback = callback;
}

void MessageBus::onMessage(function<void(const Message &message)> callback)
{
	mMessageCallback = callback;
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

void MessageBus::dispatch(const Message &message)
{
	if(message.destination.empty())
	{
		std::cout << "Missing message destination" << std::endl;
		return;
	}

	if(message.destination == mLocalId)
	{
		// Dispatch locally
		if(mMessageCallback) mMessageCallback(message);
		auto range = mListeners.equal_range(message.source);
		if(range.first != range.second) {
			for(auto it = range.first; it != range.second; ++it)
			{
				Listener *listener = it->second;
				listener->onMessage(message);
			}
		}
		else {
			std::cout << "No listener for " << to_hex(message.source)  << std::endl;
		}
	}
	else {
		auto it = mRoutes.find(message.destination);
		if(it != mRoutes.end())
		{
			shared_ptr<Channel> channel = it->second;
			channel->send(message);
		}
		else {
			std::cout << "No route for " << to_hex(message.destination)  << std::endl;
		}
	}
}

}

