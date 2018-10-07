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

#include <vector>
#include <iostream>

namespace convergence
{

using pla::binary;
using pla::pack_strings;
using pla::unpack_strings;
using std::vector;

const string DataChannelName = "data";

Peering::Peering(const identifier &id, shared_ptr<MessageBus> messageBus) :
	mId(id),
	mMessageBus(messageBus)
{
	mMessageBus->registerListener(mId, this);
	
	vector<string> iceServers;
	iceServers.emplace_back("stun:stun.ageneau.net:3478");
	mPeerConnection = std::make_shared<PeerConnection>(iceServers);
	
	mPeerConnection->onDataChannel([this](shared_ptr<DataChannel> dataChannel) {
		std::cout << "Got a data channel !" << std::endl;
		if(dataChannel->label() == DataChannelName) setDataChannel(dataChannel);
	});
	
	mPeerConnection->onLocalDescription([this](const PeerConnection::SessionDescription &description) {
		std::cout << "Local description: " << description.sdp << std::endl;
		vector<string> fields;
		fields.push_back(description.type);
		fields.push_back(description.sdp);
		sendSignaling(Message::Description, pack_strings(fields));
	});
	
	mPeerConnection->onLocalCandidate([this](const PeerConnection::IceCandidate &candidate) {
		if(candidate.candidate.empty()) return;
		std::cout << "Local candidate: " << candidate.candidate << std::endl;
		vector<string> fields;
		fields.push_back(candidate.sdpMid);
		fields.push_back(candidate.candidate);
		sendSignaling(Message::Candidate, pack_strings(fields));
	});
}

Peering::~Peering(void) 
{
	mMessageBus->unregisterListener(mId, this);
	disconnect();
}

identifier Peering::id(void) const
{
	return mId;
}

bool Peering::isConnected(void) const
{
	return mDataChannel && mDataChannel->isOpen();
}

void Peering::connect(void)
{
	disconnect();
	setDataChannel(mPeerConnection->createDataChannel(DataChannelName));
}

void Peering::disconnect(void)
{
	if(mDataChannel)
	{
		mMessageBus->removeChannel(mDataChannel);
		mDataChannel->close();
		mDataChannel.reset();
	}
}

void Peering::onMessage(const Message &message)
{
	if(uint32_t(message.type) < 0x20) 
	{
		processSignaling(message.type, message.payload);
	}
}

void Peering::setDataChannel(shared_ptr<DataChannel> dataChannel)
{
	mDataChannel = dataChannel;
	
	mDataChannel->onOpen([this]() {
		std::cout << "Data channel open !" << std::endl;
		mMessageBus->addChannel(mDataChannel, MessageBus::Priority::Relay);
		mMessageBus->addRoute(mId, mDataChannel, MessageBus::Priority::Direct);
	});
	
	mDataChannel->onClosed([this]() {
		std::cout << "Data channel closed" << std::endl;
		mMessageBus->removeChannel(mDataChannel);
	});
}

void Peering::processSignaling(Message::Type type, const binary &payload)
{
	switch(type)
	{
		case Message::Join:
		{
			// TODO: send list message
			break;
		}
		
		case Message::Description:
		{
			vector<string> fields(unpack_strings(payload));
			std::cout << "Remote description: " << fields[1] << std::endl;
			mPeerConnection->setRemoteDescription(PeerConnection::SessionDescription(fields[1], fields[0]));
			break;
		}

		case Message::Candidate:
		{
			vector<string> fields(unpack_strings(payload));
			std::cout << "Remote candidate: " << fields[1] << std::endl;
			mPeerConnection->setRemoteCandidate(PeerConnection::IceCandidate(fields[1], fields[0]));
			break;
		}

		default:
		{
			std::cout << "Unknown signaling message type: " << type << std::endl;
			break;
		}
	}
}

void Peering::sendSignaling(Message::Type type, const binary &payload)
{
	Message message;
	message.destination = mId;
	message.type = type;
	message.payload = payload;
	mMessageBus->send(message);
}

}

