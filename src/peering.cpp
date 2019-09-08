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

#include "src/peering.hpp"
#include "src/message.hpp"

#include "pla/binary.hpp"

#include <iostream>
#include <vector>

namespace convergence {

using pla::binary;
using pla::pack_strings;
using pla::unpack_strings;
using std::function;
using std::vector;

const string DataChannelName = "data";

Peering::Peering(const identifier &id, shared_ptr<MessageBus> messageBus)
    : mId(id), mMessageBus(messageBus) {
	net::Configuration config;
	config.iceServers.emplace_back("stun:stun.ageneau.net:3478");
	mPeerConnection = std::make_shared<net::PeerConnection>(config);

	mPeerConnection->onDataChannel([this](shared_ptr<net::DataChannel> dataChannel) {
		std::cout << "Data channel received" << std::endl;
		if (dataChannel->label() == DataChannelName)
			setDataChannel(dataChannel);
	});

	mPeerConnection->onLocalDescription([this](const net::Description &description) {
		std::cout << "Local description: " << description << std::endl;
		vector<string> fields;
		fields.push_back(description.typeString());
		fields.push_back(string(description));
		sendSignaling(Message::Description, pack_strings(fields));
	});

	mPeerConnection->onLocalCandidate([this](const std::optional<net::Candidate> &candidate) {
		if (!candidate)
			return;
		std::cout << "Local candidate: " << *candidate << std::endl;
		vector<string> fields;
		fields.push_back(candidate->mid());
		fields.push_back(candidate->candidate());
		sendSignaling(Message::Candidate, pack_strings(fields));
	});
}

Peering::~Peering(void) { disconnect(); }

identifier Peering::id(void) const { return mId; }

bool Peering::isConnected(void) const { return mDataChannel && mDataChannel->isOpen(); }

void Peering::connect(void) {
	disconnect();
	setDataChannel(mPeerConnection->createDataChannel(DataChannelName));
}

void Peering::disconnect(void) {
	if (mDataChannel) {
		mMessageBus->removeChannel(mDataChannel);
		mDataChannel->close();
		mDataChannel.reset();
	}
}

void Peering::onMessage(const Message &message) {
	if (uint32_t(message.type) < 0x20) {
		processSignaling(message.type, message.payload);
	}
}

void Peering::setDataChannel(shared_ptr<net::DataChannel> dataChannel) {
	mDataChannel = dataChannel;

	mDataChannel->onOpen([this]() {
		std::cout << "Data channel open" << std::endl;
		mMessageBus->addChannel(mDataChannel, MessageBus::Priority::Relay);
		mMessageBus->addRoute(mId, mDataChannel, MessageBus::Priority::Direct);
	});

	mDataChannel->onClosed([this]() {
		std::cout << "Data channel closed" << std::endl;
		mMessageBus->removeChannel(mDataChannel);
	});
}

void Peering::processSignaling(Message::Type type, const binary &payload) {
	switch (type) {
	case Message::Join: {
		// TODO: send list message
		break;
	}

	case Message::Description: {
		vector<string> fields(unpack_strings(payload));
		net::Description description(fields[1], fields[0]);
		std::cout << "Remote description: " << description << std::endl;
		mPeerConnection->setRemoteDescription(description);
		break;
	}

	case Message::Candidate: {
		vector<string> fields(unpack_strings(payload));
		net::Candidate candidate(fields[1], fields[0]);
		std::cout << "Remote candidate: " << candidate << std::endl;
		mPeerConnection->addRemoteCandidate(candidate);
		break;
	}

	default: {
		std::cout << "Unknown signaling message type: " << type << std::endl;
		break;
	}
	}
}

void Peering::sendSignaling(Message::Type type, const binary &payload) {
	Message message;
	message.destination = mId;
	message.type = type;
	message.payload = payload;
	mMessageBus->send(message);
}

} // namespace convergence
