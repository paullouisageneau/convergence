/*************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                       *
 *   paul-louis (at) ageneau (dot) org                                   *
 *                                                                       *
 *   This file is part of Plateform.                                     *
 *                                                                       *
 *   Plateform is free software: you can redibufferibute it and/or modify   *
 *   it under the terms of the GNU Affero General Public License as      *
 *   published by the Free Software Foundation, either version 3 of      *
 *   the License, or (at your option) any later version.                 *
 *                                                                       *
 *   Plateform is dibuffeributed in the hope that it will be useful, but    *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        *
 *   GNU Affero General Public License for more details.                 *
 *                                                                       *
 *   You should have received a copy of the GNU Affero General Public    *
 *   License along with Plateform.                                       *
 *   If not, see <http://www.gnu.org/licenses/>.                         *
 *************************************************************************/

#ifndef PLA_BINARYFORMATTER_H
#define PLA_BINARYFORMATTER_H

#include "pla/include.hpp"
#include "pla/binary.hpp"

namespace pla
{

class BinaryFormatter
{
public:
	BinaryFormatter(void);
	BinaryFormatter(const binary &b);
	
	binary remaining(void) const;
	binary data(void) const;
	binary data(const binary &data);
	
	void append(const binary &b);
	void clear(void);
	
	size_t read(char *data, size_t size);
	void write(const char *data, size_t size);
	
	BinaryFormatter &operator>> (binary &b);
	BinaryFormatter &operator>> (string &s);
	BinaryFormatter &operator>> (uint8_t &i);
	BinaryFormatter &operator>> (uint16_t &i);
	BinaryFormatter &operator>> (uint32_t &i);
	BinaryFormatter &operator>> (uint64_t &i);
	BinaryFormatter &operator>> (int8_t &i);
	BinaryFormatter &operator>> (int16_t &i);
	BinaryFormatter &operator>> (int32_t &i);
	BinaryFormatter &operator>> (int64_t &i);
	BinaryFormatter &operator>> (float32_t &f);
	BinaryFormatter &operator>> (float64_t &f);
	
	BinaryFormatter &operator<< (const binary &b);
	BinaryFormatter &operator<< (const string &s);
	BinaryFormatter &operator<< (const char *s);
	BinaryFormatter &operator<< (uint8_t i);
	BinaryFormatter &operator<< (uint16_t i);
	BinaryFormatter &operator<< (uint32_t i);
	BinaryFormatter &operator<< (uint64_t i);
	BinaryFormatter &operator<< (int8_t i);
	BinaryFormatter &operator<< (int16_t i);
	BinaryFormatter &operator<< (int32_t i);
	BinaryFormatter &operator<< (int64_t i);
	BinaryFormatter &operator<< (float32_t f);
	BinaryFormatter &operator<< (float64_t f);
	
	bool operator!(void) const { return mReadFailed; }
	operator bool(void) const { return !mReadFailed; }
	
protected:
	static uint16_t fixEndianess(uint16_t n);
	static uint32_t fixEndianess(uint32_t n);
	static uint64_t fixEndianess(uint64_t n);
	
	binary mData;
	size_t mReadPosition = 0;
	bool mReadFailed = false;
};

template<typename T>
T checksum(const binary &b, T &result)
{
	BinaryFormatter formatter(b);
	
	size_t padding = (sizeof(T) - (b.size() % sizeof(T))) % sizeof(T);
	for(size_t i = 0; i < padding; ++i) 
		formatter << uint8_t(0);

	result = 0;
	T value;
	while(formatter >> value)
		result = result ^ value;

	return result;
}

}

#endif
