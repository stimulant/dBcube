#pragma once
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "Kinect.h"
#include "Room.h"
#include "PingPongFbo.h"
#include "SpringCam.h"

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
	void	update( IBody* pBody );
	void	update( const ci::Vec3f farJoints[JointType_Count] );
	void	draw();
};