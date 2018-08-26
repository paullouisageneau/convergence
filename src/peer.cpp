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

#include "src/peer.hpp"
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

Peer::Peer(const identifier &id, shared_ptr<MessageBus> signaling) :
	mId(id),
	mSignaling(signaling)
{
	mSignaling->registerListener(mId, this);
	
	vector<string> iceServers;
	iceServers.emplace_back("stun:stun.ageneau.net:3478");
	mPeerConnection = std::make_shared<PeerConnection>(iceServers);
	
	mPeerConnection->onDataChannel([this](shared_ptr<DataChannel> dataChannel) {
		std::cout << "Got a data channel !" << std::endl;
		if(dataChannel->label() == "main") {
			mDataChannel = dataChannel;
			mDataChannel->onOpen([this]() {
				std::cout << "Data channel open !" << std::endl;
			});
		}
	});

	mPeerConnection->onLocalDescription([this](const PeerConnection::SessionDescription &description) {
		std::cout << "Local description: " << description.sdp << std::endl;
		vector<string> fields;
		fields.push_back(description.type);
		fields.push_back(description.sdp);
		sendMessage(0x11, pack_strings(fields));
	});
	
	mPeerConnection->onLocalCandidate([this](const PeerConnection::IceCandidate &candidate) {
		if(candidate.candidate.empty()) return;
		std::cout << "Local candidate: " << candidate.candidate << std::endl;
		vector<string> fields;
		fields.push_back(candidate.sdpMid);
		fields.push_back(candidate.candidate);
		sendMessage(0x12, pack_strings(fields));
	});
}

Peer::~Peer(void) 
{
	mSignaling->unregisterListener(mId, this);
}

void Peer::connect(void)
{
	mDataChannel = mPeerConnection->createDataChannel("main");
	mDataChannel->onOpen([this]() {
		std::cout << "Data channel open !" << std::endl;
	});
}

void Peer::sendMessage(uint32_t type, const binary &payload)
{
	Message message;
	message.destination = mId;
	message.type = type;
	message.payload = payload;
	mSignaling->send(message);
}

void Peer::processMessage(uint32_t type, const binary &payload)
{
	switch(type)
	{
		case 0x11:
		{
			vector<string> fields(unpack_strings(payload));
			std::cout << "Remote description: " << fields[1] << std::endl;
			mPeerConnection->setRemoteDescription(PeerConnection::SessionDescription(fields[1], fields[0]));
			break;
		}

		case 0x12:
		{
			vector<string> fields(unpack_strings(payload));
			std::cout << "Remote candidate: " << fields[1] << std::endl;
			mPeerConnection->setRemoteCandidate(PeerConnection::IceCandidate(fields[1], fields[0]));
			break;
		}

		default:
		{
			std::cout << "Unknown message type: " << std::endl;
			break;
		}
	}
}

void Peer::onMessage(const Message &message)
{
	processMessage(message.type, message.payload);
}

}

