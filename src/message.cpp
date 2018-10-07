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

#include "src/message.hpp"

#include "pla/binaryformatter.hpp"

namespace convergence
{

using pla::BinaryFormatter;

Message::Message(Type _type) :
	type(_type)
{
	
}

Message::Message(const binary &data)
{
	uint32_t size = 0;
	uint32_t tmpType = 0;
	
	BinaryFormatter formatter(data);
	formatter >> tmpType >> size;
	payload.resize(size);
	type = Type(tmpType);
	
	formatter >> source;
	formatter >> destination;
	formatter >> payload;
}

Message::operator binary(void) const
{
	uint32_t size(payload.size());
	
	BinaryFormatter formatter;
	formatter << uint32_t(type) << size;
	
	formatter << source;
	formatter << destination;
	formatter << payload;
	
	return formatter.data();
}

}

