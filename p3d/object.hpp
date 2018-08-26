/***************************************************************************
 *   Copyright (C) 2006-2010 by Paul-Louis Ageneau                         *
 *   paul-louis (at) ageneau (dot) org                               *
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

#ifndef OBJECT_H
#define OBJECT_H

#include "p3d/include.hpp"
#include "p3d/mesh.hpp"
#include "p3d/program.hpp"
#include "p3d/context.hpp"

namespace pla
{

class Object : public Mesh
{
public:
	Object(void);
	Object(	const index_t *indices,
		size_t nindices,
		const float *vertices,
		size_t nvertices,
		sptr<Program> program);
	virtual ~Object(void);
	
	void setProgram(sptr<Program> program, size_t firstIndex = 0);
	void unsetProgram(size_t firstIndex = 0);
	
	virtual int draw(const Context &context);
	
protected:
	std::map<size_t, sptr<Program> > mPrograms;
};

}

#endif
