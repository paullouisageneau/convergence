/***************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_MESSAGE_H
#define CONVERGENCE_MESSAGE_H

#include "src/include.hpp"

namespace convergence {

using pla::binary;

class Message {
public:
	enum Type : uint32_t {
		Dummy = 0x00,

		// Signaling
		Join = 0x01,
		List = 0x02,

		// Peering
		Description = 0x11,
		Candidate = 0x12,

		// Player
		EntityReserved = 0x20,
		EntityTransform = 0x21,
		EntitySpeed = 0x22,
		EntityControl = 0x23,

		// Store
		Store = 0x30,
		Request = 0x31,

		// Terrain
		TerrainRoot = 0x40,
		TerrainUpdate = 0x41
	};

	Message(Type _type = Dummy);
	Message(const binary &data);

	operator binary(void) const;

	Type type;
	identifier source;
	identifier destination;
	binary payload;
};

} // namespace convergence

#endif
