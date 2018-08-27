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

#ifndef PLA_BUFFEROBJECT_H
#define PLA_BUFFEROBJECT_H

#include "pla/include.hpp"
#include "pla/opengl.hpp"

namespace pla
{

class BufferObject
{
public:
	BufferObject(	GLenum type = GL_ELEMENT_ARRAY_BUFFER,
					GLenum usage = GL_DYNAMIC_DRAW,
					bool readable = true);
	virtual ~BufferObject(void);
	
	size_t size(void) const;
	
	void bind(void);
	void *offset(size_t offset);
	
	void fill(const void *ptr, size_t size);
	void replace(size_t offset, const void *ptr, size_t size);
	void *data(size_t offset, size_t size);

private:
	GLenum mType;
	GLenum mUsage;
	GLuint mBuffer;
	bool mReadable;
	
	size_t mSize = 0;
	char *mCache = NULL;
};

class IndexBufferObject : public BufferObject
{
public:
	IndexBufferObject(bool readable = true) : 
		BufferObject(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, readable) {}
};

class AttribBufferObject : public BufferObject
{
public:
	AttribBufferObject(bool readable = true) : BufferObject(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, readable) {}
};

}

#endif

