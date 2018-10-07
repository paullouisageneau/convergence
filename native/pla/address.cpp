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

#include "pla/address.hpp"
#include "pla/binaryformatter.hpp"

namespace pla
{
	
bool Address::Resolve(const string &host, const string &service, std::list<Address> &result)
{
	result.clear();

	addrinfo aiHints;
	std::memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_UNSPEC;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = 0;
	if(host.find(':') != string::npos || !hasalpha(host)) aiHints.ai_flags = AI_NUMERICHOST;
	else aiHints.ai_flags = AI_ADDRCONFIG;

	addrinfo *ailist = NULL;
	if(getaddrinfo(host.c_str(), service.c_str(), &aiHints, &ailist) != 0)
		return false;

	addrinfo *ai = ailist;
	while(ai)
	{
		result.push_back(Address(ai->ai_addr, ai->ai_addrlen));
		ai = ai->ai_next;
	}

	freeaddrinfo(ailist);
	return !result.empty();
}

bool Address::Resolve(const string &host, uint16_t port, std::list<Address> &result)
{
	std::ostringstream ss;
	ss << port;
	return Resolve(host, ss.str(), result);
}

bool Address::Resolve(const string &str, std::list<Address> &result, const string &protocol)
{
	result.clear();

	string host, service;
	int separator = str.find_last_of(':');
	if(separator != string::npos && str.substr(separator+1).find(']') == string::npos)
	{
		host = str.substr(0, separator);
		service = str.substr(separator+1);
	}
	else {
		host = str;
		service = !protocol.empty() ? tolower(protocol) : "http";
	}

	if(!host.empty() && host[0] == '[')
		host = host.substr(1, host.find(']')-1);

	if(host.empty() || service.empty())
		return false;

	return Resolve(host, service, result);
}

bool Address::Reverse(const Address &a, string &result)
{
	if(a.isEmpty()) return "";

	char host[HOST_NAME_MAX];
	char service[SERVICE_NAME_MAX];
	if(getnameinfo(a.addr(), a.addrLen(), host, HOST_NAME_MAX, service, SERVICE_NAME_MAX, NI_NUMERICSERV))
	{
		result = a.toString();
		return false;
	}

	std::ostringstream ss;
	if(a.addrFamily() == AF_INET6 && string(host).find(':') != string::npos)
	{
		ss << '[' << host << ']' << ':' << service;
	}
	else {
		ss << host << ':' << service;
	}
	result = ss.str();
	return true;
}

Address::Address(void)
{
	clear();
}

Address::Address(const string &host, const string &service)
{
	set(host,service);
}

Address::Address(const string &host, uint16_t port)
{
	set(host,port);
}

Address::Address(const string &str)
{
	set(str);
}

Address::Address(const sockaddr *addr, socklen_t addrlen)
{
	set(addr,addrlen);
}

Address::~Address(void)
{

}

void Address::set(const string &host, const string &service, int family, int socktype)
{
	addrinfo aiHints;
	std::memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = family;
	aiHints.ai_socktype = socktype;
	aiHints.ai_protocol = 0;
	if(host.find(':') != string::npos || !hasalpha(host)) aiHints.ai_flags = AI_NUMERICHOST;
	else aiHints.ai_flags = AI_ADDRCONFIG;

	addrinfo *ailist = NULL;
	if(getaddrinfo(host.c_str(), service.c_str(), &aiHints, &ailist) != 0)
		throw std::runtime_error("Unable to resolve address");

	set(ailist->ai_addr, ailist->ai_addrlen);
	freeaddrinfo(ailist);
}

void Address::set(const string &host, uint16_t port, int family, int socktype)
{
	std::ostringstream ss;
	ss << port;
	set(host, ss.str(), family);
}

void Address::set(const string &str)
{
	fromString(str);
}

void Address::set(const sockaddr *addr, socklen_t addrlen)
{
	if(!addr || !addrlen)
	{
		std::memset(&mAddr, 0, sizeof(mAddr));
		mAddrLen = 0;
		return;
	}

	mAddrLen = addrlen;
	std::memcpy(&mAddr, addr, addrlen);
}

void Address::setPort(uint16_t port)
{
	if(isEmpty()) return;

	switch(addrFamily())
	{
	case AF_INET:	// IPv4
	{
		sockaddr_in *sa = reinterpret_cast<sockaddr_in*>(&mAddr);
		sa->sin_port = port;
		break;
	}

	case AF_INET6:	// IPv6
	{
		sockaddr_in6 *sa = reinterpret_cast<sockaddr_in6*>(&mAddr);
		sa->sin6_port = port;
		break;
	}

	default:
		throw std::runtime_error("Unable to set the port for this address family");
	}
}

void Address::clear(void)
{
	const sockaddr *null = NULL;
	set(null, 0);
}

bool Address::isEmpty(void) const
{
	return (mAddrLen == 0);
}

bool Address::isLocal(void) const
{
	switch(addrFamily())
	{
	case AF_INET:	// IPv4
	{
		const sockaddr_in *sa = reinterpret_cast<const sockaddr_in*>(&mAddr);
		const uint8_t *b = reinterpret_cast<const uint8_t *>(&sa->sin_addr.s_addr);
		if(b[0] == 127) return true;
		break;
	}

	case AF_INET6:	// IPv6
	{
		const sockaddr_in6 *sa6 = reinterpret_cast<const sockaddr_in6*>(&mAddr);
		const uint8_t *b = reinterpret_cast<const uint8_t *>(sa6->sin6_addr.s6_addr);
		for(int i=0; i<9; ++i) if(b[i] != 0) break;
		if(b[10] == 0xFF && b[11] == 0xFF)		// mapped
		{
			if(b[12] == 127) return true;
		}
		for(int i=10; i<15; ++i) if(b[i] != 0) break;
		if(b[15] == 1) return true;
		break;
	}

	}
	return false;
}

bool Address::isPrivate(void) const
{
	switch(addrFamily())
	{
	case AF_INET:	// IPv4
	{
		const sockaddr_in *sa = reinterpret_cast<const sockaddr_in*>(&mAddr);
		const uint8_t *b = reinterpret_cast<const uint8_t *>(&sa->sin_addr.s_addr);
		if(b[0] == 10) return true;
		if(b[0] == 172 && b[1] >= 16 && b[1] < 32) return true;
		if(b[0] == 192 && b[1] == 168) return true;
		if(b[0] == 169 && b[1] == 254) return true;	// link-local
		break;
	}

	case AF_INET6:	// IPv6
	{
		const sockaddr_in6 *sa6 = reinterpret_cast<const sockaddr_in6*>(&mAddr);
		const uint8_t *b = reinterpret_cast<const uint8_t *>(sa6->sin6_addr.s6_addr);
		if(b[0] == 0xFC || b[0] == 0xFD) return true;
		for(int i=0; i<9; ++i) if(b[i] != 0) break;
		if(b[10] == 0xFF && b[11] == 0xFF)		// mapped
		{
			if(b[12] == 10) return true;
			if(b[12] == 172 && b[13] >= 16 && b[13] < 32) return true;
			if(b[12] == 192 && b[13] == 168) return true;
		}
		break;
	}

	}
	return false;
}

bool Address::isPublic(void) const
{
	return !isLocal() && !isPrivate();
}

bool Address::isIpv4(void) const
{
	return (addrFamily() == AF_INET);
}

bool Address::isIpv6(void) const
{
	return (addrFamily() == AF_INET6);
}

string Address::host(bool numeric) const
{
	if(isEmpty()) return "";

	char host[HOST_NAME_MAX];
	if(getnameinfo(addr(), addrLen(), host, HOST_NAME_MAX, NULL, 0, (numeric ?  NI_NUMERICHOST : 0)))
		throw std::invalid_argument("Invalid stored network address");
	return tolower(string(host));
}

string Address::service(bool numeric) const
{
	if(isEmpty()) return "";

	char service[SERVICE_NAME_MAX];
	if(getnameinfo(addr(), addrLen(), NULL, 0, service, SERVICE_NAME_MAX, (numeric ? NI_NUMERICSERV : 0)))
		throw std::invalid_argument("Invalid stored network address");
	return tolower(string(service));
}

uint16_t Address::port(void) const
{
	if(isEmpty()) return 0;
	return uint16_t(std::stoul(service(true)));	// numeric
}

string Address::reverse(bool numericHost) const
{
	if(isEmpty()) return "";

	char host[HOST_NAME_MAX];
	char service[SERVICE_NAME_MAX];
	if(getnameinfo(addr(), addrLen(), host, HOST_NAME_MAX, service, SERVICE_NAME_MAX, (numericHost ?  NI_NUMERICHOST : 0) | NI_NUMERICSERV))
		throw std::invalid_argument("Invalid stored network address");

	std::ostringstream ss;
	if(addrFamily() == AF_INET6 && string(host).find(':') != string::npos)
	{
		ss << '[' << host << ']' << ':' << service;
	}
	else {
		ss << host << ':' << service;
	}
	return ss.str();
}

Address Address::unmap(void) const
{
	if(addrFamily() == AF_INET6)
	{
		const sockaddr_in6 *sa6 = reinterpret_cast<const sockaddr_in6*>(&mAddr);
		const uint8_t *b = reinterpret_cast<const uint8_t *>(sa6->sin6_addr.s6_addr);

		for(int i=0; i<9; ++i)
			if(b[i] != 0)
				return *this;

		if(b[10] != 0xFF || b[11] != 0xFF)
			return *this;

		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = sa6->sin6_port;
		std::memcpy(reinterpret_cast<uint8_t*>(&sin.sin_addr.s_addr), b + 12, 4);

		return Address(reinterpret_cast<sockaddr*>(&sin), sizeof(sin));
	}

	return *this;
}

string Address::toString(void) const
{
	Address a(unmap());
	return a.reverse(true);
}

void Address::fromString(string str)
{
	str = trim(str);
	if(str.empty()) {
		clear();
		return;
	}

	string host, service;
	int separator = str.find_last_of(':');
	if(separator != string::npos && str.substr(separator+1).find(']') == string::npos)
	{
		host = str.substr(0,separator);
		service = str.substr(separator+1);
	}
	else {
		host = str;
		service = "80";
	}

	if(!host.empty() && host[0] == '[')
		host = host.substr(1, host.find(']')-1);

	if(host.empty() || service.empty())
		throw std::invalid_argument("Invalid network address: " + str);

	set(host, service, AF_UNSPEC);
}

binary Address::toBinary(void) const
{
	BinaryFormatter formatter;
	if(isEmpty()) return formatter.data();
	
	switch(addrFamily())
	{
	case AF_INET:	// IPv4
	{
		formatter << uint8_t(4);
		const sockaddr_in *sa = reinterpret_cast<const sockaddr_in*>(&mAddr);
		const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&sa->sin_addr.s_addr);

		for(int i=0; i<4; ++i) 
			formatter << bytes[i];

		formatter << uint16_t(ntohs(sa->sin_port));
		break;
	}

	case AF_INET6:	// IPv6
	{
		formatter << uint8_t(16);
		const sockaddr_in6 *sa6 = reinterpret_cast<const sockaddr_in6*>(&mAddr);
		const uint8_t *bytes = sa6->sin6_addr.s6_addr;
		
		for(int i=0; i<16; ++i) 
			formatter << bytes[i];
		
		formatter << uint16_t(ntohs(sa6->sin6_port));
		break;
	}

	default:
		throw std::invalid_argument("Stored network address cannot be serialized");
	}

