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

#ifndef PLA_MEDIAMANAGER_H
#define PLA_MEDIAMANAGER_H

#include "pla/include.hpp"
#include "pla/loader.hpp"
#include "pla/resourcemanager.hpp"

namespace pla
{

template <class T> struct MediaHandler
{
    std::map<string, sptr<Loader<T> > > loaders;
};

class Shader;
class Program;

class MediaManager :
	public MediaHandler<Shader>,
	public MediaHandler<Program>
{

public:
	MediaManager(sptr<ResourceManager> resourceManager);
        ~MediaManager(void);

	// Ajoute un repertoire de recherche de medias
	void addPath(string path);

	 // Enregistre un nouveau chargeur
	template <class T> void registerLoader(Loader<T>* loader, const string &extensions);

	// Supprime un chargeur
	template <class T> void unregisterLoader(const string &extension);

 	// Charge a partir d'un fichier
	template <class T> sptr<T> load(const string &filename) const;

	 // Retourne la ressource si deja chargee, sinon charge
	template <class T> sptr<T> get(const string &filename) const;

	// Trouve un fichier
	string findMedia(string filename) const;

private :
	// Trouve un loader
	template <class T> Loader<T> &findLoader(const string &filename) const;

	sptr<ResourceManager> mResourceManager;
	std::set<string> mPaths; // Liste des chemins de recherche
};

// Enregistre un nouveau chargeur de media
template <class T>
void MediaManager::registerLoader(Loader<T>* loader, const string &extensions)
{
	if(!loader) return;

	// TODO
	// Récupération des extensions
	/*std::list<string> lst;
	extensions.explode(lst,',');
	while(!lst.empty())
	{
		string ext = lst.front();
		ext.trim();
		lst.pop_front();
		MediaHandler<T>::loaders.insert(make_pair(ext,loader));
	}*/
}

// Supprime un chargeur
template <class T>
void MediaManager::unregisterLoader(const string &extensions)
{
	MediaHandler<T>::loaders.erase(extensions);
}


// Charge un media
template <class T>
sptr<T> MediaManager::load(const string &filename) const
{
	// Recherche du fichier dans les répertoires enregistrés
	string fullpath = findMedia(filename);

	// On appelle le loader approprié
	sptr<T> media = findLoader<T>(filename).load(fullpath);

	// On enregistre la ressource
	mResourceManager->add(fullpath,media);

	return media;
}

 // Retourne la ressource si déjà chargée, sinon charge
template <class T>
sptr<T> MediaManager::get(const string &filename) const
{
	// Recherche directement la ressource
	sptr<T> media = mResourceManager->get<T>(filename);
	if(media) return media;

	// Recherche du fichier dans les répertoires enregistrés
	string fullpath = findMedia(filename);

	// Recherche la ressource
	media = mResourceManager->get<T>(fullpath);
	if(media) return media;

	// Sinon on appelle le loader approprié
	media = findLoader<T>(filename).load(fullpath);

	// On enregistre la ressource
	mResourceManager->add(fullpath,media);

	return media;
}

// Cherche le loader correspondant à un fichier donné
template <class T>
Loader<T> &MediaManager::findLoader(const string &filename) const
{
	size_t p = filename.find_last_of('.');
	if(p != string::npos)
	{
		string extension(filename,++p);

		// Recherche de l'extension dans la map de loaders
		auto it = MediaHandler<T>::loaders.find(extension);

		// On renvoie le loader approprié
		if(it != MediaHandler<T>::loaders.end())
			return *it->second;
	}

	throw std::runtime_error("No loader for file: " + filename);
}

}

#endif
