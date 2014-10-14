#include "DBCClient.h"
#include "AssetManager.h"
#include "ShaderManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
    if (pInterfaceToRelease != NULL)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = NULL;
    }
}

#define APP_WIDTH		768
#define APP_HEIGHT		768
#define ROOM_FBO_RES	2
#define NUM_PARTICLES_TO_SPAWN 50

// singleton instance
DBCClient* DBCClient::sInstance = NULL;
DBCClient* DBCClient::get()
{
	return sInstance;
}

DBCClient::DBCClient()
{
	sInstance = this;
}

void DBCClient::prepareSettings( Settings* settings )
{
	settings->prepareWindow( Window::Format().size( APP_WIDTH, APP_HEIGHT ).title( "Decibel Cube" ) );
	settings->setFrameRate( 60.0f );
}

void DBCClient::setup()
{	
	gl::enable( GL_TEXTURE_2D );
	mMouseDown = false;
	mAddedSkelicles = false;
	mMouseDownPos = Vec2f::zero();
	mMouseOffset = Vec2f::zero();
	setFullScreen( true );

	HRESULT hr = GetDefaultKinectSensor(&mKinectSensor);
    if (SUCCEEDED(hr) && mKinectSensor)
    {
        IBodyFrameSource* pBodyFrameSource = NULL;
        hr = mKinectSensor->Open();
        if (SUCCEEDED(hr))
            hr = mKinectSensor->get_CoordinateMapper(&mCoordinateMapper);
        if (SUCCEEDED(hr))
            hr = mKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
        if (SUCCEEDED(hr))
            hr = pBodyFrameSource->OpenReader(&mBodyFrameReader);
		if (SUCCEEDED(hr))
			hr = mBodyFrameReader->SubscribeFrameArrived(&mKinectFrameEvent);
        SafeRelease(pBodyFrameSource);
    }

	Emitter::setupParticles();
	Skeleton::setupSkelicles();

	// load all shaders
	ShaderManager::load("room", "shaders/room.vert", "shaders/room.frag");
	ShaderManager::load("sphere", "shaders/sphere.vert", "shaders/sphere.frag");
	ShaderManager::load("ribbon", "shaders/ribbon.vert", "shaders/ribbon.frag");

	ShaderManager::load("particleUpdate", "shaders/passThrough.vert", "shaders/particlesUpdate.frag");
	ShaderManager::load("particleAdd", "shaders/passThrough.vert", "shaders/particlesAdd.frag");
	ShaderManager::load("particleDraw", "shaders/particlesDraw.vert", "shaders/particlesDraw.frag");

	ShaderManager::load("skelicleUpdate", "shaders/passThrough.vert", "shaders/skeliclesUpdate.frag");
	ShaderManager::load("skelicleAdd", "shaders/passThrough.vert", "shaders/skeliclesAdd.frag");
	ShaderManager::load("skelicleDraw", "shaders/skeliclesDraw.vert", "shaders/skeliclesDraw.frag");

	// textures
	mRoomTexture = gl::Texture::create( loadImage( loadAsset( "textures/room.jpg" ) ) );
	mRoomTextureName = "room.jpg";
	mOverlayTexture = gl::Texture::create( loadImage( loadAsset( "textures/overlay.png" ) ) );
	mOverlayTextureName = "overlay.jpg";
	
	// camera
	mSpringCam		= SpringCam( -getFloatParam("globalCameraDist"), APP_HEIGHT/APP_WIDTH );
	mSpringCam.setEye( Vec3f(0.0f, 0.0f, -getFloatParam("globalCameraDist") ) );

	// skeletons
	mCenterAttract = false;
	mSkeletonCount = 0;
	mFarSkeletonCount = 0;
	for (int i=0; i<DBC_BODY_COUNT; i++)
	{
		mSkeletons[i] = Skeleton(i);
		mFarSkeletons[i] = Skeleton(i+DBC_BODY_COUNT);
	}

	// room
	bool isPowerOn		= true;
	bool isGravityOn	= true;
	mRoom				= Room( Vec3f( 193.0f, 193.0f, 500.0f ), isPowerOn, isGravityOn );	
	mRoom.init();

	mOSCManager.setup(3333);
	mAudioLevel = 0.0f;
}