	return formatter.data();
}

void Address::fromBinary(binary b)
{
	BinaryFormatter formatter(b);
	
	uint8_t size = 0;
	 formatter >> size;

	switch(size)
	{
	case 0:
	{
		clear();
		break;
	}

	case 4:		// IPv4
	{
		mAddrLen = sizeof(sockaddr_in);
		sockaddr_in *sa = reinterpret_cast<sockaddr_in*>(&mAddr);
		uint8_t *bytes = reinterpret_cast<uint8_t *>(&sa->sin_addr.s_addr);

		sa->sin_family = AF_INET;
		for(int i=0; i<4; ++i)
		{
			uint8_t u = 0;
			formatter >> u;
			bytes[i] = u;
		}
		uint16_t port = 0;
		formatter >> port;
		sa->sin_port = htons(port);
		break;
	}

	case 16:	// IPv6
	{
		mAddrLen = sizeof(sockaddr_in6);
		sockaddr_in6 *sa6 = reinterpret_cast<sockaddr_in6*>(&mAddr);
		uint8_t *bytes = sa6->sin6_addr.s6_addr;
		
		sa6->sin6_family = AF_INET6;
		for(int i=0; i<16; ++i)
		{
			uint8_t u = 0;
			formatter >> u;
			bytes[i] = u;
		}
		uint16_t port = 0;
		formatter >> port;
		sa6->sin6_port = htons(port);
		break;
	}

	default:
		throw std::invalid_argument("Invalid network address");
	}
}
const sockaddr *Address::addr(void) const
{
	return reinterpret_cast<const sockaddr*>(&mAddr);
}

