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

#include "pla/stream.hpp"

namespace pla
{

size_t Stream::read(char *buffer, size_t size)
{
	if(!isMessage())
	{
		size_t left = size;
		while(left)
		{
			size_t len = readSome(buffer + size - left, left);
			if(!len) break;
			left-= len;
		}
		return size - left;
	}
	else {
		return readSome(buffer, size);
	}
}

size_t Stream::read(binary &buf, size_t size)
{
	buf.resize(size);
	size = read(buf.data(), buf.size());
	buf.resize(size);
	return size;
}

size_t Stream::read(string &str, size_t size)
{
	str.resize(size);
	size = read(size ? &str[0] : NULL, str.size());
	str.resize(size);
	return size;
}

void Stream::write(const char *data, size_t size)
{
	if(!isMessage())
	{
		do {
			size_t len = writeSome(data, size);
			data+= len;
			size-= len;
		}
		while(size);
	}
	else {
		writeSome(data, size);
	}
}

void Stream::write(const binary &buf)
{
	write(buf.data(), buf.size());
}

void Stream::write(const string &str)
{
	write(str.data(), str.size());
}

size_t Stream::readAll(binary &buf)
{
	char tmp[BufferSize];
	size_t len;
	while((len = read(tmp, BufferSize)) > 0)
		buf.insert(buf.end(), tmp, tmp + len);
	return buf.size();
}

size_t Stream::readAll(string &str)
{
	char tmp[BufferSize];
	size_t len;
	while((len = read(tmp, BufferSize)) > 0)
		str.append(tmp, tmp + len);
	return str.size();
}

bool Stream::readLine(string &str)
{
	return readUntil(str, "\n", "\r\0");
}

void Stream::writeLine(const string &str)
{
	string line(str + "\r\n");
	write(line.data(), line.size());
}

size_t Stream::ignore(size_t size)
{
	char buffer[BufferSize];
	size_t left = size;
	size_t len;
	while(left && (len = read(buffer, std::min(left, BufferSize))))
		left-= len;

	return size - left;
}

void Stream::discard(void)
{
	char buffer[BufferSize];
	while(read(buffer, BufferSize)) {}
}

bool Stream::readUntil(string &output, const string &delimiters, const string &ignored)
{
	output.clear();
	if(!isMessage())
	{
		const size_t maxCount = 102400;	// 100 Ko for security reasons
		size_t left = maxCount;
		char chr;
		if(!read(&chr, 1)) return false;
		while(delimiters.find(chr) == string::npos)
		{
			if(ignored.find(chr) == string::npos) output+= chr;
			--left;
			if(!left || !read(&chr, 1)) break;
		}
		return true;
	} 
	else {
		char buffer[BufferSize];
		size_t len = read(buffer, BufferSize);
		if(!len) return false;

		for(size_t i=0; i<len; ++len)
		{
			char chr = buffer[i];
			if(delimiters.find(chr) != string::npos) break;
			if(ignored.find(chr) == string::npos) output+= chr;
		}
		return true;
	}
}

}