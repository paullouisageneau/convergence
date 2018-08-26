
#include "net/webrtc.hpp"

#include <rtcdcpp/Logging.hpp>

#include <iostream>
#include <exception>

namespace net
{

PeerConnection::PeerConnection(const vector<string> &iceServers) : mInitiated(false)
{
#ifndef SPDLOG_DISABLED
  	auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
  	spdlog::create("rtcdcpp.PeerConnection", console_sink);
  	spdlog::create("rtcdcpp.SCTP", console_sink);
  	spdlog::create("rtcdcpp.Nice", console_sink);
  	spdlog::create("rtcdcpp.DTLS", console_sink);
  	spdlog::set_level(spdlog::level::debug);
#endif

	rtcdcpp::RTCConfiguration config;
	for(string host : iceServers)
	{
		uint16_t port = 3478;
		size_t sep = host.find_first_of(':');
		if(sep != string::npos)
		{
			host.resize(sep);
			sep = host.find_last_of(':');
			if(sep != string::npos)
			{
				port = uint16_t(std::stoul(host.substr(sep + 1)));
				host.resize(sep);
			}
		}
		config.ice_servers.emplace_back(rtcdcpp::RTCIceServer{host, port});
	}
	
	std::function<void(rtcdcpp::PeerConnection::IceCandidate)> onLocalIceCandidate = [this](rtcdcpp::PeerConnection::IceCandidate candidate) {
		triggerLocalCandidate(IceCandidate(candidate.candidate, candidate.sdpMid));
	};
	
	std::function<void(shared_ptr<rtcdcpp::DataChannel> channel)> onDataChannel = [this](shared_ptr<rtcdcpp::DataChannel> channel) {
		triggerDataChannel(std::make_shared<DataChannel>(channel));
	};
	
	mPeerConnection = std::make_unique<rtcdcpp::PeerConnection>(config, onLocalIceCandidate, onDataChannel);
}

PeerConnection::~PeerConnection(void)
{

}

shared_ptr<DataChannel> PeerConnection::createDataChannel(const string &label)
{
	auto dataChannel = std::make_shared<DataChannel>(mPeerConnection->CreateDataChannel(label));
	if(!mInitiated) generateOffer();
	return dataChannel;
}

void PeerConnection::setRemoteDescription(const SessionDescription &description)
{
	mPeerConnection->ParseOffer(description.sdp);
	if(description.type == "offer") generateAnswer();
	mPeerConnection->GatherCandidates();
}

void PeerConnection::setRemoteCandidate(const IceCandidate &candidate)
{
	mPeerConnection->SetRemoteIceCandidate(candidate.candidate);
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

void PeerConnection::generateOffer(void)
{
	string offer = mPeerConnection->GenerateOffer();
	mInitiated = true;
	triggerLocalDescription(SessionDescription(offer, "offer"));
}

void PeerConnection::generateAnswer(void)
{
	string answer = mPeerConnection->GenerateAnswer();
	mInitiated = true;
	triggerLocalDescription(SessionDescription(answer, "answer"));
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

DataChannel::DataChannel(shared_ptr<rtcdcpp::DataChannel> dataChannel) :
	mDataChannel(dataChannel)
{
	mDataChannel->SetOnOpen([this]() {
		triggerOpen();
	});
	
	mDataChannel->SetOnStringMsgCallback([this](string str) {
		binary tmp(str.begin(), str.end());
		triggerMessage(tmp);
	});

	mDataChannel->SetOnBinaryMsgCallback([this](rtcdcpp::ChunkPtr chunk) {
		binary tmp(chunk->Data(), chunk->Data() + chunk->Size());
		triggerMessage(tmp);
	});
	
	mDataChannel->SetOnClosedCallback([this]() {
		triggerClosed();
	});
	
	mDataChannel->SetOnErrorCallback([this](string description) {
		triggerError(description);
	});
}

DataChannel::~DataChannel(void) 
{

}

void DataChannel::close(void)
{
	mDataChannel->Close();
}

void DataChannel::send(const binary &data)
{
	mDataChannel->SendBinary(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

bool DataChannel::isOpen(void) const
{
	// TODO
	return true;
}

bool DataChannel::isClosed(void) const
{
	// TODO
	return false;
}

std::string DataChannel::label(void) const
{
	return mDataChannel->GetLabel();
}

}
