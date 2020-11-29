/***************************************************************************
 *   Copyright (C) 2017-2020 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_IDENTIFIER_H
#define CONVERGENCE_IDENTIFIER_H

#include "src/include.hpp"
#include "src/types.hpp"

#include "pla/binary.hpp"
#include "pla/binaryformatter.hpp"

namespace convergence {

class identifier : public binary {
public:
	static identifier generate(const int3 &p, uint32_t id) {
		pla::BinaryFormatter formatter;
		formatter << int32_t(p.x) << int32_t(p.y) << int32_t(p.z) << id;
		return formatter.data();
	}

	identifier(binary b = binary(16, byte(0))) : binary(std::move(b)) {}

	inline bool isNull() const {
		return std::all_of(begin(), end(), [](byte b) { return b == byte(0); });
	}

	inline void clear() { assign(16, byte(0)); }
};

} // namespace convergence

#endif
