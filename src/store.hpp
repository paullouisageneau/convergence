/***************************************************************************
 *   Copyright (C) 2017-2019 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_STORE_H
#define CONVERGENCE_STORE_H

#include "src/include.hpp"
#include "src/messagebus.hpp"

#include <mutex>
#include <unordered_map>

namespace convergence {

class Store : public MessageBus::Listener, public std::enable_shared_from_this<Store> {
public:
	Store(sptr<MessageBus> messageBus);
	virtual ~Store(void);

	binary insert(const binary &data);
	shared_ptr<binary> retrieve(const binary &digest) const;

	class Notifiable {
	public:
		virtual void notify(const binary &digest, shared_ptr<binary> data,
		                    shared_ptr<Store> store) = 0;
	};

	void request(const binary &digest, weak_ptr<Notifiable> notifiable);

private:
	void onMessage(const Message &message);
	void sendRequest(const binary &digest);
	void insertNotifiable(const binary &digest, weak_ptr<Notifiable> notifiable);

	shared_ptr<MessageBus> mMessageBus;
	std::unordered_map<binary, shared_ptr<binary>, binary_hash> mData;
	std::unordered_multimap<binary, weak_ptr<Notifiable>, binary_hash> mNotifiables;

	mutable std::mutex mMutex;

	static binary Hash(const binary &data);
};
} // namespace convergence

#endif
