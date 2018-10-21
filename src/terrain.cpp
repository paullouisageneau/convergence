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

#include "src/world.hpp"

#include "pla/binaryformatter.hpp"

namespace convergence
{

using pla::BinaryFormatter;

Terrain::Terrain(shared_ptr<Ledger> ledger, unsigned int seed) :
	mLedger(ledger),
	mSurface(seed)
{

}

Terrain::~Terrain(void)
{

}

void Terrain::update(double time)
{
	mSurface.update(time);
}

int Terrain::draw(const Context &context)
{
	return mSurface.draw(context);
}

float Terrain::intersect(const vec3 &pos, const vec3 &move, float radius, vec3 *intersection)
{
	return mSurface.intersect(pos, move, radius, intersection);
}

void Terrain::build(const vec3 &p, int weight)
{
	if(weight == 0) return;
	
	const Surface::int3 i(p + vec3(0.5f));
	const int type = 0;
	
	Surface::value pv = mSurface.getValue(i);
	Surface::value v = mSurface.addWeight(i, weight, type);
	
	if(v != pv) 
	{
		std::list<shared_ptr<Operation>> ops;
		ops.push_back(std::make_shared<Operation>(i, v));
		appendEntries(ops);
	}
}

void Terrain::dig(const vec3 &p, int weight, float radius)
{
	if(weight == 0 || radius <= 0.f) return;

	const Surface::int3 origin(p + vec3(0.5f));
	const int d = int(radius) + 1;
	
	std::list<shared_ptr<Operation>> ops;
	for(int dx=-d; dx<=d+1; ++dx)
		for(int dy=-d; dy<=d+1; ++dy)
			for(int dz=-d; dz<=d+1; ++dz)
			{
				Surface::int3 i = origin + Surface::int3(dx, dy, dz);
				vec3 c = vec3(i.x, i.y, i.z) + vec3(0.5f);
				float t = 1.f - glm::distance(p, c)/radius;
				if(t > 0.f)
				{
					int w = int(weight*t);
					Surface::value pv = mSurface.getValue(i);
					Surface::value v = mSurface.addWeight(i, w, -1);
					if(v != pv) ops.push_back(std::make_shared<Operation>(i, v));
				}
			}
	
	if(!ops.empty()) appendEntries(ops);
}

shared_ptr<Ledger::Entry> Terrain::createEntry(Ledger::Entry::Type type, const binary &data)
{
	if(type == Ledger::Entry::Terrain) return std::make_shared<Operation>(data);
	else return nullptr;
}

void Terrain::applyEntry(shared_ptr<Ledger::Entry> entry)
{
	if(entry->type() == Ledger::Entry::Terrain)
	{
		shared_ptr<Operation> operation = std::dynamic_pointer_cast<Operation>(entry);
		operation->apply(&mSurface);
	}
}

void Terrain::appendEntries(const std::list<shared_ptr<Operation>> &ops)
{
	mLedger->append(std::list<shared_ptr<Ledger::Entry>>(ops.begin(), ops.end()));
}

Terrain::Operation::Operation(const Surface::int3 &p, Surface::value v) :
	mPosition(p),
	mValue(v)
{

}

Terrain::Operation::Operation(const binary &data)
{
	// TODO: 16 bits is probably too small here
	
	BinaryFormatter formatter(data);
	int16_t x = 0;
	int16_t y = 0;
	int16_t z = 0;
	formatter >> x >> y >> z;
	mPosition = Surface::int3(x, y, z);

	uint8_t t = 0;
	uint8_t w = 0;
	formatter >> t >> w;
	mValue = Surface::value(t, w);
}

binary Terrain::Operation::toBinary(void) const
{
	BinaryFormatter formatter;
	formatter << int16_t(mPosition.x) << int16_t(mPosition.y) << int16_t(mPosition.z);
	formatter << uint8_t(mValue.type) << uint8_t(mValue.weight);
	return formatter.data();
}

void Terrain::Operation::apply(Surface *surface) const
{
	surface->setValue(mPosition, mValue);
}

}

