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

#include "net/webrtc.hpp"

#include <emscripten/emscripten.h>
#include <iostream>
#include <exception>

extern "C" {
	extern int  rtcCreatePeerConnection(const char **iceServers);
	extern void rtcDeletePeerConnection(int pc);
	extern int  rtcCreateDataChannel(int pc, const char *label);
	extern void rtcDeleteDataChannel(int dc);
	extern void rtcSetDataChannelCallback(int pc, void (*dataChannelCallback)(int, void*));
	extern void rtcSetLocalDescriptionCallback(int pc, void (*descriptionCallback)(const char*, const char*, void*));
	extern void rtcSetLocalCandidateCallback(int pc, void (*candidateCallback)(const char*, const char*, void*));
	extern void rtcSetRemoteDescription(int pc, const char *sdp, const char *type);
	extern void rtcSetRemoteCandidate(int pc, const char* candidate, const char *sdpMid);
	extern int  rtcGetDataChannelLabel(int dc, char *buffer, int size);
	extern void rtcSetOpenCallback(int dc, void (*openCallback)(void*));
	extern void rtcSetErrorCallback(int dc, void (*errorCallback)(const char*, void*));
	extern void rtcSetMessageCallback(int dc, void (*messageCallback)(const char *, int, void*));
	extern int  rtcSendMessage(int dc, const char *buffer, int size);
	extern void rtcSetUserPointer(int i, void *ptr);
}

namespace net
{

void PeerConnection::DataChannelCallback(int dc, void *ptr)
{
	PeerConnection *p = static_cast<PeerConnection*>(ptr);
	if(p) p->triggerDataChannel(std::make_shared<DataChannel>(dc));
}

void PeerConnection::DescriptionCallback(const char *sdp, const char *type, void *ptr)
{
	PeerConnection *p = static_cast<PeerConnection*>(ptr);
	if(p) p->triggerLocalDescription(PeerConnection::SessionDescription(sdp, type));
}

void PeerConnection::CandidateCallback(const char *candidate, const char *sdpMid, void *ptr)
{
	PeerConnection *p = static_cast<PeerConnection*>(ptr);
	if(p) p->triggerLocalCandidate(PeerConnection::IceCandidate(candidate, sdpMid));
}

void DataChannel::OpenCallback(void *ptr)
{
	DataChannel *d = static_cast<DataChannel*>(ptr);
	if(d)
	{
		d->mConnected = true;
		d->triggerOpen();
	}
}

void DataChannel::ErrorCallback(const char *error, void *ptr)
{
	DataChannel *d = static_cast<DataChannel*>(ptr);
	if(d) d->triggerError(string(error ? error : "unknown"));
}

void DataChannel::MessageCallback(const char *data, int size, void *ptr)
{
	DataChannel *d = static_cast<DataChannel*>(ptr);
	if(d)
	{
		if(data) d->triggerMessage(binary(data, data + size));
		else {
			d->close();
			d->triggerClosed();
		}
	}
}

PeerConnection::PeerConnection(const vector<string> &iceServers)
{
	vector<const char*> ptrs;
	ptrs.reserve(iceServers.size());
	for(const string &s : iceServers) ptrs.push_back(s.c_str());
	ptrs.push_back(nullptr);
	mId = rtcCreatePeerConnection(ptrs.data());
	if(!mId) throw std::runtime_error("WebRTC not supported");

	rtcSetUserPointer(mId, this);
	rtcSetDataChannelCallback(mId, DataChannelCallback);
	rtcSetLocalDescriptionCallback(mId, DescriptionCallback);
	rtcSetLocalCandidateCallback(mId, CandidateCallback);
}

PeerConnection::~PeerConnection(void)
{
	rtcDeletePeerConnection(mId);
}

shared_ptr<DataChannel> PeerConnection::createDataChannel(const string &label)
{
	return std::make_shared<DataChannel>(rtcCreateDataChannel(mId, label.c_str()));
}

void PeerConnection::setRemoteDescription(const SessionDescription &description)
{
	rtcSetRemoteDescription(mId, description.sdp.c_str(), description.type.c_str());
}

void PeerConnection::setRemoteCandidate(const IceCandidate &candidate)
{
	rtcSetRemoteCandidate(mId, candidate.candidate.c_str(), candidate.sdpMid.c_str());
}

void PeerConnection::onDataChannel(function<void(shared_ptr<DataChannel>)> callback)
{
	mDataChannelCallback = callback;
}

void PeerConnection::onLocalDescription(function<void(const SessionDescription&)> callback)
{
	mLocalDescriptionCallback = callback;
}

void PeerConnection::onLocalCandidate(function<void(const IceCandidate&)> callback)
{
	mLocalCandidateCallback = callback;
}

void PeerConnection::triggerDataChannel(shared_ptr<DataChannel> dataChannel)
{
	if(mDataChannelCallback) mDataChannelCallback(dataChannel);
}

void PeerConnection::triggerLocalDescription(const SessionDescription &description)
{
	if(mLocalDescriptionCallback) mLocalDescriptionCallback(description);
}

void PeerConnection::triggerLocalCandidate(const IceCandidate &candidate)
{
	if(mLocalCandidateCallback) mLocalCandidateCallback(candidate);
}

DataChannel::DataChannel(int id) :
	mId(id),
	mConnected(false)
{
	rtcSetUserPointer(mId, this);
	rtcSetOpenCallback(mId, OpenCallback);
	rtcSetErrorCallback(mId, ErrorCallback);
	rtcSetMessageCallback(mId, MessageCallback);

	char str[256];
	rtcGetDataChannelLabel(mId, str, 256);
	mLabel = str;
}

DataChannel::~DataChannel(void)
{
	close();
}

void DataChannel::close(void)
{
	mConnected = false;
	if(mId)
	{
		rtcDeleteDataChannel(mId);
		mId = 0;
	}
}

void DataChannel::send(const binary &data)
{
	if(!mId) return;
	rtcSendMessage(mId, data.data(), data.size());
}

bool DataChannel::isOpen(void) const
{
	return mConnected;
}

bool DataChannel::isClosed(void) const
{
	return mId == 0;
}

std::string DataChannel::label(void) const
{
	return mLabel;
}

}

