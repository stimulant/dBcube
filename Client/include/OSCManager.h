//
//  OSCManager.h
//
//  Created by Joel Pryde on 05/14/13.

#pragma once

#include "osc/OscSender.h"
#include "osc/OscListener.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OSCManager
{
	static OSCManager* sInstance;

public:
	static OSCManager* get();

	OSCManager();

	void setup(int recvPort);
    void update();
	void sendMessage( osc::Message message );
	bool hasOSCReceived() const { return mOSCReceived; }
	int getClientIdx() const { return mClientIdx; }
	int getClientCount() const { return mClientCount; }
	int getConnectedTo() const { return mConnectedTo; }
	bool getIsTop() const { return mIsTop; }

private:
	ci::osc::Sender		mSender;
	ci::osc::Listener	mListener;
	bool				mOSCReceived;
	int					mRecvPort;
	std::string			mServerHost;
	int					mServerPort;
	int					mClientIdx;
	int					mClientCount;
	int					mConnectedTo;
	bool				mIsTop;
};