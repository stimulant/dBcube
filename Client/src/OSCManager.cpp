//
//  OSCManager.h
//
//  Created by Joel Pryde on 05/14/13.

#include "cinder/app/AppBasic.h"
#include "DBCClient.h"
#include "OSCManager.h"
#include "cinder/Json.h"

using namespace ci;

// singleton instance
OSCManager* OSCManager::sInstance = NULL;
OSCManager* OSCManager::get()
{
	return sInstance;
}

OSCManager::OSCManager()
{
	mOSCReceived = false;
	sInstance = this;
	mClientCount = 1;
	mConnectedTo = 2;
	mClientIdx = 0;
	mIsTop;
}

void OSCManager::setup(int recvPort)
{
	// setup osc
	mRecvPort = recvPort;
	mListener.setup(recvPort);
}

void OSCManager::update()
{
	while (mListener.hasWaitingMessages()) 
	{
		osc::Message message;
		mListener.getNextMessage(&message);

		if (!mOSCReceived && message.getAddress() == "config")
		{
			mServerHost				= message.getArgAsString( 0, true );
			mServerPort				= message.getArgAsInt32( 1, true );
			mClientIdx				= message.getArgAsInt32( 2, true );
			mClientCount			= message.getArgAsInt32( 3, true );
			mIsTop					= (message.getArgAsInt32( 4, true ) == 1);
			mConnectedTo			= message.getArgAsInt32( 5, true );
			
			// setup sender
			mSender.setup( mServerHost, mServerPort );
			mOSCReceived = true;

			// tell the server we are connected
			osc::Message message;
			message.setAddress("connect");
			message.addIntArg(mClientIdx);
			sendMessage(message);
			DBCClient::get()->setupEmitters();
		}
		else if (mOSCReceived)
		{
			DBCClient::get()->onOSCMessage(message);
		}

		// do heartbeat message
		if (mOSCReceived)
		{
			osc::Message message;
			message.setAddress("heartbeat");
			message.addIntArg(mClientIdx);
			message.addFloatArg(DBCClient::get()->getAverageFps());
			sendMessage(message);
		}
	}
}

void OSCManager::sendMessage( osc::Message message )
{
	if (mOSCReceived)
		mSender.sendMessage(message);
}

