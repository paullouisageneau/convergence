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
#include "pla/binaryformatter.hpp" // for checksum()

#include <algorithm>

namespace pla {

using std::to_integer;

binary &operator^=(binary &a, const binary &b) {
	a.resize(std::max(a.size(), b.size()), byte(0));
	memxor(a.data(), b.data(), b.size());
	return a;
}

binary operator^(binary a, const binary &b) { return a ^= b; }

binary &operator+=(binary &a, const binary &b) {
	a.insert(a.end(), b.begin(), b.end());
	return a;
}

binary operator+(binary a, const binary &b) { return a += b; }

string to_string(const binary &data) {
	string r;
	r.reserve(data.size());
	std::transform(data.begin(), data.end(), std::back_inserter(r),
	               [](byte b) { return to_integer<char>(b); });
	return r;
}

binary to_binary(const string &str) {
	binary r;
	r.reserve(str.size());
	std::transform(str.begin(), str.end(), std::back_inserter(r), [](char c) { return byte(c); });
	return r;
}

string to_hex(const binary &data) {
	std::ostringstream oss;
	for (int i = 0; i < data.size(); ++i) {
		oss << std::hex << std::uppercase;
		oss << std::setfill('0') << std::setw(2);
		oss << unsigned(uint8_t(data[i]));
	}
	return oss.str();
}

binary from_hex(const string &str) {
	binary out;
	if (str.empty())
		return out;

	int count = (str.size() + 1) / 2;
	out.reserve(count);
	for (int i = 0; i < count; ++i) {
		std::string s;
		s += str[i * 2];
		if (i * 2 + 1 != str.size())
			s += str[i * 2 + 1];
		else
			s += '0';

		unsigned value = 0;
		std::istringstream iss(s);
		if (!(iss >> std::hex >> value))
			throw std::invalid_argument("invalid hexadecimal representation");

		out.push_back(byte(value & 0xFF));
	}

	return out;
}

string to_base64(const binary &data, bool safeMode) {
	static const char standardTab[] =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static const char safeTab[] =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	const char *tab = (safeMode ? safeTab : standardTab);

	string out;
	out.reserve(3 * ((data.size() + 3) / 4));
	int i = 0;
	while (data.size() - i >= 3) {
		auto d0 = to_integer<uint8_t>(data[i]);
		auto d1 = to_integer<uint8_t>(data[i + 1]);
		auto d2 = to_integer<uint8_t>(data[i + 2]);
		out += tab[d0 >> 2];
		out += tab[((d0 & 3) << 4) | (d1 >> 4)];
		out += tab[((d1 & 0x0F) << 2) | (d2 >> 6)];
		out += tab[d2 & 0x3F];
		i += 3;
	}

	int left = data.size() - i;
	if (left) {
		auto d0 = to_integer<uint8_t>(data[i]);
		out += tab[d0 >> 2];
		if (left == 1) {
			out += tab[(d0 & 3) << 4];
			if (!safeMode)
				out += '=';
		} else { // left == 2
			auto d1 = to_integer<uint8_t>(data[i + 1]);
			out += tab[((d0 & 3) << 4) | (d1 >> 4)];
			out += tab[(d1 & 0x0F) << 2];
		}
		if (!safeMode)
			out += '=';
	}

	return out;
}

binary from_base64(const string &str) {
	binary out;
	out.reserve(4 * ((str.size() + 2) / 3));
	int i = 0;
	while (i < str.size()) {
		byte tab[4] = {};
		int j = 0;
		while (i < str.size() && j < 4) {
			uint8_t c = str[i];
			if (c == '=')
				break;

			if ('A' <= c && c <= 'Z')
				tab[j] = byte(c - 'A');
			else if ('a' <= c && c <= 'z')
				tab[j] = byte(c + 26 - 'a');
			else if ('0' <= c && c <= '9')
				tab[j] = byte(c + 52 - '0');
			else if (c == '+' || c == '-')
				tab[j] = byte(62);
			else if (c == '/' || c == '_')
				tab[j] = byte(63);
			else
				throw std::invalid_argument("Invalid character in base64");

			++i;
			++j;
		}

		if (j) {
			out.push_back((tab[0] << 2) | (tab[1] >> 4));
			if (j > 2) {
				out.push_back((tab[1] << 4) | (tab[2] >> 2));
				if (j > 3)
					out.push_back((tab[2] << 6) | (tab[3]));
			}
		}

		if (i < str.size() && str[i] == '=')
			break;
	}

	return out;
}

binary pack_strings(const std::vector<string> &strs) {
	size_t size = 0;
	for (const string &str : strs)
		size += str.size() + 1;

	binary result;
	result.reserve(size);
	for (const string &str : strs) {
		std::transform(str.begin(), str.end(), std::back_inserter(result),
		               [](char c) { return byte(c); });
		result.emplace_back(byte(0));
	}
	return result;
}

std::vector<string> unpack_strings(const binary &data) {
	std::vector<string> result;
	size_t i = 0;
	while (i < data.size()) {
		result.emplace_back();
		string &str = result.back();
		while (i < data.size() && data[i] != byte(0)) {
			str += to_integer<char>(data[i]);
			++i;
		}
		++i;
	}
	return result;
}

uint16_t checksum16(const binary &b) {
	uint16_t i = 0;
	return checksum(b, i);
}
uint32_t checksum32(const binary &b) {
	uint32_t i = 0;
	return checksum(b, i);
}
uint64_t checksum64(const binary &b) {
	uint64_t i = 0;
	return checksum(b, i);
}

std::size_t binary_hash::operator()(const binary &b) const noexcept {
	std::size_t i = 0;
	checksum(b, i);
	return i;
}

} // namespace pla
