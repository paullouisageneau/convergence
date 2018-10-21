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

#ifndef CONVERGENCE_MESSAGEBUS_H
#define CONVERGENCE_MESSAGEBUS_H

#include "src/include.hpp"
#include "src/message.hpp"

#include "net/channel.hpp"

#include <memory>
#include <map>
#include <set>
#include <queue>

namespace convergence
{

using net::Channel;

class MessageBus
{
public:
	enum class Priority : int { Default = 0, Relay = 1, Direct = 2 };
	
	MessageBus(void);
	~MessageBus(void);
	
	identifier localId(void) const;
	
	void addChannel(shared_ptr<Channel> channel, Priority priority);
	void removeChannel(shared_ptr<Channel> channel);
	
	void addRoute(const identifier &id, shared_ptr<Channel> channel, Priority priority);
	void removeRoute(const identifier &id, shared_ptr<Channel> channel);
	void removeAllRoutes(shared_ptr<Channel> channel);
	
	void send(Message &message);
	void broadcast(Message &message);
	void dispatch(const Message &message);
	
	class Listener
	{
	public:
		virtual void onPeer(const identifier &id) {};
		virtual void onMessage(const Message &message) = 0;
	};
	
	class AsyncListener : public Listener
	{
	public:
		void onMessage(const Message &message);
		bool readMessage(Message &message);
		
	private:
		std::queue<Message> mQueue;
		std::mutex mQueueMutex;
	};
	
	void registerTypeListener(Message::Type type, weak_ptr<Listener> listener);
	void registerListener(const identifier &remoteId, weak_ptr<Listener> listener);
	
private:
	void dispatchPeer(const identifier &id);
	void route(Message &message);
	shared_ptr<Channel> findRoute(const identifier &remoteId);
	
	identifier mLocalId;
	std::set<shared_ptr<Channel>> mChannels;
	std::mutex mChannelsMutex;
	std::map<identifier, std::map<Priority, shared_ptr<Channel>>> mRoutes;
	std::mutex mRoutesMutex;
	std::multimap<Message::Type, weak_ptr<Listener>> mTypeListeners;
	std::multimap<identifier, weak_ptr<Listener>> mListeners;
	std::mutex mListenersMutex;
};

}

#endif

