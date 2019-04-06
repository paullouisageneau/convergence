// "use strict";

(function() {
	var WebSocket = {
		$WEBSOCKET: {
			map: {},
			nextId: 1,

			allocUTF8FromString: function(str) {
				var strLen = lengthBytesUTF8(str);
				var strOnHeap = _malloc(strLen+1);
				stringToUTF8(str, strOnHeap, strLen+1);
				return strOnHeap;
			},

			registerWebSocket: function(webSocket) {
				var ws = WEBSOCKET.nextId++;
				WEBSOCKET.map[ws] = webSocket;
				webSocket.binaryType = 'arraybuffer';
				return ws;
			},
		},

		wsCreateWebSocket: function(pUrl) {
			var url = UTF8ToString(pUrl);
			if(!window.WebSocket) return 0;
			return WEBSOCKET.registerWebSocket(new WebSocket(url));
		},

		wsDeleteWebSocket: function(ws) {
			var webSocket = WEBSOCKET.map[ws];
			if(webSocket) {
				webSocket.close();
				webSocket.wsUserDeleted = true;
				delete WEBSOCKET.map[ws];
			}
		},

		wsSetOpenCallback: function(ws, openCallback) {
			var webSocket = WEBSOCKET.map[ws];
			var cb = function() {
				if(webSocket.rtcUserDeleted) return;
				var userPointer = webSocket.rtcUserPointer || 0;
				Module.dynCall_vi(openCallback, userPointer);
			};
			webSocket.onopen = cb;
			if(webSocket.readyState == 1) setTimeout(cb, 0);
		},

 		wsSetErrorCallback: function(ws, errorCallback) {
			var webSocket = WEBSOCKET.map[ws];
			var cb = function() {
				if(webSocket.rtcUserDeleted) return;
				var userPointer = webSocket.rtcUserPointer || 0;
				Module.dynCall_vii(errorCallback, 0, userPointer);
			};
			webSocket.onerror = cb;
		},

		wsSetMessageCallback: function(ws, messageCallback) {
			var webSocket = WEBSOCKET.map[ws];
			webSocket.onmessage = function(evt) {
				if(webSocket.rtcUserDeleted) return;
				var byteArray = new Uint8Array(evt.data);
				var size = byteArray.byteLength;
				if(!size) return;
				var pBuffer = _malloc(size);
				var heapBytes = new Uint8Array(Module.HEAPU8.buffer, pBuffer, size);
				heapBytes.set(byteArray);
				var userPointer = webSocket.rtcUserPointer || 0;
				Module.dynCall_viii(messageCallback, pBuffer, size, userPointer);
				_free(pBuffer);
			};
			webSocket.onclose = function() {
				if(webSocket.rtcUserDeleted) return;
				var userPointer = webSocket.rtcUserPointer || 0;
				Module.dynCall_viii(messageCallback, 0, 0, userPointer);
			};
		},

		wsSendMessage: function(ws, pBuffer, size) {
			var webSocket = WEBSOCKET.map[ws];
			if(webSocket.readyState != 1) return 0;
			var heapBytes = new Uint8Array(Module.HEAPU8.buffer, pBuffer, size);
			webSocket.send(heapBytes);
			return size;
		},

		wsSetUserPointer: function(ws, ptr) {
			var webSocket = WEBSOCKET.map[ws];
			if(webSocket) webSocket.rtcUserPointer = ptr;
		},
	};

	autoAddDeps(WebSocket, '$WEBSOCKET');
	mergeInto(LibraryManager.library, WebSocket);
})();