void DBCClient::keyDown( KeyEvent event )
{
	switch( event.getCode() ){
		case KeyEvent::KEY_UP:		mSpringCam.setCenter( Vec3f(0.0f, -1.0f, mRoom.getDims().z) ); mSpringCam.setEye( Vec3f(0.0, 1000.0f, mRoom.getDims().z) );		break;
		case KeyEvent::KEY_DOWN:	mSpringCam.setCenter( Vec3f(0.0f, 1.0f, mRoom.getDims().z) ); mSpringCam.setEye( Vec3f(0.0, -1000.0f, mRoom.getDims().z) );		break;
		case KeyEvent::KEY_LEFT:	mSpringCam.setCenter( Vec3f(0.0f, 0.0f, mRoom.getDims().z) ); mSpringCam.setEye( Vec3f(-1000.0, 0.0f, mRoom.getDims().z) );		break;
		case KeyEvent::KEY_RIGHT:	mSpringCam.resetCenter(); mSpringCam.resetEye();								break;
		case KeyEvent::KEY_ESCAPE:  quit();												break;
		default: break;
	}
}

void DBCClient::mouseDown( MouseEvent event )
{    
	mMouseDown = true;
	mMouseDownPos = event.getPos();
	mMouseOffset = Vec2f::zero();
}

void DBCClient::mouseUp( MouseEvent event )
{    
	mMouseDown = false;
	mMouseOffset = Vec2f::zero();
}

void DBCClient::mouseDrag( MouseEvent event )
{
	mMousePos = screenToWorld(event.getPos() - Vec2i(0, APP_HEIGHT/2));
}

void DBCClient::mouseMove( MouseEvent event )
{
	mMousePos = screenToWorld(event.getPos() - Vec2i(0, APP_HEIGHT/2));
}

void DBCClient::setupEmitters()
{
	int side = mOSCManager.getClientIdx();
	int farSide = mOSCManager.getConnectedTo();

	mMouseEmitter = Emitter(side, 0, true);
	mFarMouseEmitter = Emitter(farSide, 0, false);
	for (int i=0; i<DBC_BODY_COUNT; i++)
	{
		mLeftEmitter[i] = Emitter(side, i);
		mRightEmitter[i] = Emitter(side, i);
		mFarLeftEmitter[i] = Emitter(farSide, i, false);
		mFarRightEmitter[i] = Emitter(farSide, i, false);
	}
}

void DBCClient::updateKinect()
{
	if (getBoolParam("disableUsers"))
	{
		mSkeletonCount = 0;
		return;
	}
	DWORD dwResult = WaitForSingleObjectEx(reinterpret_cast<HANDLE>(mKinectFrameEvent), 0, FALSE);
    if (WAIT_OBJECT_0 != dwResult)
	{
		return;
	}

	if (!mBodyFrameReader)
	{
        return;
	}

    IBodyFrame* pBodyFrame = NULL;
    HRESULT hr = mBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        hr = pBodyFrame->get_RelativeTime(&nTime);
        IBody* ppBodies[BODY_COUNT] = {0};

        if (SUCCEEDED(hr))
            hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		
        if (SUCCEEDED(hr))
		{
			mSkeletonCount = 0;
			for (int i = 0; i < BODY_COUNT && mSkeletonCount < DBC_BODY_COUNT; ++i)
            {
                IBody* pBody = ppBodies[i];
				if (pBody)
                {
					BOOLEAN bTracked = false;
                    hr = pBody->get_IsTracked(&bTracked);
					
                    if (SUCCEEDED(hr) && bTracked)
                    {
						mSkeletons[mSkeletonCount].update(pBody);
						mSkeletonCount++;
					}
				}
			}
		}
		
        for (int i = 0; i < _countof(ppBodies); ++i)
            SafeRelease(ppBodies[i]);
    }
	
    SafeRelease(pBodyFrame);
}


