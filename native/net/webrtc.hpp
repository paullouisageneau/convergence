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

#ifndef NET_WEBRTC_H
#define NET_WEBRTC_H

#include "net/channel.hpp"

#include <rtcdcpp/PeerConnection.hpp>
#include <rtcdcpp/DataChannel.hpp>

#include <functional>
#include <vector>

namespace net
{

using std::function;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;

class DataChannel : public Channel
{
public:
	explicit DataChannel(shared_ptr<rtcdcpp::DataChannel> dataChannel);
	~DataChannel(void);

	void close(void);
	void send(const binary &data);

	bool isOpen(void) const;
	bool isClosed(void) const;
	
	string label(void) const;
	
private:
	shared_ptr<rtcdcpp::DataChannel> mDataChannel;
	
	static void OpenCallback(void *ptr);
	static void ErrorCallback(const char *error, void *ptr);
	static void MessageCallback(const char *data, int size, void *ptr);
};

class PeerConnection
{
public:
	struct SessionDescription {
		SessionDescription(const string &sdp, const string &type)
			: sdp(sdp), type(type) {}
		string sdp;
		string type;
	};
	
	struct IceCandidate {
		IceCandidate(const string &candidate, const string &sdpMid)
			: candidate(candidate), sdpMid(sdpMid) {}
		string candidate;
		string sdpMid;
	};
	
	explicit PeerConnection(const vector<string> &iceServers = vector<string>());
	~PeerConnection(void);
	
	shared_ptr<DataChannel> createDataChannel(const string &label);
	
	void setRemoteDescription(const SessionDescription &description);
	void setRemoteCandidate(const IceCandidate &candidate);
	
	void onDataChannel(function<void(shared_ptr<DataChannel>)> callback);
	void onLocalDescription(function<void(const SessionDescription&)> callback);
	void onLocalCandidate(function<void(const IceCandidate&)> callback);
	
private:
	void generateOffer(void);
	void generateAnswer(void);
	
	void triggerDataChannel(shared_ptr<DataChannel> channel);
	void triggerLocalDescription(const SessionDescription &description);
	void triggerLocalCandidate(const IceCandidate &candidate);
	
	function<void(shared_ptr<DataChannel>)> mDataChannelCallback;
	function<void(const SessionDescription&)> mLocalDescriptionCallback;
	function<void(const IceCandidate&)> mLocalCandidateCallback;
	
	unique_ptr<rtcdcpp::PeerConnection> mPeerConnection;
	bool mInitiated;
};

}

#endif // NET_WEBRTC_H
