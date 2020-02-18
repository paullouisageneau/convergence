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

#include "pla/object.hpp"

namespace pla {

Object::Object(sptr<Mesh> mesh, sptr<Program> program) : mMesh(mesh) {
	if (program)
		setProgram(program, 0);
}

Object::Object(const index_t *indices, size_t nindices, const float *vertices, size_t nvertices,
               sptr<Program> program)
    : mMesh(std::make_shared<Mesh>(indices, nindices, vertices, nvertices)) {
	if (program)
		setProgram(program, 0);
}

Object::~Object(void) {}

void Object::setMesh(sptr<Mesh> mesh) { mMesh = mesh; }

void Object::setProgram(sptr<Program> program, size_t firstIndex) {
	if (program)
		mPrograms.insert(std::make_pair(firstIndex, program));
	else
		mPrograms.erase(firstIndex);
}

void Object::unsetProgram(size_t firstIndex) { mPrograms.erase(firstIndex); }

int Object::draw(const Context &context) const {
	int count = 0;
	if (mMesh) {
		auto it = mPrograms.begin();
		while (it != mPrograms.end()) {
			size_t first = it->first;
			size_t last = mMesh->indicesCount();
			auto program = it->second;

			if (++it != mPrograms.end())
				last = it->first;

			if (program) {
				context.prepare(program);
				program->bind();
				count += mMesh->drawElements(first, last - first);
				program->unbind();
			}
		}
	}
	return count;
}

} // namespace pla
