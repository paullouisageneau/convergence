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

#include "pla/binaryformatter.hpp"

namespace pla {

BinaryFormatter::BinaryFormatter(void) {}

BinaryFormatter::BinaryFormatter(const binary &b) : mData(b) {}

binary BinaryFormatter::remaining(void) const {
	return binary(mData.begin() + mReadPosition, mData.end());
}

const binary &BinaryFormatter::data(void) const { return mData; }

binary &BinaryFormatter::data(void) { return mData; }

binary &BinaryFormatter::data(const binary &data) {
	mData = data;
	mReadPosition = 0;
	return mData;
}

void BinaryFormatter::append(const binary &b) { mData.insert(mData.end(), b.begin(), b.end()); }

void BinaryFormatter::clear(void) {
	mData.clear();
	mReadPosition = 0;
}

size_t BinaryFormatter::read(byte *data, size_t size) {
	auto begin = mData.begin() + mReadPosition;
	size = std::min(size, size_t(mData.end() - begin));
	std::copy(begin, begin + size, data);
	mData.erase(begin, begin + size);
	return size;
}

void BinaryFormatter::write(const byte *data, size_t size) {
	mData.insert(mData.end(), data, data + size);
}

BinaryFormatter &BinaryFormatter::operator>>(binary &b) {
	mReadFailed = (read(b.data(), b.size()) != b.size());
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(std::string &s) {
	mReadFailed = (read(reinterpret_cast<byte *>(s[0]), s.size()) != s.size());
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(int8_t &i) {
	uint8_t u;
	if (*this >> u)
		i = int8_t(u);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(int16_t &i) {
	uint16_t u;
	if (*this >> u)
		i = int16_t(u);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(int32_t &i) {
	uint32_t u;
	if (*this >> u)
		i = int32_t(u);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(int64_t &i) {
	uint64_t u;
	if (*this >> u)
		i = int64_t(u);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(uint8_t &i) {
	mReadFailed = !read(reinterpret_cast<byte *>(&i), sizeof(i));
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(uint16_t &i) {
	mReadFailed = !read(reinterpret_cast<byte *>(&i), sizeof(i));
	if (!mReadFailed)
		i = toBigEndian(i);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(uint32_t &i) {
	mReadFailed = !read(reinterpret_cast<byte *>(&i), sizeof(i));
	if (!mReadFailed)
		i = toBigEndian(i);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(uint64_t &i) {
	mReadFailed = !read(reinterpret_cast<byte *>(&i), sizeof(i));
	if (!mReadFailed)
		i = toBigEndian(i);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(float32_t &f) {
	mReadFailed = !read(reinterpret_cast<byte *>(&f), sizeof(f));
	return *this;
}

BinaryFormatter &BinaryFormatter::operator>>(float64_t &f) {
	mReadFailed = !read(reinterpret_cast<byte *>(&f), sizeof(f));
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(const binary &b) {
	write(b.data(), b.size());
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(const string &s) {
	write(reinterpret_cast<const byte *>(s.data()), s.size());
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(const char *s) {
	size_t len = strlen(s);
	*this << uint32_t(len);
	write(reinterpret_cast<const byte *>(s), len);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(int8_t i) {
	*this << uint8_t(i);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(int16_t i) {
	*this << uint16_t(i);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(int32_t i) {
	*this << uint32_t(i);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(int64_t i) {
	*this << uint64_t(i);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(uint8_t i) {
	write(reinterpret_cast<byte *>(&i), 1);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(uint16_t i) {
	i = toBigEndian(i);
	write(reinterpret_cast<byte *>(&i), 2);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(uint32_t i) {
	i = toBigEndian(i);
	write(reinterpret_cast<byte *>(&i), 4);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(uint64_t i) {
	i = toBigEndian(i);
	write(reinterpret_cast<byte *>(&i), 8);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(float32_t f) {
	write(reinterpret_cast<byte *>(&f), 4);
	return *this;
}

BinaryFormatter &BinaryFormatter::operator<<(float64_t f) {
	write(reinterpret_cast<byte *>(&f), 8);
	return *this;
}

uint16_t BinaryFormatter::toBigEndian(uint16_t n) {
	uint8_t *p = reinterpret_cast<uint8_t *>(&n);
	return (uint16_t(p[0]) << 8) | (uint16_t(p[1]));
}

uint32_t BinaryFormatter::toBigEndian(uint32_t n) {
	uint8_t *p = reinterpret_cast<uint8_t *>(&n);
	return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) |
	       (uint32_t(p[3]));
}

uint64_t BinaryFormatter::toBigEndian(uint64_t n) {
	uint8_t *p = reinterpret_cast<uint8_t *>(&n);
	return (uint64_t(p[0]) << 56) | (uint64_t(p[1]) << 48) | (uint64_t(p[2]) << 40) |
	       (uint64_t(p[3]) << 32) | (uint64_t(p[4]) << 24) | (uint64_t(p[5]) << 16) |
	       (uint64_t(p[6]) << 8) | (uint64_t(p[7]));
}

} // namespace pla
