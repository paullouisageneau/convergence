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

#ifndef P3D_BUFFER_H
#define P3D_BUFFER_H

#include "p3d/include.hpp"
#include "p3d/bufferobject.hpp"

namespace pla
{

template<typename T>
class Buffer
{
public:
	Buffer(BufferObject *bufferObject);
	~Buffer(void);
	
	size_t count(void) const;
	
	void bind(void) const;	
	void *offset(size_t offset = 0) const;
	
	void fill(const T *ptr, size_t nbr);
	void replace(size_t offset, const T *ptr, size_t nbr);
	void append(const T *ptr, size_t nbr);
	T *data(size_t offset, size_t nbr) const;
	T *data(void) const;

protected:
	std::unique_ptr<BufferObject> mBufferObject;
	size_t mCount = 0;
};


template <class T>
Buffer<T>::Buffer(BufferObject *bufferObject) :
	mBufferObject(bufferObject),
	mCount(mBufferObject->size()/sizeof(T))
{

}

template <class T>
Buffer<T>::~Buffer(void)
{

}

template <class T>
size_t Buffer<T>::count(void) const
{
	return mCount;
}

template <class T>
void Buffer<T>::bind(void) const
{
	return mBufferObject->bind();
}

template <class T>
void *Buffer<T>::offset(size_t offset) const
{
	return mBufferObject->offset(offset*sizeof(T));
}

template <class T>
void Buffer<T>::fill(const T *ptr, size_t nbr)
{
	mBufferObject->fill(ptr, nbr*sizeof(T));
	mCount=nbr;
}

template <class T>
void Buffer<T>::replace(size_t offset, const T *ptr, size_t nbr)
{
	mBufferObject->replace(offset*sizeof(T), ptr, nbr*sizeof(T));
}

template <class T>
void Buffer<T>::append(const T *ptr, size_t nbr)
{
	if(!nbr) return;
	if(!mCount) {
		fill(ptr, nbr);
		return;
	}

	// Allocate buffer
	T *temp = new T[mCount + nbr];

	// Copy old
	const T *p = data(0, mCount);
	if(!p) throw std::runtime_error("Unable to append to unreadable buffer object");
	std::copy(p, p + mCount, temp);

	// Copy new
	if(ptr) std::copy(ptr, ptr + nbr, temp + mCount);
	else std::fill(temp + mCount, temp + mCount + nbr, T(0));

	// Refill
	fill(temp, mCount + nbr);
}

template <class T>
T *Buffer<T>::data(size_t offset, size_t nbr) const
{
	return reinterpret_cast<T*>(mBufferObject->data(offset*sizeof(T), nbr*sizeof(T)));
}

template <class T>
T *Buffer<T>::data(void) const
{
	return data(0, mCount);
}

}

#endif

