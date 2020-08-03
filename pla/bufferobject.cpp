/***************************************************************************
 *   Copyright (C) 2006-2016 by Paul-Louis Ageneau                         *
 *   paul-louis (at) ageneau (dot) org                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "pla/bufferobject.hpp"

namespace pla {

BufferObject::BufferObject(GLenum type, GLenum usage, bool readable)
    : mType(type), mUsage(usage), mReadable(readable) {
	glGenBuffers(1, &mBuffer);
}

BufferObject::~BufferObject(void) {
	glDeleteBuffers(1, &mBuffer);
	delete[] mCache;
}

size_t BufferObject::size(void) const { return mSize; }

void BufferObject::bind(void) { glBindBuffer(mType, mBuffer); }

void *BufferObject::offset(size_t offset) { return reinterpret_cast<void *>(offset); }

void BufferObject::fill(const void *ptr, size_t size) {
	if (size == mSize) {
		replace(0, ptr, size);
		return;
	}

	mSize = size;
	glBindBuffer(mType, mBuffer);
	glBufferData(mType, size, ptr, mUsage);

	if (mReadable) {
		delete[] mCache;
		mCache = new char[size];
		std::memcpy(mCache, ptr, size);
	}
}

void BufferObject::replace(size_t offset, const void *ptr, size_t size) {
	if (size == 0)
		return;

	glBindBuffer(mType, mBuffer);
	glBufferSubData(mType, offset, size, ptr);

	if (mCache)
		std::memcpy(mCache + offset, ptr, size);
}

void *BufferObject::data(size_t offset, size_t size) { return mCache; }

} // namespace pla
