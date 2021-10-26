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

#include "src/identifier.hpp"
#include "src/include.hpp"
#include "src/message.hpp"

#include "legio/legio.hpp"

#include <map>
#include <memory>
#include <queue>
#include <set>

namespace convergence {

class MessageBus {
public:
	MessageBus();
	virtual ~MessageBus();

	identifier localId() const;

	void bootstrap(string url);

	void send(Message &message);
	void broadcast(Message &message);
	void dispatch(const Message &message);

	class Listener {
	public:
		virtual void onPeer(const identifier &id){};
		virtual void onMessage(const Message &message) = 0;
	};

	class AsyncListener : public Listener {
	public:
		void onMessage(const Message &message);
		bool readMessage(Message &message);

	private:
		std::queue<Message> mQueue;
		std::mutex mQueueMutex;
	};

	using listenerFilter = variant<nullptr_t, Message::Type, identifier>;
	void registerListener(listenerFilter filter, weak_ptr<Listener> listener);

private:
	std::vector<shared_ptr<Listener>> getListeners(const listenerFilter &filter);
	void dispatchPeer(const identifier &id);

	legio::Node mNode;
	std::multimap<listenerFilter, weak_ptr<Listener>> mListeners;
	std::mutex mListenersMutex;
};

} // namespace convergence

#endif
