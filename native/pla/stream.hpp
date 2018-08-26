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

#ifndef PLA_STREAM_H
#define PLA_STREAM_H

#include "pla/include.hpp"
#include "pla/string.hpp"
#include "pla/binary.hpp"

namespace pla
{

class Stream
{
public:
	Stream(void) {}
	virtual ~Stream(void) {}
	
	size_t read(char *buffer, size_t size);
	size_t read(binary &buf, size_t size);
	size_t read(string &str, size_t size);
	
	void write(const char *data, size_t size);
	void write(const binary &buf);
	void write(const string &str);
	
	size_t readAll(binary &buf);
	size_t readAll(string &str);
	
	bool readLine(string &str);
	void writeLine(const string &str);

	virtual size_t readSome(char *buffer, size_t size) = 0;
	virtual size_t writeSome(const char *data, size_t size) = 0;
	
	virtual bool wait(duration timeout) { return true; }
	virtual bool isMessage(void) const { return false; }
	virtual void setTimeout(duration timeout) {}
	virtual size_t ignore(size_t size = 1);
	virtual void discard(void);
	virtual void close(void) {}
	
private:
	bool readUntil(string &output, const string &delimiters, const string &ignored);
};

}

#endif // STREAM_H
