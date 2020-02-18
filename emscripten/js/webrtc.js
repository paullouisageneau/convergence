// "use strict";

(function() {
	var WebRTC = {
		$WEBRTC: {
			peerConnectionsMap: {},
			dataChannelsMap: {},
			nextId: 1,

			allocUTF8FromString: function(str) {
				var strLen = lengthBytesUTF8(str);
				var strOnHeap = _malloc(strLen+1);
				stringToUTF8(str, strOnHeap, strLen+1);
				return strOnHeap;
			},

			registerPeerConnection: function(peerConnection) {
				var pc = WEBRTC.nextId++;
				WEBRTC.peerConnectionsMap[pc] = peerConnection;
				peerConnection.onnegotiationneeded = function() {
					peerConnection.createOffer()
						.then(function(offer) {
							return WEBRTC.handleDescription(peerConnection, offer);
						})
						.catch(function(err) {
							console.error(err);
						});
				};
				peerConnection.onicecandidate = function(evt) {
					if(evt.candidate) WEBRTC.handleCandidate(peerConnection, evt.candidate);
				};
				return pc;
			},

			registerDataChannel: function(dataChannel) {
				var dc = WEBRTC.nextId++;
				WEBRTC.dataChannelsMap[dc] = dataChannel;
				dataChannel.binaryType = 'arraybuffer';
				return dc;
			},

			handleDescription: function(peerConnection, description) {
				return peerConnection.setLocalDescription(description)
					.then(function() {
						if(peerConnection.rtcUserDeleted) return;
						if(!peerConnection.rtcDescriptionCallback) return;
						var desc = peerConnection.localDescription;
						var pSdp = WEBRTC.allocUTF8FromString(desc.sdp);
						var pType = WEBRTC.allocUTF8FromString(desc.type);
						var callback =  peerConnection.rtcDescriptionCallback;
						var userPointer = peerConnection.rtcUserPointer || 0;
						Module['dynCall_viii'](callback, pSdp, pType, userPointer);
						_free(pSdp);
						_free(pType);
					});
			},

			handleCandidate: function(peerConnection, candidate) {
				if(peerConnection.rtcUserDeleted) return;
				if(!peerConnection.rtcCandidateCallback) return;
				var pCandidate = WEBRTC.allocUTF8FromString(candidate.candidate);
				var pSdpMid = WEBRTC.allocUTF8FromString(candidate.sdpMid);
				var candidateCallback =  peerConnection.rtcCandidateCallback;
				var userPointer = peerConnection.rtcUserPointer || 0;
				Module['dynCall_viii'](candidateCallback, pCandidate, pSdpMid, userPointer);
				_free(pCandidate);
				_free(pSdpMid);
			},
		},

		rtcCreatePeerConnection: function(pIceServers, nIceServers) {
			if(!window.RTCPeerConnection) return 0;
			var iceServers = [];
			for(var i = 0; i < nIceServers; ++i) {
				var p = Module.HEAPU32[pIceServers/Module.HEAPU32.BYTES_PER_ELEMENT + i];
				iceServers.push({
					urls: [UTF8ToString(p)],
				});
			}
			var config = {
				iceServers: iceServers,
			};
			return WEBRTC.registerPeerConnection(new RTCPeerConnection(config));
		},

		rtcDeletePeerConnection: function(pc) {
			var peerConnection = WEBRTC.peerConnectionsMap[pc];
			if(peerConnection) {
				peerConnection.rtcUserDeleted = true;
				delete WEBRTC.peerConnectionsMap[pc];
			}
		},

		rtcCreateDataChannel: function(pc, pLabel) {
			if(!pc) return 0;
			var label = UTF8ToString(pLabel);
			var peerConnection = WEBRTC.peerConnectionsMap[pc];
			var channel = peerConnection.createDataChannel(label);
			return WEBRTC.registerDataChannel(channel);
		},

 		rtcDeleteDataChannel: function(dc) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			if(dataChannel) {
				dataChannel.rtcUserDeleted = true;
				delete WEBRTC.dataChannelsMap[dc];
			}
		},

		rtcSetDataChannelCallback: function(pc, dataChannelCallback) {
			if(!pc) return;
			var peerConnection = WEBRTC.peerConnectionsMap[pc];
			peerConnection.ondatachannel = function(evt) {
				if(peerConnection.rtcUserDeleted) return;
				var dc = WEBRTC.registerDataChannel(evt.channel);
				var userPointer = peerConnection.rtcUserPointer || 0;
				Module['dynCall_vii'](dataChannelCallback, dc, userPointer);
			};
		},

		rtcSetLocalDescriptionCallback: function(pc, descriptionCallback) {
			if(!pc) return;
			var peerConnection = WEBRTC.peerConnectionsMap[pc];
			peerConnection.rtcDescriptionCallback = descriptionCallback;
		},

		rtcSetLocalCandidateCallback: function(pc, candidateCallback) {
			if(!pc) return;
			var peerConnection = WEBRTC.peerConnectionsMap[pc];
			peerConnection.rtcCandidateCallback = candidateCallback;
		},

		rtcSetRemoteDescription: function(pc, pSdp, pType) {
			var description = new RTCSessionDescription({
				sdp: UTF8ToString(pSdp),
				type: UTF8ToString(pType),
			});
			var peerConnection = WEBRTC.peerConnectionsMap[pc];
			peerConnection.setRemoteDescription(description)
				.then(function() {
					if(peerConnection.rtcUserDeleted) return;
					if(description.type == 'offer') {
						peerConnection.createAnswer()
							.then(function(answer) {
								return WEBRTC.handleDescription(peerConnection, answer);
							})
							.catch(function(err) {
								console.error(err);
							});
					}
				})
				.catch(function(err) {
					console.error(err);
				});
		},

		rtcAddRemoteCandidate: function(pc, pCandidate, pSdpMid) {
			var iceCandidate = new RTCIceCandidate({
				candidate: UTF8ToString(pCandidate),
				sdpMid: UTF8ToString(pSdpMid),
			});
			var peerConnection = WEBRTC.peerConnectionsMap[pc];
			peerConnection.addIceCandidate(iceCandidate)
				.catch(function(err) {
					console.error(err);
				});
		},

		rtcGetDataChannelLabel: function(dc, pBuffer, size) {
			var label = WEBRTC.dataChannelsMap[dc].label;
			stringToUTF8(label, pBuffer, size);
			return lengthBytesUTF8(label);
		},

		rtcSetOpenCallback: function(dc, openCallback) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			var cb = function() {
				if(dataChannel.rtcUserDeleted) return;
				var userPointer = dataChannel.rtcUserPointer || 0;
				Module['dynCall_vi'](openCallback, userPointer);
			};
			dataChannel.onopen = cb;
			if(dataChannel.readyState == 'open') setTimeout(cb, 0);
		},

		rtcSetErrorCallback: function(dc, errorCallback) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			var cb = function(evt) {
				if(dataChannel.rtcUserDeleted) return;
				var userPointer = dataChannel.rtcUserPointer || 0;
				var pError = WEBRTC.allocUTF8FromString(evt.message);
				Module['dynCall_vii'](errorCallback, pError, userPointer);
				_free(pError);
			};
			dataChannel.onerror = cb;
		},

		rtcSetMessageCallback: function(dc, messageCallback) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			dataChannel.onmessage = function(evt) {
				if(dataChannel.rtcUserDeleted) return;
				var userPointer = dataChannel.rtcUserPointer || 0;
				if(typeof evt.data == 'string') {
					var strLen = lengthBytesUTF8(evt.data);
					var pBuffer = _malloc(strLen + 1);
					stringToUTF8(evt.data, pBuffer, strLen + 1);
					Module['dynCall_viii'](messageCallback, pBuffer, -1, userPointer);
					_free(pBuffer);
				} else {
					var byteArray = new Uint8Array(evt.data);
					var size = byteArray.length;
					var pBuffer = _malloc(size);
					var heapBytes = new Uint8Array(Module.HEAPU8.buffer, pBuffer, size);
					heapBytes.set(byteArray);
					Module['dynCall_viii'](messageCallback, pBuffer, size, userPointer);
					_free(pBuffer);
				}
			};
			dataChannel.onclose = function() {
				if(dataChannel.rtcUserDeleted) return;
				var userPointer = dataChannel.rtcUserPointer || 0;
				Module['dynCall_viii'](messageCallback, 0, 0, userPointer);
			};
		},

		rtcSendMessage: function(dc, pBuffer, size) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			if(dataChannel.readyState != 'open') return 0;
			if(size >= 0) {
				var heapBytes = new Uint8Array(Module.HEAPU8.buffer, pBuffer, size);
				dataChannel.send(heapBytes.buffer);
				return size;
			} else {
				var str = UTF8ToString(pBuffer);
				dataChannel.send(str);
				return lengthBytesUTF8(str);
			}
		},

		rtcSetUserPointer: function(i, ptr) {
			if(WEBRTC.peerConnectionsMap[i]) WEBRTC.peerConnectionsMap[i].rtcUserPointer = ptr;
			if(WEBRTC.dataChannelsMap[i]) WEBRTC.dataChannelsMap[i].rtcUserPointer = ptr;
		},
	};

	autoAddDeps(WebRTC, '$WEBRTC');
	mergeInto(LibraryManager.library, WebRTC);
})();
