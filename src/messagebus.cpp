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

#include <chrono>
#include <list>
#include <random>

namespace convergence {

using pla::binary;
using pla::BinaryFormatter;
using pla::to_hex;

MessageBus::MessageBus() : mNode(legio::Configuration{}) {
	std::cout << "Local identifier: " << to_hex(localId()) << std::endl;
	mNode.onMessage([this](binary id, binary payload) {
		Message message(payload);
		message.source = std::move(id);
		message.destination = localId();
		dispatch(message);
	});
}

MessageBus::~MessageBus(void) {}

identifier MessageBus::localId(void) const { return mNode.id(); }

string MessageBus::localUrl() const { return mNode.url(); }

void MessageBus::bootstrap(string url) { mNode.connect(std::move(url)); }

void MessageBus::send(Message &message) {
	message.source = localId();
	if (message.destination == localId())
		dispatch(message);
	if (message.destination)
		mNode.send(*message.destination, message);
	else
		mNode.broadcast(message);
}

void MessageBus::broadcast(Message &message) {
	message.destination.reset();
	send(message);
}

void MessageBus::dispatch(const Message &message) {
	for (auto l : getListeners(message.type))
		l->onMessage(message);

	if (message.destination)
		for (auto l : getListeners(*message.destination))
			l->onMessage(message);
}

void MessageBus::dispatchPeer(const identifier &id) {
	for (auto l : getListeners())
		l->onPeer(id);
}

void MessageBus::registerListener(listenerFilter filter, weak_ptr<Listener> listener) {
	std::lock_guard<std::mutex> lock(mListenersMutex);
	mListeners.emplace(std::move(filter), std::move(listener));
}

std::vector<shared_ptr<MessageBus::Listener>>
MessageBus::getListeners(const listenerFilter &filter) {
	std::lock_guard<std::mutex> lock(mListenersMutex);
	std::vector<shared_ptr<Listener>> listeners;
	auto range = std::holds_alternative<std::monostate>(filter)
	                 ? mListeners.equal_range(filter)
	                 : std::make_pair(mListeners.begin(), mListeners.end());
	auto it = range.first;
	while (it != range.second) {
		if (auto l = it->second.lock()) {
			listeners.push_back(l);
			++it;
		} else {
			it = mListeners.erase(it);
		}
	}
	return listeners;
}

void MessageBus::AsyncListener::onMessage(const Message &message) {
	std::lock_guard<std::mutex> lock(mQueueMutex);
	mQueue.push(message);
}

bool MessageBus::AsyncListener::readMessage(Message &message) {
	std::lock_guard<std::mutex> lock(mQueueMutex);
	if (!mQueue.empty()) {
		message = mQueue.front();
		mQueue.pop();
		return true;
	}

	return false;
}

} // namespace convergence
