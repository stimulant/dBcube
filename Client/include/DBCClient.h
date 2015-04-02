#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Font.h"
#include "cinder/Rand.h"

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Json.h"

#include "OSCManager.h"
#include "PingPongFbo.h"

#include "CubeMap.h"
#include "Room.h"
#include "Emitter.h"
#include "Skeleton.h"
#include "SpringCam.h"

#define DBC_SIDE_COUNT 3
#define DBC_BODY_COUNT 3

class DBCClient : public ci::app::AppBasic 
{
	static DBCClient*			sInstance;

	// Kinect
#if USE_KINECT1
    INuiSensor*					mNuiSensor;
	HANDLE						mSkeletonStreamHandle;
    HANDLE						mNextSkeletonEvent;
#else
	IKinectSensor*				mKinectSensor;
    ICoordinateMapper*			mCoordinateMapper;
    IBodyFrameReader*			mBodyFrameReader;
	WAITABLE_HANDLE				mKinectFrameEvent;
#endif

	// Room
	Room						mRoom;
	gl::TextureRef				mRoomTexture;
	std::string					mRoomTextureName;

	// Overlay
	gl::TextureRef				mOverlayTexture;
	std::string					mOverlayTextureName;

	// Emitters
	bool						mMouseDown;
	Vec3f						mMousePos;
	Vec2f						mMouseDownPos, mMouseOffset;
	Vec3f						mMouseVel;

	// skeltons
	Skeleton					mSkeletons[DBC_BODY_COUNT];
	unsigned int				mSkeletonCount;
	float						mHandDist[DBC_BODY_COUNT*2];
	float						mLastHandDist[DBC_BODY_COUNT*2];
	bool						mAddedSkelicles;
	bool						mCenterAttract;
	
	// far skeletons
	Skeleton					mFarSkeletons[DBC_BODY_COUNT];
	unsigned int				mFarSkeletonCount;
	Vec3f						mFarJoints[DBC_BODY_COUNT][JointType_Count];
	
	// enitters
	Emitter						mMouseEmitter, mFarMouseEmitter;
	Emitter						mLeftEmitter[DBC_BODY_COUNT];
	Emitter						mRightEmitter[DBC_BODY_COUNT];
	Emitter						mFarLeftEmitter[DBC_BODY_COUNT];
	Emitter						mFarRightEmitter[DBC_BODY_COUNT];

	// Camera
	SpringCam					mSpringCam;

	// Rendering
	CubeMap						mCubeMap;

	// Params
	JsonTree					mJSParams;
	
	OSCManager					mOSCManager;
	float						mAudioLevel;

	void						setupKinect();
	void						mouseDown( MouseEvent event );
	void						mouseUp( MouseEvent event );
	void						mouseDrag( MouseEvent event );
	void						mouseMove( MouseEvent event );
	void						keyDown( KeyEvent event );
	void						updateKinect();
	void						updateBodies();

	Matrix44f					mModelView;
	Matrix44f					mProjection;
	Vec3f						screenToWorld(const Vec2i & point);
	Vec3f						unproject(const Vec3f & point);
	Area						mViewport;
	Rectf						mWindowSize;
	ColorAf						mTopColor;

public:
	static DBCClient*			get();

								DBCClient();
	void						draw();
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						setup();
	void						setupEmitters();
	void						update();
	void						onOSCMessage(osc::Message message);
	bool						getCenterAttract() const { return mCenterAttract; }

	float						getAudioLevel() { return mAudioLevel; }

	bool						getBoolParam(std::string paramName);
	float						getFloatParam(std::string paramName);
	std::string					getStringParam(std::string paramName);
	ColorA						getColorParam(std::string paramName);
};