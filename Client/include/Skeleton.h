#pragma once
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "Room.h"
#include "PingPongFbo.h"
#include "SpringCam.h"

#define USE_KINECT1 1

// kinect headers
#if USE_KINECT1
	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
	#include <windows.h>
	#include <Shlobj.h>
	#include "NuiApi.h"
#else
	#include "Kinect.h"
#endif

class Skeleton
{
	unsigned int mIndex;
	std::map< JointType, ci::Vec3f > mJointPositions;
	ci::Vec3f	mBodyCoordScale, mBodyCoordOffset;
	HandState mLeftHandState, mRightHandState;
	float	mSmoothAmount;
	bool	mSmoothOnlyHands;
	float	mScale;
	float	mHeadScale;
	ci::ColorAf	mColor;

	static int					mSkelicleSpawnIdx;
	static PingPongFbo			mSkelicleFbo;
    static ci::gl::VboMesh		mSkelicleVbo;

	void	drawBone( JointType jointType1, JointType jointType2 );

	static void	setupSkelicleFbo();
	static void	setupSkelicleVbo();
	void	addSkelicles(JointType jointType1, JointType jointType2, unsigned int count, float offsetAmount = 1.0f);

  public:
	Skeleton() {}
	Skeleton(unsigned int index);

	static void	setupSkelicles();
	static void	updateSkelicles(Room& room, int skeletonCount, int farSkeletonCount, Skeleton (&skeletons)[3], Skeleton (&farSkeletons)[3]);
	static void	drawSkelicles(Room& room, SpringCam& camera);
	void addSkelicles();

	ci::Vec3f	getJointPos( JointType jointType );
	HandState	getLeftHandState() { return mLeftHandState; }
	HandState	getRightHandState() { return mRightHandState; }

#if USE_KINECT1
	void	update( NUI_SKELETON_DATA skeletonData );
#else
	void	update( IBody* pBody );
#endif
	
	void	update( const ci::Vec3f farJoints[JointType_Count] );
	void	draw();
};