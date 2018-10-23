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
	mCurrentAncestor.clear();
	
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
	for(const auto &b : mCurrentBlocks) formatter << b;
	
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
	auto block = std::make_shared<Block>(mCurrentBlocks, entries);
	
	binary data = block->toBinary();
	binary digest = Hash(data);
	
	if(addCurrentBlock(digest, block))
	{
		std::cout << "Emitting block " << to_hex(digest) << std::endl;
		
		Message message(Message::LedgerBlock);
		message.payload = data;
		mMessageBus->send(message);
	}
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
		doResolve(digest, block);
		tryResolveAll();
		return std::make_pair(block, true);
	}
	
	mUnresolvedBlocks[digest] = block;
	return std::make_pair(block, false);
}

bool Ledger::isResolvable(shared_ptr<Block> block)
{
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
			if(isResolvable(it->second))
			{
				doResolve(it->first, it->second);
				it = mUnresolvedBlocks.erase(it);
				++resolved;
			}
			else ++it;
		}
	}
	while(resolved);
}

void Ledger::doResolve(const binary &digest, shared_ptr<Block> block)
{
	std::cout << "Resolved block " << to_hex(digest) << std::endl;
	
	if(addCurrentBlock(digest, block))
	{
		auto it = mBlocks.find(mCurrentAncestor);
		if(it != mBlocks.end()) apply(it->second);
	}
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

void Ledger::apply(shared_ptr<Block> block)
{
	for(auto e : block->entries) apply(e);
}

void Ledger::apply(shared_ptr<Entry> entry)
{
	auto it = mProcessors.find(entry->type());
	if(it != mProcessors.end())
	{
		if(auto processor = it->second.lock())
		{
			processor->applyEntry(entry);
		}
		else {
			mProcessors.erase(it);
		}
	}
}

bool Ledger::addCurrentBlock(const binary &digest, shared_ptr<Block> block)
{
	mBlocks[digest] = block;
	
	for(const auto &p : block->parents)
	{
		mCurrentBlocks.erase(p);
		
		auto it = mBlocks.find(p);
		if(it != mBlocks.end()) it->second->childs[digest] = block;
	}
	
	if(mCurrentBlocks.empty())
	{
		// The block is a merge
		mCurrentAncestor = digest;
		mCurrentBlocks.insert(digest);
		return true;
	}
	
	std::unordered_set<binary, binary_hash> ancestors, visited;
	ancestors.insert(mCurrentAncestor);
	while(!ancestors.empty())
	{
		for(const auto &a : ancestors)
		{
			auto it = mBlocks.find(a);
			if(it != mBlocks.end() && it->second->findDescendant(digest, visited))
			{
				std::cout << "Found ancestor block " << to_hex(a) << std::endl;
				
				mCurrentAncestor = a;
				mCurrentBlocks.insert(digest);
				return true;
			}
			
			visited.insert(a);
		}

		std::unordered_set<binary, binary_hash> nextAncestors;
		for(const auto &a : ancestors)
		{
			auto it = mBlocks.find(a);
			if(it != mBlocks.end())
			{
				for(const auto &p : it->second->parents)
					if(visited.find(p) == visited.end())
						nextAncestors.insert(p);
			}
		}
		
		std::swap(ancestors, nextAncestors);
	}
	
	std::cerr << "Unable to add current block: no common ancestor" << std::endl;
	return false;
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

shared_ptr<Ledger::Block> Ledger::Block::findDescendant(const binary &digest, const std::unordered_set<binary, binary_hash> &blacklist) const
{
	auto it = childs.find(digest);
	if(it != childs.end()) return it->second;
	
	for(const auto &c : childs)
		if(blacklist.find(c.first) == blacklist.end())
			if(shared_ptr<Block> block = c.second->findDescendant(digest, blacklist))
				return block;
	
	return nullptr;
}

binary Ledger::Hash(const binary &data)
{
	binary digest;
	Sha3_256(data, digest);
	return digest;
}

}


