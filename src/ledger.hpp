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

#ifndef CONVERGENCE_LEDGER_H
#define CONVERGENCE_LEDGER_H

#include "src/messagebus.hpp"
#include "src/include.hpp"

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

namespace convergence
{

class Ledger : public MessageBus::AsyncListener
{
public:
	Ledger(shared_ptr<MessageBus> messageBus);
	~Ledger(void);
	
	void update();
	void sync(const identifier &destination);
	
	class Entry
	{
	public:
		enum Type : uint8_t
		{
			Dummy = 0x00,
			Terrain = 0x10
		};
		
		virtual Type type(void) const = 0;
		virtual binary toBinary(void) const = 0;
	};
	
	class GenericEntry : public Entry
	{
	public:
		GenericEntry(Type type, const binary &data);
		Type type(void) const;
		binary toBinary(void) const;
		
	private:
		Type mType;
		binary mData;
	};
	
	class Processor
	{
	public:
		virtual shared_ptr<Entry> createEntry(Entry::Type type, const binary &data) = 0;
		virtual void applyEntry(shared_ptr<Entry> entry) = 0;
	};
	
	void registerProcessor(Entry::Type type, weak_ptr<Processor> processor);
	void append(const std::list<shared_ptr<Entry>> &entries);

private:
	struct Block
	{
		Block(const std::set<binary> &_parents, const std::list<shared_ptr<Entry>> &_entries);
		Block(Ledger *ledger, const binary &data);
		
		binary toBinary(void) const;
		
		std::set<binary> parents;
		std::list<shared_ptr<Entry>> entries;
	};
	
	void processMessage(const Message &message);
	void sendBlockRequest(const identifier &destination, const binary &digest);
	
	shared_ptr<Entry> createEntry(Entry::Type type, const binary &data);
	std::pair<shared_ptr<Block>, bool> createBlock(const binary &data);
	
	bool isResolvable(shared_ptr<Block> block);
	void tryResolveAll(void);
	void doResolve(const binary &digest, shared_ptr<Block> block);
	void getMissingAncestors(shared_ptr<Block> block, std::set<binary> &missing);
	void apply(shared_ptr<Block> block);
	void apply(shared_ptr<Entry> entry);
	
	shared_ptr<MessageBus> mMessageBus;
	std::map<Entry::Type, weak_ptr<Processor>> mProcessors;
	std::unordered_map<binary, shared_ptr<Block>, binary_hash> mBlocks;
	std::map<binary, shared_ptr<Block>> mUnresolvedBlocks;
	std::set<binary> mCurrentBlocks;
	
	static binary Hash(const binary &data);
};

}

#endif

