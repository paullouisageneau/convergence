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

#ifndef PLA_BINARY_H
#define PLA_BINARY_H

#include "pla/include.hpp"
#include "pla/string.hpp"

#include <cstddef>
#include <vector>

namespace pla
{

using std::byte;
using binary = std::vector<byte>;
using std::to_integer;

binary &operator^= (binary &a, const binary &b);
binary operator^ (binary a, const binary &b);

binary &operator+= (binary &a, const binary &b);
binary operator+ (binary a, const binary &b);

string to_string(const binary &data);
binary to_binary(const string &str);

string to_hex(const binary &data);
binary from_hex(const string &str);

// safe mode is RFC 4648 'base64url' encoding
string to_base64(const binary &data, bool safeMode = false);
binary from_base64(const string &str);

// Pack zero-terminated strings in a binary
binary pack_strings(const std::vector<string> &strs);
std::vector<string> unpack_strings(const binary &data);

uint16_t checksum16(const binary &b);
uint32_t checksum32(const binary &b);
uint64_t checksum64(const binary &b);

struct binary_hash
{
	std::size_t operator()(const binary &b) const noexcept;
};

}

#endif
