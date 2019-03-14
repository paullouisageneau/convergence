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

#include <emscripten/emscripten.h>
#include <iostream>
#include <exception>
#include <memory>

extern "C" {
	extern int  wsCreateWebSocket(const char *url);
	extern void wsDeleteWebSocket(int ws);
	extern void wsSetOpenCallback(int ws, void (*openCallback)(void*));
	extern void wsSetErrorCallback(int dc, void (*errorCallback)(const char*, void*));
	extern void wsSetMessageCallback(int ws, void (*messageCallback)(const char *, int, void*));
	extern int  wsSendMessage(int ws, const char *buffer, int size);
	extern void wsSetUserPointer(int ws, void *ptr);
}

namespace net
{

void WebSocket::OpenCallback(void *ptr)
{
	WebSocket *w = static_cast<WebSocket*>(ptr);
	if(w)
	{
		w->mConnected = true;
		w->triggerOpen();
	}
}

void WebSocket::ErrorCallback(const char *error, void *ptr)
{
	WebSocket *w = static_cast<WebSocket*>(ptr);
	if(w) w->triggerError(string(error ? error : "unknown"));
}

void WebSocket::MessageCallback(const char *data, int size, void *ptr)
{
	WebSocket *w = static_cast<WebSocket*>(ptr);
	if(w)
	{
		if(data) w->triggerMessage(binary(data, data + size));
		else {
			w->close();
			w->triggerClosed();
		}
	}
}

WebSocket::WebSocket(void) : mId(0), mConnected(false)
{

}

WebSocket::WebSocket(const string &url) : mId(0)
{
	open(url);
}

WebSocket::~WebSocket(void)
{
	close();
}

void WebSocket::open(const string &url)
{
	close();

	mId = wsCreateWebSocket(url.c_str());
	if(!mId) throw std::runtime_error("WebSocket not supported");

	wsSetUserPointer(mId, this);
	wsSetOpenCallback(mId, OpenCallback);
	wsSetErrorCallback(mId, ErrorCallback);
	wsSetMessageCallback(mId, MessageCallback);
}

void WebSocket::close(void)
{
	mConnected = false;
	if(mId)
	{
		wsDeleteWebSocket(mId);
		mId = 0;
	}
}

bool WebSocket::isOpen(void) const
{
	return mConnected;
}

bool WebSocket::isClosed(void) const
{
	return mId == 0;
}

void WebSocket::send(const binary &data)
{
	if(!mId) return;
	wsSendMessage(mId, data.data(), data.size());
}

void WebSocket::triggerOpen(void)
{
	mConnected = true;
	Channel::triggerOpen();
}

}