void DBCClient::updateBodies()
{
	// update far skeletons
	for ( unsigned int i=0; i < mFarSkeletonCount; i++ )
	{
		if (i >= DBC_BODY_COUNT) break;
		mFarSkeletons[i].update(mFarJoints[i]);
	}

	// update emitters
	for ( unsigned int b = 0; b < mFarSkeletonCount; b++ ) 
	{
		if (b >= DBC_BODY_COUNT) break;
		mFarLeftEmitter[b].setPos(mFarJoints[b][JointType_HandLeft]);
		mFarRightEmitter[b].setPos(mFarJoints[b][JointType_HandRight]);
	}
	for ( unsigned int b = 0; b < mSkeletonCount; b++ ) 
	{
		if (b < mFarSkeletonCount)
		{
			mLeftEmitter[b].connect(&(mFarLeftEmitter[b]));
			mRightEmitter[b].connect(&(mFarRightEmitter[b]));
		}
		else
		{
			mLeftEmitter[b].disconnect();
			mRightEmitter[b].disconnect();
		}
	}

	bool particleAlwaysEmit = getBoolParam("particleAlwaysEmit");
	float clapDistance = getFloatParam("skeletonClapDist");
	std::vector<Emitter*> allEmitters;

	// update emitters
	for ( unsigned int i=0; i < mSkeletonCount; i++ )
	{
		mLeftEmitter[i].setPos( mSkeletons[i].getJointPos( JointType_HandLeft ) );
		mRightEmitter[i].setPos( mSkeletons[i].getJointPos( JointType_HandRight ) );
		allEmitters.push_back(&mLeftEmitter[i]);
		allEmitters.push_back(&mRightEmitter[i]);
	}
	
	// detect claps
	bool clapDetected = false;
	for ( unsigned int i=0; !clapDetected && i < allEmitters.size(); i++ )
	{
		mLastHandDist[i] = mHandDist[i];
		mHandDist[i] = 99999.0f;
		Vec3f emitter1Pos = allEmitters[i]->getPos();

		// test each emitter against other emitters
		for ( unsigned int j=0; !clapDetected && j < allEmitters.size(); j++ )
		{
			if (i != j)
			{
				Vec3f emitter2Pos = allEmitters[j]->getPos();
				mHandDist[i] = min(emitter1Pos.distance( emitter2Pos ), mHandDist[i]);
				if (mHandDist[i] <= clapDistance && mLastHandDist[i] > clapDistance)
				{
					allEmitters[i]->addParticles( NUM_PARTICLES_TO_SPAWN );
					allEmitters[j]->addParticles( NUM_PARTICLES_TO_SPAWN );
					clapDetected = true;
				}
			}
		}
	}

	// send body update
	osc::Message message;
	message.setAddress("bodyupdate");
	message.addIntArg(mOSCManager.getClientIdx());
	if (getBoolParam("disableUsers"))
	{
		message.addIntArg(0);
	}
	else
	{
		message.addIntArg((int)mSkeletonCount);
		for ( unsigned int b=0; b < mSkeletonCount; b++ )
		{
			if (b >= DBC_BODY_COUNT) break;

			// send all joints
			for (int j=0; j < JointType_Count; j++)
			{
				message.addFloatArg( mSkeletons[b].getJointPos( (JointType)j ).x );
				message.addFloatArg( mSkeletons[b].getJointPos( (JointType)j ).y );
				message.addFloatArg( mSkeletons[b].getJointPos( (JointType)j ).z );
			}
		}
	}
	mOSCManager.sendMessage(message);

	// send mouse update
	if (mMouseDown)
	{
		mMouseEmitter.setPos( mMousePos );
		mMouseEmitter.addParticles( NUM_PARTICLES_TO_SPAWN );

		// send mouse update
		osc::Message message;
		message.setAddress("mouseupdate");
		message.addIntArg(mOSCManager.getClientIdx());
		message.addFloatArg(mMousePos.x);
		message.addFloatArg(mMousePos.y);
		message.addFloatArg(mMousePos.z);
		mOSCManager.sendMessage(message);
	}
	for (unsigned int i=0; i<mSkeletonCount; i++)
	{
		mLeftEmitter[i].update( mRoom.getTimeDelta() );
		mRightEmitter[i].update( mRoom.getTimeDelta() );
	}
	for (unsigned int i=0; i<mFarSkeletonCount; i++)
	{
		mFarLeftEmitter[i].update( mRoom.getTimeDelta() );
		mFarRightEmitter[i].update( mRoom.getTimeDelta() );
	}
}

