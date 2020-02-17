/***************************************************************************
 *   Copyright (C) 2006-2020 by Paul-Louis Ageneau                         *
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

#ifndef PLA_TEXT_H
#define PLA_TEXT_H

#include "pla/font.hpp"
#include "pla/include.hpp"
#include "pla/texture.hpp"

#include <string>

namespace pla {

class Text {
public:
	Text(shared_ptr<Font> font, std::string content = "");
	virtual ~Text();

	std::string content() const;

	void setContent(std::string content);

private:
	std::string mContent;
};

} // namespace pla

#endif // PLA_TEXT_H
