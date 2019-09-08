/*************************************************************************
 *   Copyright (C) 2011-2013 by Paul-Louis Ageneau                       *
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

#ifndef PLA_SOCKET_H
#define PLA_SOCKET_H

#include "pla/address.hpp"
#include "pla/include.hpp"
#include "pla/stream.hpp"

namespace pla {

class Socket : public Stream {
public:
	static Address HttpProxy;

	Socket(void);
	Socket(const Address &a, duration timeout = seconds(-1.));
	Socket(socket_t sock);
	virtual ~Socket(void);

	bool isConnected(void) const; // Does not garantee the connection isn't actually lost
	bool isReadable(void) const;
	bool isWriteable(void) const;
	Address getLocalAddress(void) const;
	Address getRemoteAddress(void) const;

	void setConnectTimeout(duration timeout);
	void setReadTimeout(duration timeout);
	void setWriteTimeout(duration timeout);
	void setTimeout(duration timeout); // connect + read + write

	void connect(const Address &addr);
	void close(void);

	// Stream
	size_t readSome(byte *buffer, size_t size);
	size_t writeSome(const byte *data, size_t size);
	bool wait(duration timeout);

	// Socket-specific
	size_t peek(byte *buffer, size_t size);

private:
	size_t recv(byte *buffer, size_t size, int flags);
	size_t send(const byte *data, size_t size, int flags);

	socket_t mSock;
	duration mConnectTimeout, mReadTimeout, mWriteTimeout;

	friend class ServerSocket;
};

} // namespace pla

#endif