bool DBCClient::getBoolParam(std::string paramName)
{
	if (OSCManager::get()->hasOSCReceived() && mJSParams.hasChild(paramName))
		return mJSParams.getChild(paramName).getValue<bool>();
	else
		return false;
}

float DBCClient::getFloatParam(std::string paramName)
{
	if (OSCManager::get()->hasOSCReceived() && mJSParams.hasChild(paramName))
		return mJSParams.getChild(paramName).getValue<float>();
	else
		return 0.0f;
}

std::string	DBCClient::getStringParam(std::string paramName)
{
	if (OSCManager::get()->hasOSCReceived() && mJSParams.hasChild(paramName))
		return mJSParams.getChild(paramName).getValue();
	else
		return "";
}

ColorA DBCClient::getColorParam(std::string paramName)
{
	if (OSCManager::get()->hasOSCReceived() && mJSParams.hasChild(paramName) && mJSParams.getChild(paramName).getValue().length() >= 6)
	{
		std::stringstream converter(mJSParams.getChild(paramName).getValue());
		unsigned int value;
		converter >> std::hex >> value;

		float a = 1.0f;
		if (mJSParams.getChild(paramName).getValue().length() == 8)
			a = ((value >> 24) & 0xFF) / 255.0f;
		float r = ((value >> 16) & 0xFF) / 255.0f;
		float g = ((value >> 8) & 0xFF) / 255.0f;
		float b = ((value) & 0xFF) / 255.0f;

		return ci::ColorA(r, g, b, a);
	}
	else
		return ColorA::black();
}

void DBCClient::update()
{
	if (mSpringCam.getCamDist() != -getFloatParam("globalCameraDist"))
	{
		mSpringCam		= SpringCam( -getFloatParam("globalCameraDist"), APP_HEIGHT/APP_WIDTH );
		mSpringCam.setEye( Vec3f(0.0f, 0.0f, -getFloatParam("globalCameraDist") ) );
	}
	// update osc
	mOSCManager.update();

	if (mOSCManager.getIsTop())
	{
	}
	else
	{
		// Update view dimensions
		gl::setMatrices( mSpringCam.getCam() );
		mModelView = gl::getModelView();
		mProjection = gl::getProjection();
		mViewport = gl::getViewport();
	
		// update room
		mSpringCam.update( 0.5f );
		mRoom.setDims( Vec3f( getFloatParam("roomDimX"), getFloatParam("roomDimY"), getFloatParam("roomDimZ") ) );
		mRoom.update();

		// update bodies
		updateKinect();
		updateBodies();

		// update room textures and colors
		std::string roomTextureName = getStringParam("roomTextureSide");
		if (roomTextureName != "" && roomTextureName != mRoomTextureName)
		{
			bool success = true;
			gl::TextureRef newRoomTexture = mRoomTexture;
			try
			{
				newRoomTexture = gl::Texture::create( loadImage( loadAsset( "textures/" + roomTextureName ) ) );
			} 
			catch( Exception e ) 
			{
				success = false;
				app::console() << e.what() << endl;
			}
			if (success)
			{
				mRoomTextureName = roomTextureName;
				mRoomTexture = newRoomTexture;
			}
		}

		// update overlay texture
		std::string overlayTextureName = getStringParam("screenOverlay");
		if (overlayTextureName != "" && overlayTextureName != mOverlayTextureName)
		{
			bool success = true;
			gl::TextureRef newOverlayTexture = mOverlayTexture;
			try
			{
				newOverlayTexture = gl::Texture::create( loadImage( loadAsset( "textures/" + overlayTextureName ) ) );
			} 
			catch( Exception e ) 
			{
				success = false;
				app::console() << e.what() << endl;
			}
			if (success)
			{
				mOverlayTextureName = overlayTextureName;
				mOverlayTexture = newOverlayTexture;
			}
		}

		Emitter::updateParticles(mRoom);
		Skeleton::updateSkelicles(mRoom, mSkeletonCount, mFarSkeletonCount, mSkeletons, mFarSkeletons);
	}
}

