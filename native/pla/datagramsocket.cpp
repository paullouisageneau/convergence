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

#include "pla/datagramsocket.hpp"
#include "pla/string.hpp"

namespace pla
{

const size_t DatagramSocket::MaxDatagramSize = 1500;

DatagramSocket::DatagramSocket(int port, bool broadcast) :
		mSock(INVALID_SOCKET)
{
	bind(port, broadcast);
}

DatagramSocket::DatagramSocket(const Address &local, bool broadcast) :
		mSock(INVALID_SOCKET)
{
	bind(local, broadcast);
}

DatagramSocket::~DatagramSocket(void)
{
	NOEXCEPTION(close());
}

Address DatagramSocket::getBindAddress(void) const
{
	char hostname[HOST_NAME_MAX];
	if(gethostname(hostname,HOST_NAME_MAX))
		throw std::runtime_error("Cannot retrieve hostname");

	sockaddr_storage sa;
	socklen_t sl = sizeof(sa);
	int ret = getsockname(mSock, reinterpret_cast<sockaddr*>(&sa), &sl);
	if(ret < 0) throw std::runtime_error("Cannot obtain Address of socket");

	return Address(reinterpret_cast<sockaddr*>(&sa), sl);
}

void DatagramSocket::getLocalAddresses(std::set<Address> &set) const
{
	set.clear();

	Address bindAddr = getBindAddress();

#ifdef NO_IFADDRS
	// Retrieve hostname
	char hostname[HOST_NAME_MAX];
	if(gethostname(hostname,HOST_NAME_MAX))
		throw std::runtime_error("Cannot retrieve hostname");

	// Resolve hostname
	addrinfo *aiList = NULL;
	addrinfo aiHints;
	memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_UNSPEC;
	aiHints.ai_socktype = SOCK_DGRAM;
	aiHints.ai_protocol = 0;
	aiHints.ai_flags = 0;
	string service = to_string(mPort);
	if(getaddrinfo(hostname, service.c_str(), &aiHints, &aiList) != 0)
	{
		LogDebug("DatagramSocket", "Local hostname is not resolvable");
		if(getaddrinfo("localhost", service.c_str(), &aiHints, &aiList) != 0)
		{
			set.insert(bindAddr);
			return;
		}
	}

	addrinfo *ai = aiList;
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

	freeaddrinfo(aiList);
#else
	ifaddrs *ifas;
	if(getifaddrs(&ifas) < 0)
		throw std::runtime_error("Unable to list network interfaces");

	ifaddrs *ifa = ifas;
	while(ifa)
	{
		sockaddr *sa = ifa->ifa_addr;
		if(!sa) break;

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
			if(sa->sa_family != AF_INET6 || host.substr(0,4) != "fe80")
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

		ifa = ifa->ifa_next;
	}

	freeifaddrs(ifas);
#endif
}

void DatagramSocket::getHardwareAddresses(std::set<binary> &set) const
{
	set.clear();

#ifdef WINDOWS
	IP_ADAPTER_ADDRESSES adapterInfo[16];
	DWORD dwBufLen = sizeof(IP_ADAPTER_ADDRESSES)*16;

	DWORD dwStatus = GetAdaptersAddresses(getBindAddress().addrFamily(), 0, NULL, adapterInfo, &dwBufLen);
	if(dwStatus != ERROR_SUCCESS)
		throw std::runtime_error("Unable to retrive hardware addresses");

	IP_ADAPTER_ADDRESSES *pAdapterInfo = adapterInfo;
	while(pAdapterInfo)
	{
		if(pAdapterInfo->PhysicalAddressLength)
		{
			auto data = reinterpret_cast<const byte *>(pAdapterInfo->PhysicalAddress);
			size_t size = pAdapterInfo->PhysicalAddressLengthconst;
			set.insert(binary(data, data + size));
		}
		pAdapterInfo = pAdapterInfo->Next;
	}
#else
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[BufferSize];

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if(ioctl(mSock, SIOCGIFCONF, &ifc) == -1)
		throw std::runtime_error("Unable to retrive hardware addresses");

	for(struct ifreq* it = ifc.ifc_req;
		it != ifc.ifc_req + ifc.ifc_len/sizeof(struct ifreq);
		++it)
	{
		strcpy(ifr.ifr_name, it->ifr_name);

		if(ioctl(mSock, SIOCGIFFLAGS, &ifr) == 0)
		{
			if(!(ifr.ifr_flags & IFF_LOOPBACK))
			{
				if(ioctl(mSock, SIOCGIFHWADDR, &ifr) == 0)
				{
					// Note: hwaddr.sa_data is big endian
					auto *data = reinterpret_cast<const byte *>(ifr.ifr_hwaddr.sa_data);
					size_t size = IFHWADDRLEN;
					set.insert(binary(data, data + size));
				}
			}
		}
	}
#endif
}

void DatagramSocket::bind(int port, bool broadcast, int family)
{
	close();
	mPort = port;

	// Obtain local Address
	addrinfo *aiList = NULL;
	addrinfo aiHints;
	std::memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = family;
	aiHints.ai_socktype = SOCK_DGRAM;
	aiHints.ai_protocol = 0;
	aiHints.ai_flags = AI_PASSIVE;
	std::ostringstream ss;
	ss << port;
	if(getaddrinfo(NULL, ss.str().c_str(), &aiHints, &aiList) != 0)
		throw std::runtime_error("Local binding address resolution failed for UDP port "+ss.str());

	try {
		// Prefer IPv6
		addrinfo *ai = aiList;
		while(ai && ai->ai_family != AF_INET6)
			ai = ai->ai_next;
		if(!ai) ai = aiList;

		// Create socket
		mSock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if(mSock == INVALID_SOCKET)
		{
			addrinfo *first = ai;
			ai = aiList;
			while(ai)
			{
				if(ai != first) mSock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
				if(mSock != INVALID_SOCKET) break;
				ai = ai->ai_next;
			}
			if(!ai) throw std::runtime_error("Datagram socket creation failed");
		}

		// Set options
		int enabled = 1;
		int disabled = 0;
		setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&enabled), sizeof(enabled));
		if(broadcast) setsockopt(mSock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&enabled), sizeof(enabled));
		if(ai->ai_family == AF_INET6)
			setsockopt(mSock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&disabled), sizeof(disabled));

		// Necessary for DTLS
