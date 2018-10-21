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

Ledger::Ledger(shared_ptr<MessageBus> messageBus) :
	mMessageBus(messageBus)
{

}

Ledger::~Ledger(void)
{
	
}

void Ledger::update(void)
{
	Message message;
	while(readMessage(message)) processMessage(message);
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
	
	mBlocks[digest] = block;
	mCurrentBlocks.clear();
	mCurrentBlocks.insert(digest);
	
	std::cout << "Emitting block " << pla::to_hex(digest) << std::endl;
	
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
				for(const auto &b : missing)
				{
					std::cout << "Requesting block " << pla::to_hex(b) << std::endl;
					
					Message message(Message::LedgerRequest);
					message.destination = message.source;
					message.payload = b;
					mMessageBus->send(message);
				}
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
		
		default:
			// Ignore
			break;
	}
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
	
	std::cout << "Incoming block " << pla::to_hex(digest) << std::endl;
	
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
	std::cout << "Resolved block " << pla::to_hex(digest) << std::endl;
	
	for(const auto &p : block->parents)
		mCurrentBlocks.erase(p);
	
	mBlocks[digest] = block;
	mCurrentBlocks.insert(digest);
	
	for(auto entry : block->entries) apply(entry);
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

binary Ledger::Hash(const binary &data)
{
	binary digest;
	Sha3_256(data, digest);
	return digest;
}

}