void DBCClient::draw()
{
	// if we haven't created skelicles, lets do that now
	if (!mAddedSkelicles && mOSCManager.hasOSCReceived())
	{
		for (int i=0; i<DBC_BODY_COUNT; i++)
			mSkeletons[i].addSkelicles();
		for (int i=0; i<DBC_BODY_COUNT; i++)
			mFarSkeletons[i].addSkelicles();
		mAddedSkelicles = true;
	}

	Vec2f startPos = Vec2f(getWindowWidth()/2.0f - 768/2.0f, getWindowHeight()/2.0f - 768/2.0f);
	gl::setViewport( Area((int)startPos.x, (int)startPos.y, (int)startPos.x + 768, (int)startPos.y + 768) );
	gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 1.0f ), true );

	if (mOSCManager.getIsTop())
	{
		gl::color(mTopColor);
		gl::enableAlphaBlending();
		gl::drawSolidRect(Rectf(0, 0, 768 * 2.0, 768));
		gl::disableAlphaBlending();
	}
	else
	{
		// draw room
		if (getBoolParam("roomDraw"))
		{
			gl::setMatrices( mSpringCam.getCam() );
		
			gl::enable( GL_TEXTURE_2D );
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );

			gl::disableDepthRead();
			gl::disableDepthWrite();

			Matrix44f m;
			m.setToIdentity();
			m.translate( Vec3f(0.0f, 0.0f, mRoom.getDims().z) );
			m.scale( mRoom.getDims() );

			if (getBoolParam("globalDrawWireframe"))
				gl::enableWireframe();
	
			ShaderManager::get("room")->bind();
			mRoomTexture->bind();
			ShaderManager::get("room")->uniform( "roomMap", 0 );
			ShaderManager::get("room")->uniform( "mvpMatrix", mSpringCam.mMvpMatrix );
			ShaderManager::get("room")->uniform( "mMatrix", m );
			ShaderManager::get("room")->uniform( "eyePos", mSpringCam.mEye );
			ShaderManager::get("room")->uniform( "roomDims", mRoom.getDims() );
			ShaderManager::get("room")->uniform( "useRoomMap", getBoolParam("roomTexture") );
			ShaderManager::get("room")->uniform( "power", getFloatParam("roomPower") );
			ShaderManager::get("room")->uniform( "lightPower", getFloatParam("roomLightPower") );
			ShaderManager::get("room")->uniform( "topColor", getColorParam("topColor") );
			ShaderManager::get("room")->uniform( "audioLevel", (DBCClient::get()->getAudioLevel() * DBCClient::get()->getFloatParam("roomAudioMod")) + (1.0f - DBCClient::get()->getFloatParam("roomAudioMod") ) );
			ShaderManager::get("room")->uniform( "timePer", mRoom.getTimePer() * 1.5f + 0.5f );
			mRoom.draw();
			mRoomTexture->unbind();
			ShaderManager::get("room")->unbind();

			gl::disableWireframe();
			glDisable( GL_CULL_FACE );

			gl::enableDepthRead();
			gl::enableDepthWrite();
		}

		if (getBoolParam("globalDrawWireframe"))
			gl::enableWireframe();

		// draw emitters
		if (getBoolParam("ribbonDraw"))
		{
			gl::setMatrices( mSpringCam.getCam() );
			gl::enableAlphaBlending();
			gl::enableDepthRead();
			gl::enableDepthWrite();
			for (unsigned int i=0; i<mSkeletonCount; i++)
			{
				mLeftEmitter[i].drawRope(mRoom, mSpringCam);
				mRightEmitter[i].drawRope(mRoom, mSpringCam);
			}
			for (unsigned int i=0; i<mFarSkeletonCount; i++)
			{
				if (i > mSkeletonCount)
				{
					mFarLeftEmitter[i].drawRope(mRoom, mSpringCam);
					mFarRightEmitter[i].drawRope(mRoom, mSpringCam);
				}
			}
		}

		// draw sphere
		if (true)
		{
			gl::setMatrices( mSpringCam.getCam() );
			gl::disable( GL_TEXTURE_2D );
			gl::enableAlphaBlending();
			gl::enableDepthRead();
			gl::enableDepthWrite();

			ShaderManager::get("sphere")->bind();
			ShaderManager::get("sphere")->uniform( "audioLevel", mAudioLevel );
			ShaderManager::get("sphere")->uniform( "mvpMatrix", mSpringCam.mMvpMatrix );
			ShaderManager::get("sphere")->uniform( "eyePos", mSpringCam.getEye() );
			ShaderManager::get("sphere")->uniform( "roomDims", mRoom.getDims() );
			ShaderManager::get("sphere")->uniform( "alpha", getFloatParam("skeletonAlpha") );

			// draw near skeletons
			for ( unsigned int i=0; i < mSkeletonCount; i++ )
			{
				if (i >= DBC_BODY_COUNT) break;
				ShaderManager::get("sphere")->uniform( "color", getColorParam( "emitterColorSide" + std::to_string(mOSCManager.getClientIdx()+1) ) );
				mSkeletons[i].draw();
			}

			// draw far skeletons
			for ( unsigned int i=0; i < mFarSkeletonCount; i++ )
			{
				if (i >= DBC_BODY_COUNT) break;
				ShaderManager::get("sphere")->uniform( "color", getColorParam( "emitterColorSide" + std::to_string(mOSCManager.getConnectedTo()+1) ) );
				mFarSkeletons[i].draw();
			}

			ShaderManager::get("sphere")->unbind();
		}

		// draw particles
		if (getBoolParam("particleDraw"))
		{
			gl::setMatrices( mSpringCam.getCam() );
			Emitter::drawParticles(mRoom, mSpringCam);
			Skeleton::drawSkelicles(mRoom, mSpringCam);
		}

		// draw tag
		{
			gl::enable( GL_TEXTURE_2D );
			gl::enableAlphaBlending();
			gl::disableDepthRead();
			gl::disableDepthWrite();

			gl::setMatricesWindow(Vec2i(768,768), true); 
			gl::color(ColorA::white());
			gl::draw(mOverlayTexture, Vec2f(0.0f, 0.0f));
		}
	
		gl::disableWireframe();
	}
}