#ifdef LINUX
		int val = IP_PMTUDISC_DO;
		setsockopt(mSock, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));
#else
		setsockopt(mSock, IPPROTO_IP, IP_DONTFRAG, reinterpret_cast<char*>(&enabled), sizeof(enabled));
#endif

		// Bind it
		if(::bind(mSock, ai->ai_addr, ai->ai_addrlen) != 0)
			throw std::runtime_error("Binding failed on UDP port " + to_string(port));

		/*
		ctl_t b = 1;
		if(ioctl(mSock,FIONBIO,&b) < 0)
		throw std::runtime_error("Cannot use non-blocking mode");
		 */
	}
	catch(...)
	{
		freeaddrinfo(aiList);
		close();
		throw;
	}

	freeaddrinfo(aiList);
}

void DatagramSocket::bind(const Address &local, bool broadcast)
{
	close();

	try {
		mPort = local.port();

		// Create datagram socket
		mSock = socket(local.addrFamily(), SOCK_DGRAM, 0);
		if(mSock == INVALID_SOCKET)
			throw std::runtime_error("Datagram socket creation failed");

		// Set options
		int enabled = 1;
		setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&enabled), sizeof(enabled));
		if(broadcast) setsockopt(mSock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&enabled), sizeof(enabled));

		// Bind it
		if(::bind(mSock, local.addr(), local.addrLen()) != 0)
			throw std::runtime_error("Binding failed on " + local.toString());

		/*
		ctl_t b = 1;
		if(ioctl(mSock,FIONBIO,&b) < 0)
		throw std::runtime_error("Cannot use non-blocking mode");
		 */
	}
	catch(...)
	{
		close();
		throw;
	}
}

