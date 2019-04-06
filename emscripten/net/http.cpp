/*************************************************************************
 *   Copyright (C) 2017-2019 by Paul-Louis Ageneau                       *
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

#include "net/http.hpp"

#include <emscripten/emscripten.h>
#include <exception>
#include <iostream>
#include <utility>

#include <cstdlib> // for std::free()

extern "C" {
extern int httpCreateRequest(const char *method, const char *url);
extern void httpDeleteRequest(int rq);
extern void httpSetRequestHeader(int rq, const char *name, const char *value);
extern void httpSetRequestBody(int rq, const char *data, int size);
extern void httpSetUserPointer(int rq, void *ptr);
extern void httpFetch(int rq, void (*responseCallback)(int, const char *, int, void *),
					  void (*errorCallback)(const char *, void *));
extern void httpAbort(int rq);
}

namespace net {

void Http::Request::ResponseCallback(int status, const char *data, int size, void *ptr) {
	Http::Request *r = static_cast<Http::Request *>(ptr);
	if (r)
		r->triggerResponse(Response(status, data, size));
}

void Http::Request::ErrorCallback(const char *error, void *ptr) {
	Http::Request *r = static_cast<Http::Request *>(ptr);
	if (r)
		r->triggerError(string(error ? error : "unknown"));
}

Http::Request::Request(void) : mId(0) {}

Http::Request::Request(const string &method, const string &url) : Request() { target(method, url); }

Http::Request::~Request(void) {
	if (mId)
		httpDeleteRequest(mId);
}

Http::Request &Http::Request::target(const string &method, const string &url) {
	if (mId)
		httpDeleteRequest(mId);
	mId = httpCreateRequest(method.c_str(), url.c_str());
	httpSetUserPointer(mId, this);
	return *this;
}

Http::Request &Http::Request::header(const string &name, const string &value) {
	httpSetRequestHeader(mId, name.c_str(), value.c_str());
	return *this;
}

Http::Request &Http::Request::body(const char *data, size_t size) {
	httpSetRequestBody(mId, data, size);
	return *this;
}

void Http::Request::fetch(function<void(Response &&)> responseCallback,
						  function<void(const string &)> errorCallback) {
	if (mResponseCallback)
		abort();
	mResponseCallback = responseCallback;
	mErrorCallback = errorCallback;
	httpFetch(mId, ResponseCallback, ErrorCallback);
}

void Http::Request::abort(void) {
	mResponseCallback = nullptr;
	mErrorCallback = nullptr;
	httpAbort(mId);
}

void Http::Request::triggerResponse(Response &&response) {
	if (mResponseCallback)
		mResponseCallback(std::forward<Response>(response));
}

void Http::Request::triggerError(const string &error) {
	if (mErrorCallback)
		mErrorCallback(error);
}

Http::Response::Response(int status, const char *data, size_t size)
	: mStatus(status), mData(data), mSize(size) {}

Http::Response::~Response(void) {
	std::free(const_cast<char *>(mData)); // data must be freed
}

int Http::Response::status(void) const { return mStatus; }

const char *Http::Response::data(void) const { return mData; }

size_t Http::Response::size(void) const { return mSize; }

} // namespace net

