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

#include "pla/serversocket.hpp"
#include "pla/string.hpp"

namespace pla
{

ServerSocket::ServerSocket(void) :
	mSock(INVALID_SOCKET),
	mPort(0)
{

}

ServerSocket::ServerSocket(int port) :
	mSock(INVALID_SOCKET),
	mPort(0)
{
	listen(port);
}

ServerSocket::~ServerSocket(void)
{
	NOEXCEPTION(close());
}

void ServerSocket::listen(int port)
{
	close();
	mPort = port;
	mSock = INVALID_SOCKET;

	// Obtain local Address
	addrinfo *ailist = NULL;
	addrinfo aiHints;
	std::memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_UNSPEC;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = 0;
	aiHints.ai_flags = AI_PASSIVE;
	std::ostringstream ss;
	ss << port;
	if(getaddrinfo(NULL, ss.str().c_str(), &aiHints, &ailist) != 0)
		throw std::runtime_error("Local binding address resolution failed");

	try {
		// Prefer IPv6
		addrinfo *ai = ailist;
		while(ai && ai->ai_family != AF_INET6)
			ai = ai->ai_next;
		if(!ai) ai = ailist;

		// Create socket
		mSock = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if(mSock == INVALID_SOCKET)
		{
			addrinfo *first = ai;
			ai = ailist;
			while(ai)
			{
				if(ai != first) mSock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
				if(mSock != INVALID_SOCKET) break;
				ai = ai->ai_next;
			}
			if(!ai) throw std::runtime_error("Socket creation failed");
		}

		// Set options
		int enabled = 1;
		int disabled = 0;
		setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&enabled), sizeof(enabled));
		if(ai->ai_family == AF_INET6)
			setsockopt(mSock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&disabled), sizeof(disabled));

		// Bind it
		if(::bind(mSock, ai->ai_addr, ai->ai_addrlen) != 0)
			throw std::runtime_error("Binding failed on port " + to_string(port));

		// Listen
		if(::listen(mSock, SOMAXCONN) != 0)
			throw std::runtime_error("Listening failed on port " + to_string(port));

		// Set non-blocking mode
		ctl_t b = 1;
		if(ioctl(mSock, FIONBIO, &b) < 0)
			throw std::runtime_error("Unable to set socket non-blocking mode");
	}
	catch(...)
	{
		freeaddrinfo(ailist);
		close();
		throw;
	}

	freeaddrinfo(ailist);
}

void ServerSocket::close(void)
{
	if(mSock != INVALID_SOCKET)
	{
		::closesocket(mSock);
		mSock = INVALID_SOCKET;
		mPort = 0;
	}
}

bool ServerSocket::isClosed(void) const
{
	return (mSock == INVALID_SOCKET);
}

bool ServerSocket::accept(Socket &sock)
{
	if(mSock == INVALID_SOCKET) throw std::runtime_error("Socket is closed");

	socket_t clientSock = ::accept(mSock, NULL, NULL);
	if(clientSock == INVALID_SOCKET) return false;

	sock.close();
	sock.mSock = clientSock;
	return true;
}

int ServerSocket::getPort(void) const
{
	if(mSock != INVALID_SOCKET) return mPort;
	else return 0;
}

Address ServerSocket::getBindAddress(void) const
{
	sockaddr_storage sa;
	socklen_t sl = sizeof(sa);
	int ret = ::getsockname(mSock, reinterpret_cast<sockaddr*>(&sa), &sl);
	if(ret < 0) throw std::runtime_error("Cannot obtain address of socket");

	return Address(reinterpret_cast<sockaddr*>(&sa), sl);
}

void ServerSocket::getLocalAddresses(std::set<Address> &set) const
{
	set.clear();
	Address bindAddr = getBindAddress();

#ifdef NO_IFADDRS
	// Retrieve hostname
	char hostname[HOST_NAME_MAX];
	if(::gethostname(hostname,HOST_NAME_MAX))
		throw std::runtime_error("Cannot retrieve hostname");

	// Resolve hostname
	addrinfo *ailist = NULL;
	addrinfo aiHints;
	std::memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_UNSPEC;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = 0;
	aiHints.ai_flags = 0;
	string service = to_string(mPort);
	if(getaddrinfo(hostname, service.c_str(), &aiHints, &ailist) != 0)
	{
		LogDebug("ServerSocket", "Local hostname is not resolvable");
		if(getaddrinfo("localhost", service.c_str(), &aiHints, &ailist) != 0)
		{
			set.insert(bindAddr);
			return;
		}
	}

	addrinfo *ai = ailist;
	while(ai)
	{
		if(ai->ai_family == AF_INET || ai->ai_family == AF_INET6)
		{
			Address addr(ai->ai_addr,ai->ai_addrlen);
			string host = addr.host();

			if(ai->ai_addr->sa_family != AF_INET6 || host.substr(0,4) != "fe80")
			{
				if(addr == bindAddr)
				{
					set.clear();
					set.insert(addr);
					break;
				}

				set.insert(addr);
			}
		}

		ai = ai->ai_next;
	}

	freeaddrinfo(ailist);
#else
	ifaddrs *ifas = NULL;
	if(getifaddrs(&ifas) < 0)
		throw std::runtime_error("Unable to list network interfaces");

	ifaddrs *ifa = ifas;
	while(ifa)
	{
		sockaddr *sa = ifa->ifa_addr;
		if(sa)
		{
			socklen_t len = 0;
			switch(sa->sa_family)
			{
				case AF_INET:  len = sizeof(sockaddr_in);  break;
				case AF_INET6: len = sizeof(sockaddr_in6); break;
			}

			if(len)
			{
				Address addr(sa, len);
				string host = addr.host();
				if(host.substr(0,4) != "fe80")
				{
					addr.set(host, mPort);
					if(addr == bindAddr)
					{
						set.clear();
						set.insert(addr);
						break;
					}
					set.insert(addr);
				}
			}
		}

		ifa = ifa->ifa_next;
	}

	freeifaddrs(ifas);
#endif
}

}