void DatagramSocket::close(void)
{
	std::unique_lock<std::mutex> lock(mStreamsMutex);

	for(auto it = mStreams.begin(); it != mStreams.end(); ++it)
	{
		DatagramStream *stream = it->second;
		std::unique_lock<std::mutex> lock(stream->mMutex);
		stream->mSock = NULL;
		stream->mCondition.notify_all();
	}

	mStreams.clear();

	if(mSock != INVALID_SOCKET)
	{
		::closesocket(mSock);
		mSock = INVALID_SOCKET;
		mPort = 0;
	}
}

int DatagramSocket::read(byte *buffer, size_t size, Address &sender, duration timeout) {
	return recv(buffer, size, sender, timeout, 0);
}

int DatagramSocket::peek(byte *buffer, size_t size, Address &sender, duration timeout) {
	return recv(buffer, size, sender, timeout, MSG_PEEK);
}

int DatagramSocket::write(const byte *data, size_t size, const Address &receiver) {
	return send(data, size, receiver, 0);
}

bool DatagramSocket::read(binary &buffer, Address &sender, duration timeout)
{
	buffer.resize(MaxDatagramSize);
	int size = read(buffer.data(), buffer.size(), sender, timeout);
	buffer.resize(size > 0 ? size : 0);
	return (size >= 0);
}

bool DatagramSocket::peek(binary &buffer, Address &sender, duration timeout)
{
	buffer.resize(MaxDatagramSize);
	int size = peek(buffer.data(), buffer.size(), sender, timeout);
	buffer.resize(size > 0 ? size : 0);
	return (size >= 0);
}

void DatagramSocket::write(const binary &data, const Address &receiver)
{
	write(data.data(), data.size(), receiver);
}

bool DatagramSocket::wait(duration timeout)
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(mSock, &readfds);

	struct timeval tv;
	durationToStruct(timeout, tv);
	int ret = ::select(SOCK_TO_INT(mSock)+1, &readfds, NULL, NULL, &tv);
	if (ret < 0) throw std::runtime_error("Unable to wait on socket");
	return (ret > 0);
}

int DatagramSocket::recv(byte *buffer, size_t size, Address &sender, duration timeout, int flags) {
	using clock = std::chrono::steady_clock;
	std::chrono::time_point<clock> end;
	if(timeout >= duration::zero()) end = clock::now() + std::chrono::duration_cast<clock::duration>(timeout);
	else end = std::chrono::time_point<clock>::max();

	do {
		duration left = end - std::chrono::steady_clock::now();
		if(!wait(left)) break;

		char datagramBuffer[MaxDatagramSize];
		sockaddr_storage sa;
		socklen_t sl = sizeof(sa);
		int result = ::recvfrom(mSock, datagramBuffer, MaxDatagramSize, flags | MSG_PEEK, reinterpret_cast<sockaddr*>(&sa), &sl);
		if(result < 0) throw std::runtime_error("Unable to read from socket (error " + to_string(sockerrno) + ")");
		sender.set(reinterpret_cast<sockaddr*>(&sa),sl);
		Address key(sender.unmap());

		std::unique_lock<std::mutex> lock(mStreamsMutex);
		auto it = mStreams.lower_bound(key);
		if(it == mStreams.end())
		{
			size = std::min(result, int(size));
			std::memcpy(buffer, datagramBuffer, size);

			if(!(flags & MSG_PEEK)) ::recvfrom(mSock, datagramBuffer, MaxDatagramSize, flags, reinterpret_cast<sockaddr*>(&sa), &sl);
			return size;
		}

		while(it != mStreams.end() && it->first == key)
	       	{
			DatagramStream *stream = it->second;

			std::unique_lock<std::mutex> lock(stream->mMutex);
			if (stream->mIncoming.size() < DatagramStream::MaxQueueSize) {
				auto b = reinterpret_cast<const byte *>(datagramBuffer);
				stream->mIncoming.push(binary(b, b + result));
			}

			stream->mCondition.notify_all();
			++it;
		}

		::recvfrom(mSock, datagramBuffer, MaxDatagramSize, flags & ~MSG_PEEK, reinterpret_cast<sockaddr*>(&sa), &sl);
	}
	while(std::chrono::steady_clock::now() <= end);

	return -1;
}

