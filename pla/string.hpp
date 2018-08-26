/*************************************************************************
 *   Copyright (C) 2011-2013 by Paul-Louis Ageneau                       *
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

#ifndef PLA_STRING_H
#define PLA_STRING_H

#include "pla/include.hpp"

namespace pla
{

using std::string;
using std::to_string;
using std::strlen;

inline string tolower(string s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { 
		return std::tolower(c);
	});
	return s;
}

inline string toupper(string s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { 
		return std::toupper(c);
	});
	return s;
}

inline bool isalpha(const string &s) {
	return std::all_of(s.begin(), s.end(), [](unsigned char c) { 
		return std::isalpha(c);
	});
}

inline bool isdigit(const string &s) {
	return std::all_of(s.begin(), s.end(), [](unsigned char c) { 
		return std::isdigit(c);
	});
}

inline bool isalnum(const string &s) {
	return std::all_of(s.begin(), s.end(), [](unsigned char c) { 
		return std::isalnum(c);
	});
}

inline bool hasalpha(const string &s) {
	return std::any_of(s.begin(), s.end(), [](unsigned char c) { 
		return std::isalpha(c);
	});
}

inline bool hasdigit(const string &s) {
	return std::any_of(s.begin(), s.end(), [](unsigned char c) { 
		return std::isdigit(c);
	});
}

inline bool hasalnum(const string &s) {
	return std::any_of(s.begin(), s.end(), [](unsigned char c) { 
		return std::isalnum(c);
	});
}

inline string trim(const string &s) {
	auto isspace = [](unsigned char c) {
	   return std::isspace(c);
	};
	auto front = std::find_if_not(s.begin(), s.end(), isspace);
	auto back  = std::find_if_not(s.rbegin(), s.rend(), isspace).base();
	return back > front ? string(front, back) : string();
}

template<typename OutIterator>
void split(const string &str, char delimiter, OutIterator result) {
	std::istringstream ss(str);
	string item;
	while (std::getline(ss, item, delimiter)) {
		*(result++) = item;
	}
}

inline std::vector<string> split(const string &str, char delimiter) {
	std::vector<string> elements;
	split(str, delimiter, std::back_inserter(elements));
	return elements;
}

}

#endif
