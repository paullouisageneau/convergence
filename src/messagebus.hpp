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
#include <functional>

namespace convergence
{

using net::Channel;
using std::shared_ptr;
using std::multimap;
using std::set;
using std::function;

class MessageBus
{
public:
	MessageBus(const identifier &localId);
	~MessageBus(void);
	
	void addChannel(shared_ptr<Channel> channel);
	void removeChannel(shared_ptr<Channel> channel);
	
	void addRoute(const identifier &id, shared_ptr<Channel> channel);
	void removeRoute(const identifier &id, shared_ptr<Channel> channel);
	
	void send(Message &message);

	void onPeer(function<void(const identifier &id)> callback);
	void onMessage(function<void(const Message &message)> callback);

	class Listener
	{
	public:
		virtual void onMessage(const Message &message) = 0;
	};
	
	void registerListener(const identifier &remoteId, Listener *listener);
	void unregisterListener(const identifier &remoteId, Listener *listener);

private:
	void dispatch(const Message &message);
	
	identifier mLocalId;
	set<shared_ptr<Channel>> mChannels;
	multimap<identifier, shared_ptr<Channel>> mRoutes;
	multimap<identifier, Listener*> mListeners;

	function<void(const identifier &id)> mPeerCallback;
	function<void(const Message &message)> mMessageCallback;
};

}

#endif

