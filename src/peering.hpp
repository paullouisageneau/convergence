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

#ifndef CONVERGENCE_PEER_H
#define CONVERGENCE_PEER_H

#include "src/include.hpp"
#include "src/messagebus.hpp"

#include "rtc/rtc.hpp"

#include <functional>

namespace convergence {

class Peering : public MessageBus::Listener {
public:
	Peering(const identifier &id, shared_ptr<MessageBus> signaling);
	virtual ~Peering(void);

	identifier id(void) const;
	bool isConnected(void) const;

	void connect(void);
	void disconnect(void);

protected:
	void onMessage(const Message &message);

private:
	void setDataChannel(shared_ptr<rtc::DataChannel> dataChannel);
	void processSignaling(Message::Type type, const binary &payload);
	void sendSignaling(Message::Type type, const binary &payload);

	identifier mId;
	shared_ptr<MessageBus> mMessageBus;
	shared_ptr<rtc::PeerConnection> mPeerConnection;
	shared_ptr<rtc::DataChannel> mDataChannel;
};

} // namespace convergence

#endif
