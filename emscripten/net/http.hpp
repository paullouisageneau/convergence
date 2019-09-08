/*************************************************************************
 *	 Copyright (C) 2017-2019 by Paul-Louis Ageneau						 *
 *	 paul-louis (at) ageneau (dot) org									 *
 *																		 *
 *	 This file is part of Plateform.									 *
 *																		 *
 *	 Plateform is free software: you can redistribute it and/or modify	 *
 *	 it under the terms of the GNU Affero General Public License as		 *
 *	 published by the Free Software Foundation, either version 3 of		 *
 *	 the License, or (at your option) any later version.				 *
 *																		 *
 *	 Plateform is distributed in the hope that it will be useful, but	 *
 *	 WITHOUT ANY WARRANTY; without even the implied warranty of			 *
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the		 *
 *	 GNU Affero General Public License for more details.				 *
 *																		 *
 *	 You should have received a copy of the GNU Affero General Public	 *
 *	 License along with Plateform.										 *
 *	 If not, see <http://www.gnu.org/licenses/>.						 *
 *************************************************************************/

#ifndef NET_HTTPREQUEST_H
#define NET_HTTPREQUEST_H

#include <functional>
#include <future>
#include <string>
#include <vector>

namespace net {

using std::function;
using std::future;
using std::promise;
using std::string;

// HTTP fetch wrapper for emscripten
class Http {
public:
	class Response {
	public:
		Response(Response &&response) = default;
		Response(const Response &response) = delete;
		Response(int status, const char *data, size_t size); // free() will be called on data
		~Response(void);

		Response &operator=(Response &&response) = default;
		Response &operator=(const Response &response) = delete;

		int status(void) const;
		const char *data(void) const;
		size_t size(void) const;

	private:
		int mStatus;
		const char *mData;
		size_t mSize;
	};

	class Request {
	public:
		Request(void);
		Request(const string &method, const string &url);
		~Request(void);

		Request &target(const string &method, const string &url);
		Request &header(const string &name, const string &value);
		Request &body(const char *data, size_t size);
		void fetch(
		    function<void(Response &&)> responseCallback = [](Response &&) {},
		    function<void(const string &)> errorCallback = [](const string &) {});
		void abort(void);

	private:
		void triggerResponse(Response &&response);
		void triggerError(const string &error);

		int mId;
		function<void(Response &&)> mResponseCallback;
		function<void(const string &)> mErrorCallback;

		static void ResponseCallback(int status, const char *data, int size, void *ptr);
		static void ErrorCallback(const char *error, void *ptr);
	};

private:
	~Http(void);
};

} // namespace net

#endif // NET_HTTPREQUEST_H
