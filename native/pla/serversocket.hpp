/*************************************************************************
 *   Copyright (C) 2011-2018 by Paul-Louis Ageneau                       *
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

#ifndef PLA_SERVERSOCKET_H
#define PLA_SERVERSOCKET_H

#include "pla/include.hpp"
#include "pla/socket.hpp"

namespace pla {

class ServerSocket {
public:
	ServerSocket(void);
	ServerSocket(int port);
	~ServerSocket(void);

	void listen(int port);
	void close(void);

	bool isClosed(void) const;

	bool accept(Socket &sock);

	int getPort(void) const;
	Address getBindAddress(void) const;
	void getLocalAddresses(std::set<Address> &set) const;

private:
	socket_t mSock;
	int mPort;
};

} // namespace pla

#endif
