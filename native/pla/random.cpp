/*************************************************************************
 *   Copyright (C) 2011-2014 by Paul-Louis Ageneau                       *
 *   paul-louis (at) ageneau (dot) org                                   *
 *                                                                       *
 *   This file is part of Plateform.                                     *
 *                                                                       *
 *   Plateform is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Affero General Public License as      *
 *   published by the Free Software Foundation, either version 3 of      *
 *   the License, or (at your option) any later version.                 *
 *                                                                       *
 *   Plateform is distributed in the hope that it will be useful, but    *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        *
 *   GNU Affero General Public License for more details.                 *
 *                                                                       *
 *   You should have received a copy of the GNU Affero General Public    *
 *   License along with Plateform.                                       *
 *   If not, see <http://www.gnu.org/licenses/>.                         *
 *************************************************************************/

#include "pla/random.hpp"
#include "pla/string.hpp"

#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>

namespace pla
{

Random::Random(QualityLevel level) :
	mLevel(level)
{

}

Random::~Random(void)
{

}

void Random::generate(byte *buffer, size_t size) const {
	gnutls_rnd_level_t level;
	switch(mLevel)
	{
		case Crypto:	level = GNUTLS_RND_RANDOM;	break;
		case Key:		level = GNUTLS_RND_KEY;		break;
		default:		level = GNUTLS_RND_NONCE;	break;
	}

	int ret = gnutls_rnd(level, buffer, size);
	if(ret < 0) throw std::runtime_error("Random generator error: " + string(gnutls_strerror(ret)));
}

size_t Random::readSome(byte *buffer, size_t size) {
	generate(buffer, size);
	return size;
}

size_t Random::writeSome(const byte *data, size_t size) {
	throw std::logic_error("Writing to random generator");
}
}
