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

#ifndef PLA_WEBSOCKET_H
#define PLA_WEBSOCKET_H

#include "pla/include.hpp"
#include "pla/stream.hpp"
#include "pla/string.hpp"
#include "pla/binary.hpp"

namespace pla
{

// WebSocket implementation
class WebSocket : public Stream
{
public:
	WebSocket(void);
	WebSocket(const string &url);
	WebSocket(Stream *stream, bool disableMask = false);
	~WebSocket(void);

	void connect(const string &url);
	void close(void);

	size_t readSome(byte *buffer, size_t size);
	size_t writeSome(const byte *data, size_t size);
	bool isMessage(void) const { return true; }

protected:
	int recvFrame(byte *buffer, size_t size, bool &fin);
	int sendFrame(uint8_t opcode, const byte *data, size_t size, bool fin = true);

private:
	Stream *mStream;
	bool mSendMask;
};

}

#endif