int Address::addrFamily(void) const
{
	return reinterpret_cast<const sockaddr_in*>(&mAddr)->sin_family;
}

socklen_t Address::addrLen(void) const
{
	return mAddrLen;
}

bool operator < (const Address &a1, const Address &a2)
{
	if(a1.addrLen() != a2.addrLen()) return a1.addrLen() < a2.addrLen();
	if(a1.addrFamily() != a2.addrFamily()) return a1.addrFamily() < a2.addrFamily();

	if(!a2.isLocal())
	{
		if(a1.isLocal()) return true;
		if(a1.isPrivate() && !a2.isPrivate()) return true;
	}
	if(!a1.isLocal())
	{
		if(a2.isLocal()) return false;
		if(a2.isPrivate() && !a1.isPrivate()) return false;
	}

	switch(a1.addrFamily())
	{
	case AF_INET:	// IPv4
	{
		const sockaddr_in *sa1 = reinterpret_cast<const sockaddr_in*>(a1.addr());
		const sockaddr_in *sa2 = reinterpret_cast<const sockaddr_in*>(a2.addr());
		return (sa1->sin_addr.s_addr < sa2->sin_addr.s_addr || (sa1->sin_addr.s_addr == sa2->sin_addr.s_addr && sa1->sin_port < sa2->sin_port && sa1->sin_port && sa2->sin_port));
	}

	case AF_INET6:	// IPv6
	{
		const sockaddr_in6 *sa1 = reinterpret_cast<const sockaddr_in6*>(a1.addr());
		const sockaddr_in6 *sa2 = reinterpret_cast<const sockaddr_in6*>(a2.addr());
		int cmp = std::memcmp(sa1->sin6_addr.s6_addr, sa2->sin6_addr.s6_addr, 16);
		return (cmp < 0 || (cmp == 0 && sa1->sin6_port < sa2->sin6_port && sa1->sin6_port && sa2->sin6_port));
	}

	default:
		return false;
	}
}

