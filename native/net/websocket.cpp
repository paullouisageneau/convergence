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

#include "net/websocket.hpp"

#include <iostream>
#include <exception>

const size_t DEFAULT_MAX_PAYLOAD_SIZE = 16384;	// 16 KB

namespace net
{

WebSocket::WebSocket(void) : mMaxPayloadSize(DEFAULT_MAX_PAYLOAD_SIZE)
{

}

WebSocket::WebSocket(const string &url) : WebSocket()
{
	open(url);
}

WebSocket::~WebSocket(void)
{

}

void WebSocket::open(const string &url)
{
	close();

	mUrl = url;
	mThread = std::thread(&WebSocket::run, this);
}

void WebSocket::close(void)
{
	mWebSocket.close();
	if(mThread.joinable()) mThread.join();
	mConnected = false;
}

bool WebSocket::isOpen(void) const
{
	return mConnected;
}

bool WebSocket::isClosed(void) const
{
	return !mThread.joinable();
}

void WebSocket::setMaxPayloadSize(size_t size) {
	mMaxPayloadSize = size;
}

void WebSocket::send(const binary &data)
{
	mWebSocket.write(data);
}

void WebSocket::run(void)
{
	if(mUrl.empty()) return;

	try {
		mWebSocket.connect(mUrl);

		mConnected = true;
		triggerOpen();

		binary buffer;
		while(mWebSocket.read(buffer, mMaxPayloadSize))
			triggerMessage(buffer);
	}
	catch(const std::exception &e)
	{
		triggerError(e.what());
	}

	mWebSocket.close();

	if(mConnected) triggerClosed();
	mConnected = false;
}

}
