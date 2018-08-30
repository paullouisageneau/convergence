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

#ifndef CONVERGENCE_NETWORKING_H
#define CONVERGENCE_NETWORKING_H

#include "src/include.hpp"
#include "src/peer.hpp"
#include "src/messagebus.hpp"

namespace convergence
{

using pla::string;
using std::shared_ptr;

class Networking
{
public:
	Networking(const string &url);
	~Networking(void);

private:
	identifier mLocalId;
	shared_ptr<MessageBus> mSignaling;
	std::map<identifier, shared_ptr<Peer>> mPeers;
};

}

#endif

