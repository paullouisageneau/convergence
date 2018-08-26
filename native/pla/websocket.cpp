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

#include "pla/websocket.hpp"
#include "pla/securetransport.hpp"
#include "pla/binaryformatter.hpp"
#include "pla/crypto.hpp"	// for SHA1
#include "pla/random.hpp"	// for mask

#include <regex>

namespace pla
{

// http://tools.ietf.org/html/rfc6455#section-5.2  Base Framing Protocol
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
// | |1|2|3|       |K|             |                               |
// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
// |    Extended payload length continued, if payload len == 127   |
// + - - - - - - - - - - - - - - - +-------------------------------+
// |                               | Masking-key, if MASK set to 1 |
// +-------------------------------+-------------------------------+
// |    Masking-key (continued)    |          Payload Data         |
// +-------------------------------+ - - - - - - - - - - - - - - - +
// :                     Payload Data continued ...                :
// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
// |                     Payload Data continued ...                |
// +---------------------------------------------------------------+

const uint8_t CONTINUATION = 0x0;
const uint8_t TEXT_FRAME = 0x1;
const uint8_t BINARY_FRAME = 0x2;
const uint8_t CLOSE = 0x8;
const uint8_t PING = 0x9;
const uint8_t PONG = 0xA;

const size_t MAX_PING_PAYLOAD_SIZE = 125;

// TODO
/*
WebSocket *WebSocket::Upgrade(Http::Request &request)
{
	auto it = request.headers.find("Upgrade");
	if(it != request.headers.end())
	{
		std::list<string> protocols;
		it->second.explode(protocols, ',');
		for(string &proto : protocols)
		{
			proto.trim();
			if(proto.toLower() == "websocket")
			{
				Http::Response response(request, 101);
				response.headers["Connection"] = "Upgrade";
				response.headers["Upgrade"] = "websocket";
				
				it = request.headers.find("Sec-WebSocket-Key");
				if(it != request.headers.end())
				{
					string key = it->second.trimmed();
					string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
					string answerKey = Sha1().compute(key + guid).base64Encode();
					response.headers["Sec-WebSocket-Accept"] = answerKey;
				}
				
				response.send();
				
				request.stream = NULL;	// Steal the stream
				return new WebSocket(response.stream, true);	// Masking may be disabled on server side
			}
		}
	}
	return NULL;
}
*/

WebSocket::WebSocket(void) :
	mStream(NULL),
	mSendMask(true)
{

}

WebSocket::WebSocket(Stream *stream, bool disableMask) :
	WebSocket()
{
	Assert(stream);
	mStream = stream;
	mSendMask = !disableMask;
}

WebSocket::WebSocket(const string &url) :
	WebSocket()
{
	connect(url);
}

WebSocket::~WebSocket(void) 
{
	NOEXCEPTION(close());
}

void WebSocket::connect(const string &url)
{
	close();
	try {
		mSendMask = true;
	
		std::regex regex(R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)", std::regex::extended);
		std::smatch match;
		if(!std::regex_match(url, match, regex))
			throw std::invalid_argument("Malformed URL: " + url);

		const string &scheme = match[2];
		const string &host = match[4];
		const string &path = match[5];
		const string &query = match[7];
		const string &fragment = match[9];

		if(scheme != "ws" && scheme != "wss")
			throw std::invalid_argument("Invalid scheme for WebSocket: " + scheme);

		mStream = new Socket(Address(host));
		if(scheme == "wss") mStream = new SecureTransportClient(mStream, new SecureTransport::Certificate, host);

		const string fullPath = !query.empty() ? path + "?" + query : path;

		binary key;
		Random().read(key, 16);
			
		string request = "GET " + fullPath + " HTTP/1.1\r\n"
		"Host: " + host + "\r\n"
		"Connection: Upgrade\r\n"
		"Upgrade: websocket\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Sec-WebSocket-Key: " + to_base64(key) + "\r\n"
		"\r\n";
		mStream->write(request);

		string line;
		if(!mStream->readLine(line)) throw std::runtime_error("Connection unexpectedly closed");
		
		std::istringstream ss(line);
		string protocol;
		unsigned int code = 0;
		ss >> protocol >> code;
		if(code != 101) throw std::runtime_error("Unexpected response code for WebSocket: " + to_string(code));

		while(true) 
		{
			if(!mStream->readLine(line)) throw std::runtime_error("Connection unexpectedly closed");
			if(line.empty()) break;
			// TODO
			//if(response.headers["Upgrade"] != "websocket")
			//	throw std::runtime_error("WebSocket upgrade header mismatch");

			// We don't bother verifying Sec-WebSocket-Accept
		}
	}
	catch(...)
	{
		delete mStream;
		mStream = NULL;
		throw;
	}
}

void WebSocket::close(void)
{
	if(mStream)
	{
		sendFrame(CLOSE, NULL, 0, false);
		delete mStream;
		mStream = NULL;
	}
}

size_t WebSocket::readSome(char *buffer, size_t size)
{
	size_t length = 0;
	bool fin = false;
	while(!fin) 
	{
		int len = recvFrame(buffer + length, size - length, fin);
		if(len < 0) return 0;
		length+= size_t(len);
	}
	return length;
}

size_t WebSocket::writeSome(const char *data, size_t size)
{
	return sendFrame(BINARY_FRAME, data, size, true);
}

int WebSocket::recvFrame(char *buffer, size_t size, bool &fin)
{
	if(!mStream) throw std::runtime_error("WebSocket closed");

	while(true)
	{
		binary buf;
		if(!mStream->read(buf, 2)) return -1;

		uint8_t b1, b2;
		if(!(BinaryFormatter(buf) >> b1 >> b2))
			throw std::runtime_error("Connection unexpectedly closed");

		fin = (b1 & 0x80) != 0;
		bool mask = (b2 & 0x80) != 0;
		uint8_t opcode = b1 & 0x0F;
		uint64_t length = b2 & 0x7F;

		if(length == 0x7E)
		{
			mStream->read(buf, 2);
			uint16_t extLen;
			if(!(BinaryFormatter(buf) >> extLen))
				throw std::runtime_error("Connection unexpectedly closed");
			length = extLen;
		}
		else if(length == 0x7F)
		{
			mStream->read(buf, 4);
			uint64_t extLen;
			if(!(BinaryFormatter(buf) >> extLen))
				throw std::runtime_error("Connection unexpectedly closed");
			length = extLen;
		}
		
		binary maskKey;
		if(mask && mStream->read(maskKey, 4) < 4)
			throw std::runtime_error("Connection unexpectedly closed");

		switch(opcode)
		{
			case TEXT_FRAME:
			case BINARY_FRAME:
			case CONTINUATION:
			{
				size_t s = std::min(size, length);
				mStream->read(buffer, s);
				if(length > s) mStream->ignore(length - s);
				if(mask)
				{
					for(size_t i = 0; i < s; ++i)
						buffer[i]^= maskKey[i%4];
				}
				return int(s);
			}
			
			case PING:
			{
				const size_t s = MAX_PING_PAYLOAD_SIZE;
				char payload[s];
				mStream->read(payload, s);
				if(length > s) mStream->ignore(length - s);
				if(mask)
				{
					for(size_t i = 0; i < s; ++i)
						payload[i]^= maskKey[i%4];
				}
				sendFrame(PONG, payload, s, true);
				break;
			}
			
			case PONG:
			{
				break;
			}
			
			case CLOSE:
			{ 
				close(); 
				return -1;
			}
			
			default:
			{
				close();
				throw std::invalid_argument("Invalid WebSocket opcode");
			}
		}
	}
}

int WebSocket::sendFrame(uint8_t opcode, const char *data, size_t size, bool fin)
{
	if(!mStream) throw std::runtime_error("WebSocket is closed");

	BinaryFormatter frame;
	frame << uint8_t((opcode & 0x0F) | (fin ? 0x80 : 0));
	
	unsigned len = size;
	if(len < 0x7E)
	{
		frame << uint8_t((len & 0x7F) | (mSendMask ? 0x80 : 0));
	}
	else if(len <= 0xFF)
	{
		frame << uint8_t(0x7E | (mSendMask ? 0x80 : 0));
		frame << uint16_t(len);
	}
	else {
		frame << uint8_t(0x7F | (mSendMask ? 0x80 : 0));
		frame << uint64_t(len);
	}
	
	binary content(data, data + size);
	if(mSendMask)
	{
		binary maskKey;
		Random().read(maskKey, 4);
		frame << maskKey;
		
		for(size_t i = 0; i < size; ++i)
			content[i]^= maskKey[i%4];
	}
	
	frame << content;

	mStream->write(frame.data());
	return int(size);
}

}
