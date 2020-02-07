/***************************************************************************
 *   Copyright (C) 2015-2020 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_FACTORY_H
#define CONVERGENCE_FACTORY_H

#include "src/include.hpp"
#include "src/types.hpp"

#include "pla/mesh.hpp"
#include "pla/object.hpp"
#include "pla/program.hpp"

namespace convergence {

using pla::Mesh;
using pla::Object;
using pla::Program;

class Factory {
public:
	Factory(const string &filename, float scale, shared_ptr<Program> program);
	virtual ~Factory();

	shared_ptr<Object> build() const;

private:
	shared_ptr<Program> mProgram;
	shared_ptr<Mesh> mMesh;
};

} // namespace convergence

#endif