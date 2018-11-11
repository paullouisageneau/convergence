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

#include "src/ledger.hpp"
#include "src/sha3.hpp"

#include "pla/binaryformatter.hpp"

namespace convergence
{

using pla::BinaryFormatter;
using pla::to_hex;

Ledger::Ledger(shared_ptr<MessageBus> messageBus) :
	mMessageBus(messageBus)
{

}

Ledger::~Ledger(void)
{
	
}

void Ledger::init(void)
{
	mBlocks.clear();
	mUnresolvedBlocks.clear();
	mCurrentBlocks.clear();
	
	binary data;
	auto entry = std::make_shared<GenericEntry>(Entry::Genesis, data);
	
	std::list<shared_ptr<Entry>> entries;
	entries.push_back(entry);
	append(entries);
}

void Ledger::update(void)
{
	Message message;
	while(readMessage(message)) processMessage(message);
}

void Ledger::sync(const identifier &destination)
{
	BinaryFormatter formatter;
	for(const auto &p : mCurrentBlocks) formatter << p.first;
	
	Message message(Message::LedgerCurrent);
	message.payload = formatter.data();
	message.destination = destination;
	mMessageBus->send(message);
}

void Ledger::registerProcessor(Entry::Type type, weak_ptr<Processor> processor)
{
	mProcessors[type] = processor;
}

void Ledger::append(const std::list<shared_ptr<Entry>> &entries)
{
	std::set<binary> parents;
	for(const auto &p : mCurrentBlocks) parents.insert(p.first);
	
	auto block = std::make_shared<Block>(parents, entries);
	binary data = block->toBinary();
	binary digest = Hash(data);
	
	addCurrentBlock(digest, block);
	
	std::cout << "Emitting block " << to_hex(digest) << std::endl;
		
	Message message(Message::LedgerBlock);
	message.payload = data;
	mMessageBus->send(message);
}

void Ledger::processMessage(const Message &message)
{
	switch(message.type)
	{
		case Message::LedgerBlock:
		{
			shared_ptr<Block> block;
			bool resolved;
			std::tie(block, resolved) = createBlock(message.payload);
			if(!resolved)
			{
				std::set<binary> missing;
				getMissingAncestors(block, missing);
				for(const auto &b : missing) sendBlockRequest(message.source, b);
			}
			break;
		}
		
		case Message::LedgerRequest:
		{
			auto it = mBlocks.find(message.payload);
			if(it != mBlocks.end())
			{
				Message message(Message::LedgerBlock);
				message.payload = it->second->toBinary();
				mMessageBus->send(message);
			}
			break;
		}
		
		case Message::LedgerCurrent:
		{
			BinaryFormatter formatter(message.payload);
			binary digest(32);
			while(formatter >> digest)
			{
				if(mBlocks.find(digest) != mBlocks.end()) continue;
				
				auto it = mUnresolvedBlocks.find(digest);
				if(it != mUnresolvedBlocks.end())
				{
					std::set<binary> missing;
					getMissingAncestors(it->second, missing);
					for(const auto &b : missing) sendBlockRequest(message.source, b);
				}
				else {
					sendBlockRequest(message.source, digest);
				}
			}
			break;
		}
		
		default:
			// Ignore
			break;
	}
}

void Ledger::sendBlockRequest(const identifier &destination, const binary &digest)
{
	std::cout << "Requesting block " << to_hex(digest) << std::endl;
	
	Message message(Message::LedgerRequest);
	message.payload = digest;
	message.destination = destination;
	mMessageBus->send(message);
}

shared_ptr<Ledger::Entry> Ledger::createEntry(Entry::Type type, const binary &data)
{
	auto it = mProcessors.find(type);
	if(it != mProcessors.end())
	{
		if(auto processor = it->second.lock())
		{
			if(auto entry = processor->createEntry(type, data))
				return entry;
		}
		else {
			mProcessors.erase(it);
		}
	}
	else {
		std::cerr << "No ledger entry processor found for type " << type << std::endl;
	}
	
	return std::make_shared<GenericEntry>(type, data);
}

std::pair<shared_ptr<Ledger::Block>, bool> Ledger::createBlock(const binary &data)
{
	binary digest = Hash(data);
	
	std::cout << "Incoming block " << to_hex(digest) << std::endl;
	
	auto it = mBlocks.find(digest);
	if(it != mBlocks.end()) return std::make_pair(it->second, true);
	
	auto jt = mUnresolvedBlocks.find(digest);
	if(jt != mUnresolvedBlocks.end()) return std::make_pair(jt->second, false);
	
	auto block = std::make_shared<Block>(this, data);
	
	if(isResolvable(block)) 
	{
		auto state = addCurrentBlock(digest, block);
		for(auto e : state->entries) apply(e);
		
		tryResolveAll();
		return std::make_pair(block, true);
	}
	
	mUnresolvedBlocks[digest] = block;
	return std::make_pair(block, false);
}

bool Ledger::isResolvable(shared_ptr<Block> block)
{
	if(block->parents.empty()) return false;
	
	for(const auto &p : block->parents)
		if(mBlocks.find(p) == mBlocks.end())
			return false;
	
	return true;
}

void Ledger::tryResolveAll(void)
{
	int resolved;
	do {
		resolved = 0;
		auto it = mUnresolvedBlocks.begin();
		while(it != mUnresolvedBlocks.end())
		{
			sptr<Block> block = it->second;
			if(isResolvable(block))
			{
				auto state = addCurrentBlock(it->first, block);
				for(auto e : state->entries) apply(e);
				
				it = mUnresolvedBlocks.erase(it);
				++resolved;
			}
			else ++it;
		}
	}
	while(resolved);
}

void Ledger::getMissingAncestors(shared_ptr<Block> block, std::set<binary> &missing)
{
	for(const auto &p : block->parents)
	{
		if(mBlocks.find(p) == mBlocks.end())
		{
			auto it = mUnresolvedBlocks.find(p);
			if(it != mUnresolvedBlocks.end()) getMissingAncestors(it->second, missing);
			else missing.insert(p);
		}
	}
}

shared_ptr<Ledger::State> Ledger::addCurrentBlock(const binary &digest, shared_ptr<Block> block)
{
	std::cout << "Adding block " << to_hex(digest) << std::endl;
	
	block->state = std::make_shared<State>();
	for(const auto &b : block->parents)
	{
		auto it = mBlocks.find(b);
		if(it != mBlocks.end()) 
		{
			block->state->merge(it->second->state, false);
		}
		
		auto jt = mCurrentBlocks.find(b);
		if(jt != mCurrentBlocks.end()) 
		{
			mCurrentBlocks.erase(jt);
		}
	}
	
	block->state->merge(std::make_shared<State>(block->entries), true); // replace

	mBlocks[digest] = block;
	mCurrentBlocks[digest] = block->state;
	return block->state;
}

void Ledger::apply(shared_ptr<Entry> entry)
{
	auto it = mProcessors.find(entry->type());
	if(it != mProcessors.end())
	{
		if(auto processor = it->second.lock()) processor->applyEntry(entry);
		else mProcessors.erase(it);
	}
}

Ledger::GenericEntry::GenericEntry(Entry::Type type, const binary &data) :
	mType(type),
	mData(data)
{

}

Ledger::Entry::Type Ledger::GenericEntry::type(void) const
{
	return mType;
}

binary Ledger::GenericEntry::toBinary(void) const
{
	return mData;
}

bool Ledger::GenericEntry::merge(shared_ptr<Entry> entry, bool replace)
{
	// Degraded behavior: just return true if entries are the same
	// If the types are equal, then the other entry is generic too and toBinary() is just a getter
	return entry->type() == mType && entry->toBinary() == mData;
}

Ledger::Block::Block(const std::set<binary> &_parents, const std::list<shared_ptr<Entry>> &_entries) :
	parents(_parents),
	entries(_entries)
{
	
}

Ledger::Block::Block(Ledger *ledger, const binary &data)
{
	BinaryFormatter formatter(data);
	
	uint32_t parentsCount = 0;
	formatter >> parentsCount;
	for(uint32_t i = 0; i < parentsCount; ++i)
	{
		binary p(32);
		formatter >> p;
		parents.insert(p);
	}
	
	uint32_t entriesCount = 0;
	formatter >> entriesCount;
	for(uint32_t i = 0; i < entriesCount; ++i)
	{
		uint8_t type = 0;
		uint8_t size = 0;
		formatter >> type >> size;
		binary b(size);
		formatter >> b;
		entries.push_back(ledger->createEntry(Entry::Type(type), b));
	}
}

binary Ledger::Block::toBinary(void) const
{
	BinaryFormatter formatter;
	
	formatter << uint32_t(parents.size());
	for(const auto &p : parents)
	{
		formatter << p;
	}
	
	formatter << uint32_t(entries.size());
	for(const auto &e : entries) 
	{
		binary b = e->toBinary();
		formatter << uint8_t(e->type());
		formatter << uint8_t(b.size());
		formatter << b;
	}
	
	return formatter.data();
}

Ledger::State::State(void)
{
	
}

Ledger::State::State(const std::list<shared_ptr<Entry>> &_entries) :
	entries(_entries)
{
	
}

void Ledger::State::merge(shared_ptr<State> state, bool replace)
{
	std::list<shared_ptr<Entry>> left;
	
	for(auto e : state->entries)
	{	
		bool merged = false;
		for(auto m : entries)
		{
			merged = m->merge(e, replace);
			if(merged) break;
		}
		
		if(!merged) left.push_back(e);
	}
	
	entries.insert(entries.end(), left.begin(), left.end());
}

binary Ledger::Hash(const binary &data)
{
	binary digest;
	Sha3_256(data, digest);
	return digest;
}

}


