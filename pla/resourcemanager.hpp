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

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "pla/include.hpp"
#include "pla/resource.hpp"
#include "pla/string.hpp"

namespace pla {

class ResourceManager {
public:
	// Recupere une ressource
	template <class T> sptr<T> get(const string &name) const;

	// Ajoute une ressource
	void add(const string &name, sptr<Resource> resource);

	// Retire une ressource
	void remove(const string &name);

	// Retire les ressources
	void flush(void);

private:
	// Table contenant les ressources associees a leur nom de fichier
	std::map<string, sptr<Resource>> mResources;
};

// Renvoie un pointeur sur une ressource déjà chargée (NULL si non trouvée)
template <class T> inline sptr<T> ResourceManager::get(const string &name) const {
	// Recherche de la ressource
	auto it = mResources.find(name);

	// Si on l'a trouvée on la renvoie, sinon on renvoie NULL
	if (it != mResources.end())
		return std::dynamic_pointer_cast<T>(it->second);
	else
		return NULL;
}

} // namespace pla

#endif // RESOURCEMANAGER_H
