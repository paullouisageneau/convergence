
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
	mWebSocket.connect(mUrl);
	
	mConnected = true;
	triggerOpen();
	
	try {
		binary buffer;
		while(mWebSocket.read(buffer, mMaxPayloadSize))
			triggerMessage(buffer);
	}
	catch(const std::exception &e)
	{
		triggerError(e.what());
	}
	
	mWebSocket.close();
	mConnected = false;
	
	triggerClosed();
}

}
