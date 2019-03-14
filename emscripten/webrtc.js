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
							return WEBRTC.handleSessionDescription(peerConnection, offer);
						})
						.catch(function(err) {
							console.error(err);
						});
				};
				peerConnection.onicecandidate = function(evt) {
					WEBRTC.handleIceCandidate(peerConnection, evt.candidate);
				};
				return pc;
			},

			registerDataChannel: function(dataChannel) {
				var dc = WEBRTC.nextId++;
				WEBRTC.dataChannelsMap[dc] = dataChannel;
				dataChannel.binaryType = 'arraybuffer';
				return dc;
			},

			handleSessionDescription: function(peerConnection, description) {
				return peerConnection.setLocalDescription(description)
					.then(function() {
						if(peerConnection.rtcUserDeleted) return;
						if(!peerConnection.rtcDescriptionCallback) return;
						var desc = peerConnection.localDescription;
						var pSdp = WEBRTC.allocUTF8FromString(desc.sdp);
						var pType = WEBRTC.allocUTF8FromString(desc.type);
						var callback =  peerConnection.rtcDescriptionCallback;
						var userPointer = peerConnection.rtcUserPointer || 0;
						Module.dyncall_viii(callback, pSdp, pType, userPointer);
						_free(pSdp);
						_free(pType);
					});
			},

			handleIceCandidate: function(peerConnection, candidate) {
				if(peerConnection.rtcUserDeleted) return;
				if(!peerConnection.rtcCandidateCallback) return;
				var pCandidate = WEBRTC.allocUTF8FromString(candidate ? candidate.candidate : "");
				var pSdpMid = WEBRTC.allocUTF8FromString(candidate ? candidate.sdpMid : "");
				var candidateCallback =  peerConnection.rtcCandidateCallback;
				var userPointer = peerConnection.rtcUserPointer || 0;
				Module.dynCall_viii(candidateCallback, pCandidate, pSdpMid, userPointer);
				_free(pCandidate);
				_free(pSdpMid);
			},
		},

		rtcCreatePeerConnection: function(pIceServers) {
			if(!window.RTCPeerConnection) return 0;
			var iceServers = [];
			if(pIceServers) {
				var i = 0;
				while(true) {
					var p = Module.HEAPU32[pIceServers/Module.HEAPU32.BYTES_PER_ELEMENT + i];
					if(!p) break;
					iceServers.push({
						urls: [UTF8ToString(p)],
					});
					++i;
				}
			}
			var config = {
				iceServers,
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
				Module.dynCall_vii(dataChannelCallback, dc, userPointer);
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
								return WEBRTC.handleSessionDescription(peerConnection, answer);
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

		rtcSetRemoteCandidate: function(pc, pCandidate, pSdpMid) {
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
				Module.dynCall_vi(openCallback, userPointer);
			};
			// dataChannel.bufferedAmountLowThreshold = 0;
			// dataChannel.onbufferedamountlow = cb;
			dataChannel.onopen = cb;
			if(dataChannel.readyState == 'open') setTimeout(cb, 0);
		},

		rtcSetErrorCallback: function(dc, openCallback) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			var cb = function(evt) {
				if(dataChannel.rtcUserDeleted) return;
				var userPointer = dataChannel.rtcUserPointer || 0;
				var pError = WEBRTC.allocUTF8FromString(evt.message);
				Module.dynCall_vii(messageCallback, pError, userPointer);
				_free(pError);
			};
			dataChannel.onerror = cb;
		},

		rtcSetMessageCallback: function(dc, messageCallback) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			dataChannel.onmessage = function(evt) {
				if(dataChannel.rtcUserDeleted) return;
				var byteArray = new Uint8Array(evt.data);
				var size = bufferArray.length;
				if(!size) return;
				var pBuffer = _malloc(size);
				var heapBytes = new Uint8Array(Module.HEAPU8.buffer, pBuffer, size);
				heapBytes.set(byteArray);
				var userPointer = dataChannel.rtcUserPointer || 0;
				Module.dynCall_viii(messageCallback, pBuffer, size, userPointer);
				_free(pBuffer);
			};
			dataChannel.onclose = function(evt) {
				if(dataChannel.rtcUserDeleted) return;
				var userPointer = dataChannel.rtcUserPointer || 0;
				Module.dynCall_viii(messageCallback, 0, 0, userPointer);
			};
		},

		rtcSendMessage: function(dc, pBuffer, size) {
			var dataChannel = WEBRTC.dataChannelsMap[dc];
			if(dataChannel.readyState != 'open') return 0;
			var heapBytes = new Uint8Array(Module.HEAPU8.buffer, pBuffer, size);
			dataChannel.send(heapBytes.buffer);
			return size;
		},

		rtcSetUserPointer: function(i, ptr) {
			if(WEBRTC.peerConnectionsMap[i]) WEBRTC.peerConnectionsMap[i].rtcUserPointer = ptr;
			if(WEBRTC.dataChannelsMap[i]) WEBRTC.dataChannelsMap[i].rtcUserPointer = ptr;
		},
	};

	autoAddDeps(WebRTC, '$WEBRTC');
	mergeInto(LibraryManager.library, WebRTC);
})();
