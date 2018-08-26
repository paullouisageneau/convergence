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

#ifndef PLA_DATAGRAMSOCKET_H
#define PLA_DATAGRAMSOCKET_H

#include "pla/include.hpp"
#include "pla/stream.hpp"
#include "pla/address.hpp"
#include "pla/binary.hpp"

namespace pla
{

class DatagramStream;

class DatagramSocket
{
public:
	static const size_t MaxDatagramSize;

	DatagramSocket(int port = 0, bool broadcast = false);
	DatagramSocket(const Address &local, bool broadcast = false);
	~DatagramSocket(void);

	Address getBindAddress(void) const;
	void getLocalAddresses(std::set<Address> &set) const;
	void getHardwareAddresses(std::set<binary> &set) const;

	void bind(int port, bool broascast = false, int family = AF_UNSPEC);
	void bind(const Address &local, bool broadcast = false);
	void close(void);

	int read(char *buffer, size_t size, Address &sender, duration timeout = seconds(-1.));
	int peek(char *buffer, size_t size, Address &sender, duration timeout = seconds(-1.));
	int write(const char *buffer, size_t size, const Address &receiver);

	bool read(binary &buffer, Address &sender, duration timeout = seconds(-1.));
	bool peek(binary &buffer, Address &sender, duration timeout = seconds(-1.));
	void write(const binary &data, const Address &receiver);

	bool wait(duration timeout);

	void accept(DatagramStream &stream);
	void registerStream(DatagramStream *stream);
	void unregisterStream(DatagramStream *stream);

private:
	int recv(char *buffer, size_t size, Address &sender, duration timeout, int flags);
	int send(const char *buffer, size_t size, const Address &receiver, int flags);

	socket_t mSock;
	int mPort;

	// Mapped streams
	std::multimap<Address, DatagramStream*> mStreams;
	std::mutex mStreamsMutex;
};

class DatagramStream : public Stream
{
public:
	static duration DefaultTimeout;
	static int MaxQueueSize;

	DatagramStream(void);
	DatagramStream(DatagramSocket *sock, const Address &addr);
	~DatagramStream(void);

	Address getLocalAddress(void) const;
	Address getRemoteAddress(void) const;

	void setTimeout(duration timeout);

	// Stream
	size_t readSome(char *buffer, size_t size);
	size_t writeSome(const char *data, size_t size);
	bool wait(duration timeout);
	void close(void);
	bool isMessage(void) const;

private:
	DatagramSocket *mSock;
	Address mAddr;
	std::queue<binary> mIncoming;
	duration mTimeout;

	std::mutex mMutex;
	std::condition_variable mCondition;

	friend class DatagramSocket;
};

}

#endif
