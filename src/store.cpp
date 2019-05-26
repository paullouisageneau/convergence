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

namespace convergence
{

using pla::BinaryFormatter;
using pla::to_hex;

Store::Store(sptr<MessageBus> messageBus) : mMessageBus(messageBus) {}

Store::~Store(void) {}

binary Store::insert(const binary &data) {
	const binary digest = Hash(data);
	const auto data_ptr = std::make_shared<binary>(data);
	mData[digest] = data_ptr;

	auto range = mNotifiables.equal_range(digest);
	for (auto it = range.first; it != range.second; ++it) {
		if (auto locked = it->second.lock()) {
			locked->notify(digest, data_ptr);
		}
	}
	mNotifiables.erase(range.first, range.second);
	return digest;
}

shared_ptr<binary> Store::retrieve(const binary &digest) const {
	auto it = mData.find(digest);
	return it != mData.end() ? it->second : nullptr;
}

void Store::addSource(const binary &digest, const identifier &source) {
	mSources.insert(std::make_pair(digest, source));
}

void Store::request(const binary &digest, weak_ptr<Notifiable> notifiable) {
	auto it = mData.find(digest);
	if (it != mData.end()) {
		if (auto locked = notifiable.lock()) {
			locked->notify(digest, it->second);
		}
	} else {
		mNotifiables.insert(std::make_pair(digest, notifiable));

		auto range = mSources.equal_range(digest);
		for (auto it = range.first; it != range.second; ++it) {
			sendRequest(digest, it->second);
		}
	}
}

void Store::processMessage(const Message &message) {
	switch(message.type)
	{
	case Message::Store: {
		insert(message.payload);
		break;
	}

	case Message::Request: {
		auto it = mData.find(message.payload);
		if (it != mData.end()) {
			Message message(Message::Store);
			message.payload = *it->second;
			mMessageBus->send(message);
		}
		break;
	}

	default:
		// Ignore
		break;
	}
}

void Store::sendRequest(const binary &digest, const identifier &destination) {
	std::cout << "Requesting " << to_hex(digest) << std::endl;

	Message message(Message::Request);
	message.payload = digest;
	message.destination = destination;
	mMessageBus->send(message);
}

binary Store::Hash(const binary &data) {
	binary digest;
	Sha3_256(data, digest);
	return digest;
}

} // namespace convergence