ColorAf colorInterpolate(ColorAf color1, ColorAf color2, float factor)
{
	Vec3f hsv1 = rgbToHSV(color1);
	Vec3f hsv2 = rgbToHSV(color2);
	Vec3f finalHSV = Vec3f( lerp(hsv1.x, hsv2.x, factor),
							lerp(hsv1.y, hsv2.y, factor),
							lerp(hsv1.z, hsv2.z, factor) );
	return hsvToRGB(finalHSV);
}

void DBCClient::onOSCMessage(osc::Message message)
{
	if (message.getAddress() == "centerAttract")
	{
		mCenterAttract = (message.getArgAsInt32( 0, true ) == 1);
	}
	if (message.getAddress() == "updateparams")
	{
		std::string	paramsStr	= message.getArgAsString( 0, true );
		mJSParams = JsonTree( paramsStr );
	}
	if (message.getAddress() == "mouseupdate")
	{
		// emit particles for mouse
		int clientIdx = message.getArgAsInt32( 0, true );
		Vec3f mousePos = Vec3f(message.getArgAsFloat( 1, true ),
								message.getArgAsFloat( 2, true ),
								message.getArgAsFloat( 3, true ));

		// emit particles for far mouse
		mousePos.z = mRoom.getDims().z * 2.0f;
		mFarMouseEmitter.setPos( mousePos );
		mFarMouseEmitter.addParticles( NUM_PARTICLES_TO_SPAWN );
	}
	if (message.getAddress() == "bodyupdate")
	{
		// emit particles for mouse
		int clientIdx = message.getArgAsInt32( 0, true );
		mFarSkeletonCount = message.getArgAsInt32( 1, true );

		for (unsigned int b=0; b < mFarSkeletonCount; b++)
		{
			for (int j=0; j < JointType_Count; j++)
			{
				mFarJoints[b][j] = Vec3f(message.getArgAsFloat( b * JointType_Count * 3 + j * 3 + 2, true ),
										 message.getArgAsFloat( b * JointType_Count * 3 + j * 3 + 3, true ),
										 message.getArgAsFloat( b * JointType_Count * 3 + j * 3 + 4, true ));
				mFarJoints[b][j].z = mRoom.getDims().z * 2.0f;
			}
		}
	}
	if (message.getAddress() == "audioUpdate")
	{
		float audioSmoothing = getFloatParam("audioSmoothing");
		float newAudioLevel = getFloatParam("audioMod") * message.getArgAsFloat(0, true) + (1.0f - getFloatParam("audioMod"));
		mAudioLevel = newAudioLevel * (1.0f - audioSmoothing) + mAudioLevel * audioSmoothing;
	}
	if (message.getAddress() == "topUpdate")
	{
		int lastClientIndex = message.getArgAsInt32( 0, true );
		float lastClientUpdateElapsed = message.getArgAsFloat( 1, true );
		if (lastClientIndex > 0)
			mTopColor = colorInterpolate(
				getColorParam( "emitterColorSide" + std::to_string(lastClientIndex+1) + "User1" ), 
				ColorA::white(), min(lastClientUpdateElapsed, 1.0f));
		else
			mTopColor = ColorAf::white();
		mTopColor.a = mAudioLevel/2.0f;
	}
}

