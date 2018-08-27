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

#include "pla/mediamanager.hpp"

namespace pla
{

MediaManager::MediaManager(sptr<ResourceManager> resourceManager) :
	mResourceManager(resourceManager)
{
	mPaths.insert("");	// current directory
}

MediaManager::~MediaManager(void)
{

}

// Ajoute un repertoire de recherche pour les medias
void MediaManager::addPath(string path)
{
	if(path.empty()) return;
	std::replace(path.begin(), path.end(), '\\', '/');
	
	if(*path.rbegin() == '/') 
		mPaths.insert(path);
	else 
		mPaths.insert(path + "/");
}

// Cherche un fichier dans les repertoires de recherche
string MediaManager::findMedia(string filename) const
{
    std::replace(filename.begin(), filename.end(), '\\', '/');
	
	// Parcours de la liste des chemins de recherche
    for (std::set<string>::const_iterator it = mPaths.begin(); it != mPaths.end(); ++it)
    {
		string fullname = *it + filename;
		std::ifstream test(fullname.c_str());	// teste l'ouverture

		if(test.is_open()) {
			test.close();
			return fullname;
		}
    }

    // Si le fichier est introuvable, on lance une exception
    throw std::runtime_error(string("File not found: ") + filename);
}

}
