/*************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                       *
 *   paul-louis (at) ageneau (dot) org                                   *
 *                                                                       *
 *   This file is part of Plateform.                                     *
 *                                                                       *
 *   Plateform is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Affero General Public License as      *
 *   published by the Free Software Foundation, either version 3 of      *
 *   the License, or (at your option) any later version.                 *
 *                                                                       *
 *   Plateform is distributed in the hope that it will be useful, but    *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        *
 *   GNU Affero General Public License for more details.                 *
 *                                                                       *
 *   You should have received a copy of the GNU Affero General Public    *
 *   License along with Plateform.                                       *
 *   If not, see <http://www.gnu.org/licenses/>.                         *
 *************************************************************************/

#ifndef NET_WEBSOCKET_H
#define NET_WEBSOCKET_H

#include "channel.hpp"

#include "pla/websocket.hpp"

#include <atomic>
#include <thread>
#include <variant>

namespace net
{

// WebSocket wrapper for emscripten
class WebSocket final : public Channel {
public:
	WebSocket(void);
	WebSocket(const string &url);
	~WebSocket(void);

	void open(const string &url);
	void close(void);
	void send(const std::variant<binary, string> &data);

	bool isOpen(void) const;
	bool isClosed(void) const;

	void setMaxPayloadSize(size_t size);

private:
	void run(void);

	string mUrl;
	pla::WebSocket mWebSocket;
	std::atomic<bool> mConnected;
	std::atomic<size_t> mMaxPayloadSize;

	std::thread mThread;
};
}

#endif // NET_WEBSOCKET_H