int DatagramSocket::send(const byte *buffer, size_t size, const Address &receiver, int flags) {
	int result = ::sendto(mSock, reinterpret_cast<const char *>(buffer), size, flags,
	                      receiver.addr(), receiver.addrLen());
	if(result < 0) throw std::runtime_error("Unable to write to socket (error " + to_string(sockerrno) + ")");
	return result;
}

void DatagramSocket::accept(DatagramStream &stream)
{
	binary buffer;
	Address sender;
	read(buffer, sender);

	unregisterStream(&stream);

	stream.mSock = this;
	stream.mAddr = sender;
}

void DatagramSocket::registerStream(DatagramStream *stream)
{
	Assert(stream);
	Address key(stream->mAddr.unmap());

	std::unique_lock<std::mutex> lock(mStreamsMutex);
	mStreams.insert(std::make_pair(key, stream));
}

void DatagramSocket::unregisterStream(DatagramStream *stream)
{
	Assert(stream);
	Address key(stream->mAddr.unmap());

	std::unique_lock<std::mutex> lock(mStreamsMutex);
	auto it = mStreams.lower_bound(key);
	while(it != mStreams.end() && it->first == key)
	{
		if(it->second == stream)
		{
			mStreams.erase(it);
			break;
		}
		++it;
	}
}

duration DatagramStream::DefaultTimeout = seconds(60.);
int DatagramStream::MaxQueueSize = 100;

DatagramStream::DatagramStream(void) :
	mSock(NULL),
	mTimeout(DefaultTimeout)
{

}

DatagramStream::DatagramStream(DatagramSocket *sock, const Address &addr) :
	mSock(sock),
	mAddr(addr),
	mTimeout(DefaultTimeout)
{
	Assert(mSock);
	mSock->registerStream(this);
}

DatagramStream::~DatagramStream(void)
{
	close();
}

Address DatagramStream::getLocalAddress(void) const
{
	// Warning: this is actually different from local address
	if(mSock) return mSock->getBindAddress();
	else return Address();
}

Address DatagramStream::getRemoteAddress(void) const
{
	return mAddr;
}

void DatagramStream::setTimeout(duration timeout)
{
	mTimeout = timeout;
}

size_t DatagramStream::readSome(byte *buffer, size_t size) {
	std::unique_lock<std::mutex> lock(mMutex);

	if(!mCondition.wait_for(lock, mTimeout, [this]() {
		return (!mSock || !mIncoming.empty());
	}))
		throw timeout();

	if(mIncoming.empty()) return 0;
	size = std::min(size, mIncoming.front().size());
	std::memcpy(buffer, mIncoming.front().data(), size);
	mIncoming.pop();
	return size;
}

size_t DatagramStream::writeSome(const byte *data, size_t size) {
	std::unique_lock<std::mutex> lock(mMutex);

	if(!mSock) throw std::runtime_error("Datagram stream closed");
	int written = mSock->write(data, size, mAddr);
	return size_t(written);
}

bool DatagramStream::wait(duration timeout)
{
	std::unique_lock<std::mutex> lock(mMutex);

	return mCondition.wait_for(lock, timeout, [this]() {
		return (!mSock || !mIncoming.empty());
	});
}

void DatagramStream::close(void)
{
	std::unique_lock<std::mutex> lock(mMutex);

	if(mSock)
	{
		while(!mIncoming.empty()) mIncoming.pop();
		mSock->unregisterStream(this);
		mSock = NULL;
	}

	mCondition.notify_all();
}

bool DatagramStream::isMessage(void) const
{
	return true;
}

}
