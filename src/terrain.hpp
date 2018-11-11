/***************************************************************************
 *   Copyright (C) 2015-2016 by Paul-Louis Ageneau                         *
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

#ifndef CONVERGENCE_ISLAND_H
#define CONVERGENCE_ISLAND_H

#include "src/include.hpp"
#include "src/surface.hpp"
#include "src/ledger.hpp"

namespace convergence
{

class Terrain : public Collidable, public Ledger::Processor
{
public:
	Terrain(shared_ptr<Ledger> ledger, unsigned int seed);
	~Terrain(void);

	void update(double time);
	int draw(const Context &context);
	float intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection = NULL);

	void build(const vec3 &p, int weight);
	void dig(const vec3 &p, int weight, float radius);

private:
	class Operation : public Ledger::Entry
	{
	public:
		Operation(const Surface::int3 &p, Surface::value v);
		Operation(const binary &data);
		Type type(void) const { return Entry::Terrain; }
		binary toBinary(void) const;
		bool merge(shared_ptr<Entry> entry, bool replace);
		void apply(Surface *surface) const;
		
	private:
		Surface::int3 mPosition;
		Surface::value mValue;
	};
	
	shared_ptr<Ledger::Entry> createEntry(Ledger::Entry::Type type, const binary &data);
	void applyEntry(shared_ptr<Ledger::Entry> entry);
	void appendEntries(const std::list<shared_ptr<Operation>> &ops);
	
	sptr<Ledger> mLedger;
	Surface mSurface;
};

}

#endif
