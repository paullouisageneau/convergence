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

#include "pla/socket.hpp"

namespace pla
{

Socket::Socket(void) :
		mSock(INVALID_SOCKET),
		mConnectTimeout(seconds(-1.)),
		mReadTimeout(seconds(-1.)),
		mWriteTimeout(seconds(-1.))
{

}

Socket::Socket(const Address &a, duration timeout) :
	mSock(INVALID_SOCKET),
	mConnectTimeout(seconds(-1.)),
	mReadTimeout(seconds(-1.)),
	mWriteTimeout(seconds(-1.))
{
	setTimeout(timeout);
	connect(a);
}

Socket::Socket(socket_t sock) :
	mConnectTimeout(seconds(-1.)),
	mReadTimeout(seconds(-1.)),
	mWriteTimeout(seconds(-1.))
{
	mSock = sock;
}

Socket::~Socket(void)
{
	NOEXCEPTION(close());
}

bool Socket::isConnected(void) const
{
	return (mSock != INVALID_SOCKET);
}

bool Socket::isReadable(void) const
{
	if(!isConnected()) return false;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(mSock, &readfds);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	::select(SOCK_TO_INT(mSock)+1, &readfds, NULL, NULL, &tv);
	return FD_ISSET(mSock, &readfds);
}

bool Socket::isWriteable(void) const
{
	if(!isConnected()) return false;

	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(mSock, &writefds);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	::select(SOCK_TO_INT(mSock)+1, NULL, &writefds, NULL, &tv);
	return FD_ISSET(mSock, &writefds);
}

Address Socket::getLocalAddress(void) const
{
	sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	if(getsockname(mSock, reinterpret_cast<sockaddr*>(&addr), &len))
		throw std::runtime_error("Unable to retrieve local address");

	return Address(reinterpret_cast<sockaddr*>(&addr), len);
}

Address Socket::getRemoteAddress(void) const
{
	sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	if(getpeername(mSock, reinterpret_cast<sockaddr*>(&addr), &len))
		throw std::runtime_error("Unable to retrieve remote address");

	return Address(reinterpret_cast<sockaddr*>(&addr), len);
}

void Socket::setConnectTimeout(duration timeout)
{
	mConnectTimeout = timeout;
}

void Socket::setReadTimeout(duration timeout)
{
	mReadTimeout = timeout;
}

void Socket::setWriteTimeout(duration timeout)
{
	mWriteTimeout = timeout;
}

void Socket::setTimeout(duration timeout)
{
	setConnectTimeout(timeout);
	setReadTimeout(timeout);
	setWriteTimeout(timeout);
}

void Socket::connect(const Address &addr)
{
	close();

	try {
		// Create socket
		mSock = ::socket(addr.addrFamily(),SOCK_STREAM,0);
		if(mSock == INVALID_SOCKET)
			throw std::runtime_error("Socket creation failed");

		if(mConnectTimeout >= duration::zero())
		{
			ctl_t b = 1;
			if(ioctl(mSock, FIONBIO, &b) < 0)
				throw std::runtime_error("Cannot set non-blocking mode");

			// Initiate connection
			::connect(mSock, addr.addr(), addr.addrLen());

			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(mSock, &writefds);

			struct timeval tv;
			durationToStruct(mConnectTimeout, tv);
			int ret = ::select(SOCK_TO_INT(mSock)+1, NULL, &writefds, NULL, &tv);

			if (ret < 0)
				throw std::runtime_error("Unable to wait on socket");

			if (ret ==  0 || ::send(mSock, NULL, 0, MSG_NOSIGNAL) != 0)
				throw std::runtime_error("Connection to " + addr.toString() + " failed");

			b = 0;
			if(ioctl(mSock, FIONBIO, &b) < 0)
				throw std::runtime_error("Cannot set blocking mode");
		}
		else {
			// Connect it
			if(::connect(mSock,addr.addr(), addr.addrLen()) != 0)
				throw std::runtime_error("Connection to " + addr.toString() + " failed");
		}
	}
	catch(...)
	{
		close();
		throw;
	}
}

void Socket::close(void)
{
	if(mSock != INVALID_SOCKET)
	{
		::closesocket(mSock);
		mSock = INVALID_SOCKET;
	}
}

size_t Socket::readSome(byte *buffer, size_t size) { return recv(buffer, size, 0); }

size_t Socket::writeSome(const byte *data, size_t size) { return send(data, size, 0); }

bool Socket::wait(duration timeout)
{
	if(mSock == INVALID_SOCKET)
		throw std::runtime_error("Socket is closed");

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(mSock, &readfds);

	struct timeval tv;
	durationToStruct(timeout, tv);
	int ret = ::select(SOCK_TO_INT(mSock)+1, &readfds, NULL, NULL, &tv);
	if (ret < 0) throw std::runtime_error("Unable to wait on socket");
	return (ret != 0);
}

size_t Socket::peek(byte *buffer, size_t size) { return ::recv(mSock, buffer, size, MSG_PEEK); }

size_t Socket::recv(byte *buffer, size_t size, int flags) {
	if(mSock == INVALID_SOCKET)
		throw std::runtime_error("Socket is closed");

	if(mReadTimeout >= duration::zero())
		if(!wait(mReadTimeout))
			throw timeout();

	int count = ::recv(mSock, buffer, size, flags);
	if(count < 0)
		throw std::runtime_error("Connection lost (error " + to_string(sockerrno) + ")");

	return count;
}

size_t Socket::send(const byte *data, size_t size, int flags) {
	struct timeval tv;
	durationToStruct(std::max(mWriteTimeout, duration::zero()), tv);

	if(mSock == INVALID_SOCKET)
		throw std::runtime_error("Socket is closed");

	if(mWriteTimeout >= duration::zero())
	{
		fd_set writefds;
		FD_ZERO(&writefds);
		FD_SET(mSock, &writefds);

		int ret = ::select(SOCK_TO_INT(mSock)+1, NULL, &writefds, NULL, &tv);
		if (ret < 0)
			throw std::runtime_error("Unable to wait on socket");
		if (ret == 0)
			throw timeout();
	}

	int count = ::send(mSock, data, size, flags | MSG_NOSIGNAL);
	if(count < 0)
		throw std::runtime_error("Connection lost (error " + to_string(sockerrno) + ")");

	return size_t(count);
}
}
