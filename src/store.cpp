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

#include "src/store.hpp"
#include "src/sha3.hpp"

#include "pla/binaryformatter.hpp"

namespace convergence {

using pla::BinaryFormatter;
using pla::to_hex;

Store::Store(sptr<MessageBus> messageBus) : mMessageBus(messageBus) {}

Store::~Store(void) {}

binary Store::insert(const binary &data) {
	const binary digest = Hash(data);
	auto dataPtr = std::make_shared<binary>(data);

	std::vector<weak_ptr<Notifiable>> notifiables;
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mData[digest] = dataPtr;

		auto range = mNotifiables.equal_range(digest);
		std::transform(range.first, range.second, std::back_inserter(notifiables),
		               [](decltype(mNotifiables)::value_type const &pair) { return pair.second; });
		mNotifiables.erase(range.first, range.second);
	}

	for (auto notif : notifiables) {
		if (auto locked = notif.lock()) {
			locked->notify(digest, dataPtr);
		}
	}

	return digest;
}

shared_ptr<binary> Store::retrieve(const binary &digest) const {
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mData.find(digest);
	return (it != mData.end() ? it->second : nullptr);
}

void Store::request(const binary &digest, weak_ptr<Notifiable> notifiable) {
	if (auto data = retrieve(digest)) {
		if (auto locked = notifiable.lock()) {
			locked->notify(digest, data);
		}
	} else {
		insertNotifiable(digest, notifiable);
		sendRequest(digest);
	}
}

void Store::onMessage(const Message &message) {
	switch (message.type) {
	case Message::Store: {
		std::cout << "Received data" << std::endl;
		insert(message.payload);
		break;
	}

	case Message::Request: {
		std::cout << "Received request for " << to_hex(message.payload) << std::endl;
		if (auto data = retrieve(message.payload)) {
			Message response(Message::Store);
			response.destination = message.source;
			response.payload = *data;
			mMessageBus->send(response);
		}
		break;
	}

	default:
		// Ignore
		break;
	}
}

void Store::sendRequest(const binary &digest) {
	std::cout << "Requesting " << to_hex(digest) << std::endl;

	Message message(Message::Request);
	message.payload = digest;
	mMessageBus->broadcast(message);
}

void Store::insertNotifiable(const binary &digest, weak_ptr<Notifiable> notifiable) {
	std::lock_guard<std::mutex> lock(mMutex);
	mNotifiables.insert(std::make_pair(digest, notifiable));
}

binary Store::Hash(const binary &data) {
	binary digest;
	Sha3_256(data, digest);
	digest.resize(16); // clamp to 128 bits
	return digest;
}

} // namespace convergence
