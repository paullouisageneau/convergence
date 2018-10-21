/*************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                       *
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

#include "pla/binary.hpp"
#include "pla/binaryformatter.hpp"	// for checksum()

namespace pla
{

binary &operator^= (binary &a, const binary &b)
{
	a.resize(std::max(a.size(), b.size()), 0);
	memxor(a.data(), b.data(), b.size());
	return a;
}

binary operator^ (binary a, const binary &b)
{
	return a^= b;
}

binary &operator+= (binary &a, const binary &b)
{
	a.insert(a.end(), b.begin(), b.end());
	return a;
}

binary operator+ (binary a, const binary &b)
{
	return a+= b;
}

string to_string(const binary &data)
{
	return string(data.begin(), data.end());
}

binary from_string(const string &str)
{
	return binary(str.begin(), str.end());
}

string to_hex(const binary &data)
{
	std::ostringstream oss;
	for(int i=0; i<data.size(); ++i)
	{
		oss << std::hex << std::uppercase;
		oss << std::setfill('0') << std::setw(2);
		oss << unsigned(uint8_t(data[i]));
	}
	return oss.str();
}

binary from_hex(const string &str)
{
	binary out;
	if(str.empty()) return out;
	
	int count = (str.size()+1)/2;
	out.reserve(count);
	for(int i=0; i<count; ++i)
	{
		std::string byte;
		byte+= str[i*2];
		if(i*2+1 != str.size()) byte+= str[i*2+1];
		else byte+= '0';
		
		unsigned value = 0;
		std::istringstream iss(byte);
		if(!(iss >> std::hex >> value))
			throw std::invalid_argument("invalid hexadecimal representation");
		
		out.push_back(uint8_t(value % 256));
	}
	
	return out;
}

string to_base64(const binary &data, bool safeMode)
{
	static const char standardTab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static const char safeTab[]     = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	const char *tab = (safeMode ? safeTab : standardTab);
	
	string out;
	out.reserve(3*((data.size()+3)/4));
	int i = 0;
	while (data.size()-i >= 3)
	{
		out+= tab[uint8_t(data[i]) >> 2];
		out+= tab[((uint8_t(data[i]) & 3) << 4) | (uint8_t(data[i+1]) >> 4)];
		out+= tab[((uint8_t(data[i+1]) & 0x0F) << 2) | (uint8_t(data[i+2]) >> 6)];
		out+= tab[uint8_t(data[i+2]) & 0x3F];
		i+= 3;
	}
	
	int left = data.size()-i;
	if(left)
	{
		out+= tab[uint8_t(data[i]) >> 2];
		if(left == 1)
		{
			out+= tab[(uint8_t(data[i]) & 3) << 4];
			if(!safeMode) out+= '=';
		}
		else {	// left == 2
			out+= tab[((uint8_t(data[i]) & 3) << 4) | (uint8_t(data[i+1]) >> 4)];
			out+= tab[(uint8_t(data[i+1]) & 0x0F) << 2];
		}
		if(!safeMode) out+= '=';
	}
	
	return out;
}


binary from_base64(const string &str)
{
	binary out;
	out.reserve(4*((str.size()+2)/3));
	int i = 0;
	while(i < str.size())
	{
		unsigned char tab[4];
		std::memset(tab, 0, 4);
		int j = 0;
		while(i < str.size() && j < 4)
		{
			char c = str[i];
			if(c == '=') break;
			
			if ('A' <= c && c <= 'Z') tab[j] = c - 'A';
			else if ('a' <= c && c <= 'z') tab[j] = c + 26 - 'a';
			else if ('0' <= c && c <= '9') tab[j] = c + 52 - '0';
			else if (c == '+' || c == '-') tab[j] = 62;
			else if (c == '/' || c == '_') tab[j] = 63;
			else throw std::invalid_argument("Invalid character in base64");
			
			++i; ++j;
		}
		
		if(j)
		{
			out.push_back((tab[0] << 2) | (tab[1] >> 4));
			if (j > 2)
			{
				out.push_back((tab[1] << 4) | (tab[2] >> 2));
				if (j > 3) out.push_back((tab[2] << 6) | (tab[3]));
			}
		}
		
		if(i < str.size() && str[i] == '=') break;
	}

	return out;
}

binary pack_strings(const std::vector<string> &strs)
{
	size_t size = 0;
	for(const string &str : strs) size+= str.size() + 1;

	binary result;
	result.reserve(size);
	for(const string &str : strs)
	{
		result.insert(result.end(), str.begin(), str.end());
		result.emplace_back('\0');
	}
	return result;
}

std::vector<string> unpack_strings(const binary &data)
{
	std::vector<string> result;
	size_t i = 0;
	while(i < data.size())
	{
		result.emplace_back();
		string &str = result.back();
		while(i < data.size() && data[i] != '\0')
		{
			str+= data[i];
			++i;
		}
		++i;
	}
	return result;
}

uint16_t checksum16(const binary &b)
{ 
	uint16_t i = 0; 
	return checksum(b, i);
}
uint32_t checksum32(const binary &b)
{
	uint32_t i = 0; 
	return checksum(b, i);
}
uint64_t checksum64(const binary &b)
{
	uint64_t i = 0; 
	return checksum(b, i);
}

std::size_t binary_hash::operator()(const binary &b) const noexcept
{
	std::size_t i = 0;
	checksum(b, i);
	return i;
}

}
