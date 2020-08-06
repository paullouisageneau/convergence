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

#include "pla/context.hpp"
#include "pla/include.hpp"
#include "pla/mesh.hpp"
#include "pla/program.hpp"

namespace pla {

class Object {
public:
	using index_t = Mesh::index_t;

	Object(sptr<Mesh> mesh = nullptr, sptr<Program> program = nullptr);
	Object(const index_t *indices, size_t nindices, const float *vertices, size_t nvertices,
	       sptr<Program> program = nullptr);
	virtual ~Object();

	void setMesh(sptr<Mesh> mesh);
	void setProgram(sptr<Program> program, size_t firstIndex = 0);
	void unsetProgram(size_t firstIndex = 0);

	virtual int draw(const Context &context) const;

protected:
	sptr<Mesh> mMesh;
	std::map<size_t, sptr<Program>> mPrograms;
};

class Sphere : public Object {
public:
	Sphere(int resolution, sptr<Program> program = nullptr);

private:
	static sptr<Mesh> Build(int resolution);
};

class Quad : public Object {
public:
	Quad(sptr<Program> program = nullptr);

private:
	static sptr<Mesh> Build();
};

} // namespace pla

#endif