bool operator > (const Address &a1, const Address &a2)
{
	if(a1.addrLen() != a2.addrLen()) return a1.addrLen() > a2.addrLen();
	if(a1.addrFamily() != a2.addrFamily()) return a1.addrFamily() > a2.addrFamily();

	if(!a1.isLocal())
	{
		if(a2.isLocal()) return true;
		if(a2.isPrivate() && !a1.isPrivate()) return true;
	}
	if(!a2.isLocal())
	{
		if(a1.isLocal()) return false;
		if(a1.isPrivate() && !a2.isPrivate()) return false;
	}

	switch(a1.addrFamily())
	{
	case AF_INET:	// IPv4
	{
		const sockaddr_in *sa1 = reinterpret_cast<const sockaddr_in*>(a1.addr());
		const sockaddr_in *sa2 = reinterpret_cast<const sockaddr_in*>(a2.addr());
		return (sa1->sin_addr.s_addr > sa2->sin_addr.s_addr || (sa1->sin_addr.s_addr == sa2->sin_addr.s_addr && sa1->sin_port > sa2->sin_port && sa1->sin_port && sa2->sin_port));
	}

	case AF_INET6:	// IPv6
	{
		const sockaddr_in6 *sa1 = reinterpret_cast<const sockaddr_in6*>(a1.addr());
		const sockaddr_in6 *sa2 = reinterpret_cast<const sockaddr_in6*>(a2.addr());
		int cmp = std::memcmp(sa1->sin6_addr.s6_addr, sa2->sin6_addr.s6_addr, 16);
		return (cmp > 0 || (cmp == 0 && sa1->sin6_port > sa2->sin6_port && sa1->sin6_port && sa2->sin6_port));
	}

	default:
		return false;
	}
}

bool operator == (const Address &a1, const Address &a2)
{
	if(a1.addrLen() != a2.addrLen()) return false;
	if(a1.addrFamily() != a2.addrFamily()) return false;

	switch(a1.addrFamily())
	{
	case AF_INET:	// IPv4
	{
		const sockaddr_in *sa1 = reinterpret_cast<const sockaddr_in*>(a1.addr());
		const sockaddr_in *sa2 = reinterpret_cast<const sockaddr_in*>(a2.addr());
		return (sa1->sin_addr.s_addr == sa2->sin_addr.s_addr && (sa1->sin_port == sa2->sin_port || !sa1->sin_port || !sa2->sin_port));
	}

	case AF_INET6:	// IPv6
	{
		const sockaddr_in6 *sa1 = reinterpret_cast<const sockaddr_in6*>(a1.addr());
		const sockaddr_in6 *sa2 = reinterpret_cast<const sockaddr_in6*>(a2.addr());
		return (std::memcmp(sa1->sin6_addr.s6_addr, sa2->sin6_addr.s6_addr, 16) == 0 && (sa1->sin6_port == sa2->sin6_port || !sa1->sin6_port || !sa2->sin6_port));
	}

	default:
		return false;
	}
}

bool operator != (const Address &a1, const Address &a2)
{
	return !(a1 == a2);
}

}
