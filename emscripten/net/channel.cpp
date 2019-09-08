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

#include "net/channel.hpp"

namespace net {

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

void Channel::onOpen(std::function<void()> callback) { mOpenCallback = callback; }

void Channel::onClosed(std::function<void()> callback) { mClosedCallback = callback; }

void Channel::onError(std::function<void(const string &)> callback) { mErrorCallback = callback; }

void Channel::onMessage(std::function<void(const std::variant<binary, string> &data)> callback) {
	mMessageCallback = callback;
}

void Channel::onMessage(std::function<void(const binary &data)> binaryCallback,
                        std::function<void(const string &data)> stringCallback) {
	onMessage([binaryCallback, stringCallback](const std::variant<binary, string> &data) {
		std::visit(overloaded{binaryCallback, stringCallback}, data);
	});
}

void Channel::triggerOpen(void) {
	if (mOpenCallback)
		mOpenCallback();
}

void Channel::triggerClosed(void) {
	if (mClosedCallback)
		mClosedCallback();
}

void Channel::triggerError(const string &error) {
	if (mErrorCallback)
		mErrorCallback(error);
}

void Channel::triggerMessage(const std::variant<binary, string> &data) {
	if (mMessageCallback)
		mMessageCallback(data);
}
} // namespace net
