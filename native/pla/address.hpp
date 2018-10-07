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

#ifndef PLA_ADDRESS_H
#define PLA_ADDRESS_H

#include "pla/include.hpp"
#include "pla/string.hpp"
#include "pla/binary.hpp"

namespace pla
{

class Address
{
public:
	static bool Resolve(const string &host, const string &service, std::list<Address> &result);
	static bool Resolve(const string &host, uint16_t port, std::list<Address> &result);
	static bool Resolve(const string &str, std::list<Address> &result, const string &protocol = "");
	static bool Reverse(const Address &a, string &result);
	
	Address(void);
	Address(const string &host, const string &service);
	Address(const string &host, uint16_t port);
	Address(const string &str);
	Address(const sockaddr *addr, socklen_t addrlen);
	~Address(void);

	void set(const string &host, const string &service, int family = AF_UNSPEC, int socktype = 0);
	void set(const string &host, uint16_t port, int family = AF_UNSPEC, int socktype = 0);
	void set(const string &str);
	void set(const sockaddr *addr, socklen_t addrlen = 0);
	void setPort(uint16_t port);
	void clear(void);
	bool isEmpty(void) const;
	bool isLocal(void) const;
	bool isPrivate(void) const;
	bool isPublic(void) const;
	bool isIpv4(void) const;
	bool isIpv6(void) const;

	string host(bool numeric = true) const;
	string service(bool numeric = true) const;
	uint16_t port(void) const;
	string reverse(bool numericHost = false) const;
	Address unmap(void) const;

	const sockaddr *addr(void) const;
	int addrFamily(void) const;
	socklen_t addrLen(void) const;

	// Serializable
	string toString() const;
	void fromString(string str);
	binary toBinary() const;
	void fromBinary(binary b);

private:
	sockaddr_storage	mAddr;
	socklen_t		mAddrLen;
};

bool operator <  (const Address &a1, const Address &a2);
bool operator >  (const Address &a1, const Address &a2);
bool operator == (const Address &a1, const Address &a2);
bool operator != (const Address &a1, const Address &a2);

}

#endif
