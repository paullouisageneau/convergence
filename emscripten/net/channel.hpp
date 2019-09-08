/*************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                       *
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

#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace net {

using std::byte;
using std::function;
using std::string;
typedef std::vector<byte> binary;

class Channel {
public:
	virtual void close(void) = 0;
	virtual void send(const std::variant<binary, string> &data) = 0;

	virtual bool isOpen(void) const = 0;
	virtual bool isClosed(void) const = 0;

	void onOpen(function<void()> callback);
	void onClosed(function<void()> callback);
	void onError(function<void(const string &error)> callback);
	void onMessage(function<void(const std::variant<binary, string> &data)> callback);
	void onMessage(function<void(const binary &data)> binaryCallback,
	               function<void(const string &data)> stringCallback);

protected:
	virtual void triggerOpen(void);
	virtual void triggerClosed(void);
	virtual void triggerError(const string &error);
	virtual void triggerMessage(const std::variant<binary, string> &data);

private:
	function<void()> mOpenCallback;
	function<void()> mClosedCallback;
	function<void(const string &error)> mErrorCallback;
	function<void(const std::variant<binary, string> &data)> mMessageCallback;
};

} // namespace net

#endif // NET_CHANNEL_H
