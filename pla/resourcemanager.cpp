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

#include "pla/resourcemanager.hpp"

namespace pla {

// Ajoute une ressource
void ResourceManager::add(const string &name, sptr<Resource> resource) {
	if (resource)
		mResources[name] = resource;
}

// Retire une ressource
void ResourceManager::remove(const string &name) {
	// Recherche de la ressource dans la table
	auto it = mResources.find(name);

	// Retrait de la ressource de la liste
	if (it != mResources.end())
		mResources.erase(it);
}

// Retire les ressources
void ResourceManager::flush(void) { mResources.clear(); }

} // namespace pla
