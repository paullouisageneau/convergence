// "use strict";

(function() {
	var Http = {
		$HTTP: {
			map: {},
			nextId: 1,

			allocUTF8FromString: function(str) {
				var strLen = lengthBytesUTF8(str);
				var strOnHeap = _malloc(strLen+1);
				stringToUTF8(str, strOnHeap, strLen+1);
				return strOnHeap;
			},

			registerRequest: function(request) {
				var rq = HTTP.nextId++;
				HTTP.map[rq] = request;
				return rq;
			},
		},

		httpCreateRequest: function(pMethod, pUrl) {
			if(!window.fetch) return 0;
			var method = UTF8ToString(pMethod);
			var url = UTF8ToString(pUrl);
			var headers = {};
			return HTTP.registerRequest({ url, method, headers });
		},

		httpDeleteRequest: function(rq) {
			var request = HTTP.map[rq];
			if(request) {
				if(request.abort) request.abort();
				delete HTTP.map[rq];
			}
		},

		httpSetRequestHeader: function(rq, pName, pValue) {
			var name = UTF8ToString(pName);
			var value = UTF8ToString(pValue);
			var request = HTTP.map[rq];
			if(request) request.headers[name] = value;
		},

		httpSetRequestBody: function(rq, pData, size) {
			var request = HTTP.map[rq];
			if(request) request.body = new Uint8Array(Module.HEAPU8.buffer, pData, size);
		},

		httpSetUserPointer: function(rq, ptr) {
			var request = HTTP.map[rq];
			if(request) request.httpUserPointer = ptr;
		},

		httpFetch: function(rq, responseCallback, errorCallback) {
			var request = HTTP.map[rq];
			if(!request) return 0;
			request.httpAbortCount = request.httpAbortCount || 0;
			var currentAbortCount = request.httpAbortCount;
			if (window.AbortController) {
				controller = new AbortController();
				request.signal = controller.signal;
				request.abort = function() {
					controller.abort();
					request.httpAbortCount+= 1;
				};
			} else {
				request.abort = function() {
					request.httpAbortCount+= 1;
				}
			}
			var userPointer = request.httpUserPointer || 0;
			fetch(request.url, request)
			.then(function(response) {
				request.response = response;
				return response.arrayBuffer();
			})
			.then(function(arrayBuffer) {
				if(request.httpAbortCount > currentAbortCount) return;
				var response = request.response;
				var byteArray = new Uint8Array(arrayBuffer);
				var size = byteArray.byteLength;
				var pBuffer = size ? _malloc(size) : 0;
				var heapBytes = new Uint8Array(Module.HEAPU8.buffer, pBuffer, size);
				heapBytes.set(byteArray);
				Module.dynCall_viiii(responseCallback, response.status, pBuffer, size, userPointer);
				// WARNING: pBuffer is not freed here and therefore must be from C/C++ code
			})
			.catch(function(error) {
				console.error(error);
				if(request.httpAbortCount > currentAbortCount) return;
				Module.dynCall_vii(errorCallback, 0, userPointer);
			});
		},

		httpAbort: function(rq) {
			var request = HTTP.map[rq];
			if(request && request.abort) request.abort();
		},
	};

	autoAddDeps(Http, '$HTTP');
	mergeInto(LibraryManager.library, Http);
})();