// Convert mouse position to 3D
Vec3f DBCClient::screenToWorld(const Vec2i & point)
{
	// Find near and far plane intersections
	Vec3f point3f = Vec3f((float)point.x, mWindowSize.getHeight() * 0.5f - (float)point.y, 0.0f);
	Vec3f nearPlane = unproject(point3f);
	Vec3f farPlane = unproject(Vec3f(point3f.x, point3f.y, 1.0f));

	// Calculate X, Y and return point
	float theta = (0.0f - nearPlane.z) / (farPlane.z - nearPlane.z);
	return Vec3f(
		nearPlane.x + theta * (farPlane.x - nearPlane.x), 
		nearPlane.y + theta * (farPlane.y - nearPlane.y), 
		0.0f
		);
}

// Unproject a coordinate back to to camera
Vec3f DBCClient::unproject(const Vec3f & point)
{
	// Find the inverse Modelview-Projection-Matrix
	Matrix44f mInvMVP = mProjection * mModelView;
	mInvMVP.invert();

	// Transform to normalized coordinates in the range [-1, 1]
	Vec4f pointNormal;
	pointNormal.x = (point.x - mViewport.getX1()) / mViewport.getWidth() * 2.0f - 1.0f;
	pointNormal.y = (point.y - mViewport.getY1()) / mViewport.getHeight() * 2.0f;
	pointNormal.z = 2.0f * point.z - 1.0f;
	pointNormal.w = 1.0f;

	// Find the object's coordinates
	Vec4f pointCoord = mInvMVP * pointNormal;
	if (pointCoord.w != 0.0f)
		pointCoord.w = 1.0f / pointCoord.w;

	// Return coordinate
	return Vec3f(
		pointCoord.x * pointCoord.w, 
		pointCoord.y * pointCoord.w, 
		pointCoord.z * pointCoord.w
		);
}


CINDER_APP_BASIC( DBCClient, RendererGl )
	