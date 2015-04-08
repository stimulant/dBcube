#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "Skeleton.h"
#include "DBCClient.h"
#include "ShaderManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define SKELICLES_SIDE 64

int					Skeleton::mSkelicleSpawnIdx;
PingPongFbo			Skeleton::mSkelicleFbo;
gl::VboMesh			Skeleton::mSkelicleVbo;

Skeleton::Skeleton(unsigned int index)
{
	mIndex = index;
}

void Skeleton::addSkelicles()
{
	int sideIdx = OSCManager::get()->getClientIdx();
	int colorIdx = mIndex;
	if (colorIdx >= 3)
	{
		sideIdx = OSCManager::get()->getConnectedTo();
		colorIdx -= 3;
	}
	
	mColor = DBCClient::get()->getColorParam( "emitterColorSide" + std::to_string(sideIdx+1) );

	// add skelicles for all bones
	unsigned int shortBoneCount = 10;
	unsigned int medBoneCount = 20;
	unsigned int longBoneCount = 30;

#if USE_KINECT1

	// Render Torso
	addSkelicles( (unsigned int)NUI_SKELETON_POSITION_HEAD, (unsigned int)NUI_SKELETON_POSITION_SHOULDER_CENTER, shortBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_SHOULDER_CENTER, (unsigned int)NUI_SKELETON_POSITION_SHOULDER_LEFT, shortBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_SHOULDER_CENTER, (unsigned int)NUI_SKELETON_POSITION_SHOULDER_RIGHT, shortBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_SHOULDER_CENTER, (unsigned int)NUI_SKELETON_POSITION_SPINE, medBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_SPINE, (unsigned int)NUI_SKELETON_POSITION_HIP_CENTER, medBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_HIP_CENTER, (unsigned int)NUI_SKELETON_POSITION_HIP_LEFT, shortBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_HIP_CENTER, (unsigned int)NUI_SKELETON_POSITION_HIP_RIGHT, shortBoneCount );

    // Left Arm
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_SHOULDER_LEFT, (unsigned int)NUI_SKELETON_POSITION_ELBOW_LEFT, medBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_ELBOW_LEFT, (unsigned int)NUI_SKELETON_POSITION_WRIST_LEFT, medBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_WRIST_LEFT, (unsigned int)NUI_SKELETON_POSITION_HAND_LEFT, shortBoneCount );

    // Right Arm
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_SHOULDER_RIGHT, (unsigned int)NUI_SKELETON_POSITION_ELBOW_RIGHT, medBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_ELBOW_RIGHT, (unsigned int)NUI_SKELETON_POSITION_WRIST_RIGHT, medBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_WRIST_RIGHT, (unsigned int)NUI_SKELETON_POSITION_HAND_RIGHT, shortBoneCount );

    // Left Leg
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_HIP_LEFT, (unsigned int)NUI_SKELETON_POSITION_KNEE_LEFT, longBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_KNEE_LEFT, (unsigned int)NUI_SKELETON_POSITION_ANKLE_LEFT, longBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_ANKLE_LEFT, (unsigned int)NUI_SKELETON_POSITION_FOOT_LEFT, shortBoneCount );

    // Right Leg
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_HIP_RIGHT, (unsigned int)NUI_SKELETON_POSITION_KNEE_RIGHT, longBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_KNEE_RIGHT, (unsigned int)NUI_SKELETON_POSITION_ANKLE_RIGHT, longBoneCount );
    addSkelicles( (unsigned int)NUI_SKELETON_POSITION_ANKLE_RIGHT, (unsigned int)NUI_SKELETON_POSITION_FOOT_RIGHT, shortBoneCount );

#else

	// Torso
	addSkelicles( JointType_Head, JointType_Neck, shortBoneCount );
	addSkelicles( JointType_Neck, JointType_SpineShoulder, shortBoneCount );
	addSkelicles( JointType_SpineShoulder, JointType_SpineMid, medBoneCount, 2.0f );
	addSkelicles( JointType_SpineMid, JointType_SpineBase, medBoneCount, 2.0f );
	addSkelicles( JointType_SpineShoulder, JointType_ShoulderRight, medBoneCount );
	addSkelicles( JointType_SpineShoulder, JointType_ShoulderLeft, medBoneCount );
	addSkelicles( JointType_SpineBase, JointType_HipRight, shortBoneCount );
	addSkelicles( JointType_SpineBase, JointType_HipLeft, shortBoneCount );

	// Right Arm    
	addSkelicles( JointType_ShoulderRight, JointType_ElbowRight, longBoneCount );
	addSkelicles( JointType_ElbowRight, JointType_WristRight, longBoneCount );
	addSkelicles( JointType_WristRight, JointType_HandRight, shortBoneCount );
	addSkelicles( JointType_HandRight, JointType_HandTipRight, shortBoneCount );

	// Left Arm
	addSkelicles( JointType_ShoulderLeft, JointType_ElbowLeft, longBoneCount );
	addSkelicles( JointType_ElbowLeft, JointType_WristLeft, longBoneCount );
	addSkelicles( JointType_WristLeft, JointType_HandLeft, shortBoneCount );
	addSkelicles( JointType_HandLeft, JointType_HandTipLeft, shortBoneCount );

	// Right Leg
	addSkelicles( JointType_HipRight, JointType_KneeRight, longBoneCount );
	addSkelicles( JointType_KneeRight, JointType_AnkleRight, longBoneCount );
	addSkelicles( JointType_AnkleRight, JointType_FootRight, shortBoneCount );

	// Left Leg
	addSkelicles( JointType_HipLeft, JointType_KneeLeft, longBoneCount );
	addSkelicles( JointType_KneeLeft, JointType_AnkleLeft, longBoneCount );
	addSkelicles( JointType_AnkleLeft, JointType_FootLeft, shortBoneCount );	
#endif
}

/*
	NUI_SKELETON_POSITION_SPINE
	NUI_SKELETON_POSITION_SHOULDER_CENTER
	NUI_SKELETON_POSITION_HEAD
	NUI_SKELETON_POSITION_SHOULDER_LEFT
	NUI_SKELETON_POSITION_ELBOW_LEFT
	NUI_SKELETON_POSITION_WRIST_LEFT
	NUI_SKELETON_POSITION_HAND_LEFT
	NUI_SKELETON_POSITION_SHOULDER_RIGHT
	NUI_SKELETON_POSITION_ELBOW_RIGHT
	NUI_SKELETON_POSITION_WRIST_RIGHT
	NUI_SKELETON_POSITION_HAND_RIGHT
	NUI_SKELETON_POSITION_HIP_LEFT
	NUI_SKELETON_POSITION_KNEE_LEFT
	NUI_SKELETON_POSITION_ANKLE_LEFT
	NUI_SKELETON_POSITION_FOOT_LEFT
	NUI_SKELETON_POSITION_HIP_RIGHT
	NUI_SKELETON_POSITION_KNEE_RIGHT
	NUI_SKELETON_POSITION_ANKLE_RIGHT
	NUI_SKELETON_POSITION_FOOT_RIGHT
*/

#if USE_KINECT1

void Skeleton::update( NUI_SKELETON_DATA skeletonData )
{
	mBodyCoordScale = Vec3f( DBCClient::get()->getFloatParam("skeletonPosScaleX"), 
		DBCClient::get()->getFloatParam("skeletonPosScaleY"), 
		DBCClient::get()->getFloatParam("skeletonPosScaleZ") );
	mBodyCoordOffset = Vec3f( DBCClient::get()->getFloatParam("skeletonPosOffsetX"), 
		DBCClient::get()->getFloatParam("skeletonPosOffsetY"), 
		DBCClient::get()->getFloatParam("skeletonPosOffsetZ") );
	mSmoothAmount = DBCClient::get()->getFloatParam("skeletonSmoothing");
	mSmoothOnlyHands = DBCClient::get()->getBoolParam("skeletonSmoothOnlyHands");
	mScale = DBCClient::get()->getFloatParam("skeletonScale");
	mHeadScale = DBCClient::get()->getFloatParam("skeletonHeadScale");
	
	// update and smooth joint positions
	for (unsigned int j = 0; j < 25; ++j)
	{	
		Vec3f jointPos = Vec3f(skeletonData.SkeletonPositions[j].x, skeletonData.SkeletonPositions[j].y, skeletonData.SkeletonPositions[j].z);
		if (!mSmoothOnlyHands)
			mJointPositions[j] = (jointPos * mBodyCoordScale + mBodyCoordOffset) * (1.0f - mSmoothAmount) + mJointPositions[j] * mSmoothAmount;
		else
			mJointPositions[j] = (jointPos * mBodyCoordScale + mBodyCoordOffset);
		mJointPositions[j].z = 0.0f;
	}
}

#else

void Skeleton::update( IBody* pBody )
{
	pBody->get_HandLeftState(&mLeftHandState);
	pBody->get_HandRightState(&mRightHandState);

	mBodyCoordScale = Vec3f( DBCClient::get()->getFloatParam("skeletonPosScaleX"), 
		DBCClient::get()->getFloatParam("skeletonPosScaleY"), 
		DBCClient::get()->getFloatParam("skeletonPosScaleZ") );
	mBodyCoordOffset = Vec3f( DBCClient::get()->getFloatParam("skeletonPosOffsetX"), 
		DBCClient::get()->getFloatParam("skeletonPosOffsetY"), 
		DBCClient::get()->getFloatParam("skeletonPosOffsetZ") );
	mSmoothAmount = DBCClient::get()->getFloatParam("skeletonSmoothing");
	mSmoothOnlyHands = DBCClient::get()->getBoolParam("skeletonSmoothOnlyHands");
	mScale = DBCClient::get()->getFloatParam("skeletonScale");
	mHeadScale = DBCClient::get()->getFloatParam("skeletonHeadScale");
	
	// update and smooth joint positions
	Joint joints[JointType_Count]; 
	HRESULT hr = pBody->GetJoints(_countof(joints), joints);
    if (SUCCEEDED(hr))
    {
		for (int j = 0; j < _countof(joints); ++j)
		{
			Vec3f jointPos = Vec3f(joints[j].Position.X, joints[j].Position.Y, joints[j].Position.Z);
			if (!mSmoothOnlyHands || (j == JointType_HandRight || j == JointType_HandLeft))
				mJointPositions[(JointType)j] = (jointPos * mBodyCoordScale + mBodyCoordOffset) * (1.0f - mSmoothAmount) + mJointPositions[(JointType)j] * mSmoothAmount;
			else
				mJointPositions[(JointType)j] = (jointPos * mBodyCoordScale + mBodyCoordOffset);
			mJointPositions[(JointType)j].z = 0.0f;
		}
	}
}

#endif

void Skeleton::update( const Vec3f farJoints[25] )
{
	mBodyCoordScale = Vec3f( DBCClient::get()->getFloatParam("skeletonFarPosScaleX"), 
		DBCClient::get()->getFloatParam("skeletonFarPosScaleY"), 
		DBCClient::get()->getFloatParam("skeletonFarPosScaleZ") );
	mBodyCoordOffset = Vec3f( DBCClient::get()->getFloatParam("skeletonFarPosOffsetX"), 
		DBCClient::get()->getFloatParam("skeletonFarPosOffsetY"), 
		DBCClient::get()->getFloatParam("skeletonFarPosOffsetZ") );
	mSmoothAmount = DBCClient::get()->getFloatParam("skeletonSmoothing");
	mSmoothOnlyHands = DBCClient::get()->getBoolParam("skeletonSmoothOnlyHands");
	mScale = DBCClient::get()->getFloatParam("skeletonScale");
	mHeadScale = DBCClient::get()->getFloatParam("skeletonHeadScale");
	
	// update and smooth joint positions
	for (unsigned int j=0; j < 25; j++)
	{
		mJointPositions[j] = farJoints[j];
	}	
}

void Skeleton::drawBone( unsigned int jointIdx1, unsigned int jointIdx2 )
{
	// draw elongated box between joint points
	Vec3f between = getJointPos(jointIdx2) - getJointPos(jointIdx1);
	float dist = between.length();

	Matrix44f mat;
	mat.translate( getJointPos(jointIdx1) + (between / 2.0f) ); 
	mat *= Matrix44f::createRotation( Vec3f::zAxis(), between.normalized(), Vec3f::yAxis() );
	mat.scale( Vec3f(mScale, mScale, dist) ); 

	ShaderManager::get("sphere")->uniform( "matrix", mat );
	ShaderManager::get("sphere")->uniform( "color", Vec3f(1.0f, 0.0f, 0.0f) );
	gl::drawCube( Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f) );
}

void Skeleton::draw()
{
	if ( DBCClient::get()->getBoolParam("skeletonDraw") )
	{
		// draw bones
		//drawBone( JointType_Head, JointType_Neck );
		drawBone( 2, 20 );
		drawBone( 20, 1 );
		drawBone( 1, 0 );
		drawBone( 20, 8 );
		drawBone( 20, 4 );
		drawBone( 0, 16 );
		drawBone( 0, 12 );

		// Right Arm
		drawBone( 8, 9 );
		drawBone( 9, 10 );
		drawBone( 10, 11 );
		drawBone( 11, 23 );

		// Left Arm
		drawBone( 4, 5 );
		drawBone( 5, 6 );
		drawBone( 6, 7 );
		drawBone( 7, 21 );

		// Right Leg
		drawBone( 16, 17 );
		drawBone( 17, 18 );
		drawBone( 18, 19 );

		// Left Leg
		drawBone( 12, 12 );
		drawBone( 13, 14 );
		drawBone( 14, 15 );
	}

	Matrix44f sphereMat;
	float cubeSize = 30.0f;

	// draw hands
	if ( DBCClient::get()->getBoolParam("skeletonDrawHands") )
	{
		// draw left hand
		sphereMat.setToIdentity();  sphereMat.translate( getJointPos( 7 ) );
		sphereMat.rotate( Vec3f( (float)getElapsedSeconds(), (float)getElapsedSeconds(), 0.0f ) );
		ShaderManager::get("sphere")->uniform( "matrix", sphereMat );
		gl::drawCube( Vec3f(0.0f, 0.0f, 0.0f), Vec3f(cubeSize, cubeSize, cubeSize) );

		// draw right hand
		sphereMat.setToIdentity();  sphereMat.translate( getJointPos( 11 ) );
		sphereMat.rotate( Vec3f( (float)getElapsedSeconds(), (float)getElapsedSeconds(), 0.0f ) );
		ShaderManager::get("sphere")->uniform( "matrix", sphereMat );
		ShaderManager::get("sphere")->uniform( "color", Vec3f(1.0f, 0.0f, 0.0f) );
		gl::drawCube( Vec3f(0.0f, 0.0f, 0.0f), Vec3f(cubeSize, cubeSize, cubeSize) );
	}

	// draw head
	if ( DBCClient::get()->getBoolParam("skeletonDrawHead") )
	{
		sphereMat.setToIdentity();  sphereMat.translate( getJointPos( 3 ) );
		sphereMat.rotate( Vec3f( (float)getElapsedSeconds(), (float)getElapsedSeconds(), 0.0f ) );
		ShaderManager::get("sphere")->uniform( "matrix", sphereMat );
		gl::drawSphere( Vec3f(0.0f, 0.0f, 0.0f), mHeadScale, 64);
	}
}

Vec3f Skeleton::getJointPos( unsigned int jointType )
{
	return mJointPositions[jointType];
}

void Skeleton::setupSkelicleFbo()
{
    float scale = 8.0f;
	std::vector<Surface32f> surfaces;

	// Positions 2D texture array
    surfaces.push_back( Surface32f( SKELICLES_SIDE, SKELICLES_SIDE, true) );

	// Velocity 2D texture array
    surfaces.push_back( Surface32f( SKELICLES_SIDE, SKELICLES_SIDE, true) );

    // Joints 2D texture array
    surfaces.push_back( Surface32f( SKELICLES_SIDE, SKELICLES_SIDE, true) );
    
    // Offsets 2D texture array
    surfaces.push_back( Surface32f( SKELICLES_SIDE, SKELICLES_SIDE, true) );

	// Colors 2D texture array
    surfaces.push_back( Surface32f( SKELICLES_SIDE, SKELICLES_SIDE, true) );
    
    mSkelicleFbo = PingPongFbo( surfaces );
}

void Skeleton::setupSkelicleVbo()
{
	int totalVertices = SKELICLES_SIDE * SKELICLES_SIDE * 12 * 3;
    std::vector<uint32_t>	indices;
	std::vector<ci::Vec3f>	posCoords;
	std::vector<Vec2f>		texCoords;
	std::vector<ci::Vec3f>	normals;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
	layout.setStaticTexCoords2d();
    layout.setStaticNormals();

	float X = 1.0f;
	float Y = 1.0f;
	float Z = 1.0f;
	
	static Vec3f verts[8] = {
		Vec3f(-X,-Y,-Z ), Vec3f(-X,-Y, Z ), 
		Vec3f( X,-Y, Z ), Vec3f( X,-Y,-Z ),
		Vec3f(-X, Y,-Z ), Vec3f(-X, Y, Z ), 
		Vec3f( X, Y, Z ), Vec3f( X, Y,-Z ) };
	
	static GLuint vIndices[12][3] = { 
		{0,1,3}, {1,2,3},	// floor
		{4,7,5}, {7,6,5},	// ceiling
		{0,4,1}, {4,5,1},	// left
		{2,6,3}, {6,7,3},	// right
		{1,5,2}, {5,6,2},	// back
		{3,7,0}, {7,4,0} }; // front
	
	static Vec3f vNormals[6] = {
		Vec3f( 0, 1, 0 ),	// floor
		Vec3f( 0,-1, 0 ),	// ceiling
		Vec3f( 1, 0, 0 ),	// left	
		Vec3f(-1, 0, 0 ),	// right
		Vec3f( 0, 0,-1 ),	// back
		Vec3f( 0, 0, 1 ) };	// front

   
    mSkelicleVbo = gl::VboMesh( totalVertices, totalVertices, layout, GL_TRIANGLES);
	int	index = 0, cube = 0;
    for( int x = 0; x < SKELICLES_SIDE; ++x ) 
	{
        for( int y = 0; y < SKELICLES_SIDE; ++y ) 
		{
			// 12 verts for each cube
			for( int i=0; i<12; i++ )
			{
				posCoords.push_back( verts[vIndices[i][0]] );
				posCoords.push_back( verts[vIndices[i][1]] );
				posCoords.push_back( verts[vIndices[i][2]] );

				texCoords.push_back( Vec2f( x/(float)SKELICLES_SIDE, y/(float)SKELICLES_SIDE ) );
				texCoords.push_back( Vec2f( x/(float)SKELICLES_SIDE, y/(float)SKELICLES_SIDE ) );
				texCoords.push_back( Vec2f( x/(float)SKELICLES_SIDE, y/(float)SKELICLES_SIDE ) );

				indices.push_back( index++ );
				indices.push_back( index++ );
				indices.push_back( index++ );

				normals.push_back( vNormals[i/2] );
				normals.push_back( vNormals[i/2] );
				normals.push_back( vNormals[i/2] );
			}
			cube++;
        }
    }

    mSkelicleVbo.bufferIndices( indices );
	mSkelicleVbo.bufferPositions( posCoords );
	mSkelicleVbo.bufferTexCoords2d( 0, texCoords );
	mSkelicleVbo.bufferNormals( normals );
}

void Skeleton::setupSkelicles()
{
	mSkelicleSpawnIdx = 0;

	Skeleton::setupSkelicleVbo();
	Skeleton::setupSkelicleFbo();
}

void Skeleton::updateSkelicles(Room& room, int skeletonCount, int farSkeletonCount, Skeleton (&skeletons)[3], Skeleton (&farSkeletons)[3])
{
	gl::disableAlphaBlending();
	gl::setMatricesWindow( mSkelicleFbo.getSize(), false ); // false to prevent vertical flipping
	gl::setViewport( mSkelicleFbo.getBounds() );
	mSkelicleFbo.bindUpdate();

	ShaderManager::get("skelicleUpdate")->bind();
	ShaderManager::get("skelicleUpdate")->uniform( "positions", 0 );
	ShaderManager::get("skelicleUpdate")->uniform( "velocity", 1 );
	ShaderManager::get("skelicleUpdate")->uniform( "joints", 2 );
	ShaderManager::get("skelicleUpdate")->uniform( "offsets", 3 );
	ShaderManager::get("skelicleUpdate")->uniform( "colors", 4 );

	// pass in ALL skeleton positions
	Vec3f skeletonPositions[6 * 25];
	for (int s=0; s<skeletonCount; s++)
	{
		for (unsigned int j=0; j<25; j++)
			skeletonPositions[s*25 + j] = skeletons[s].getJointPos(j);
	}
	for (int s=0; s<farSkeletonCount; s++)
	{
		for (unsigned int j=0; j<25; j++)
			skeletonPositions[(s + 3)*25 + j] = farSkeletons[s].getJointPos(j);
	}
	ShaderManager::get("skelicleUpdate")->uniform( "skeletonPositions", skeletonPositions, 6 * 25 );

	ShaderManager::get("skelicleUpdate")->uniform( "skeletonCount", skeletonCount );
	ShaderManager::get("skelicleUpdate")->uniform( "farSkeletonCount", farSkeletonCount );
	ShaderManager::get("skelicleUpdate")->uniform( "time", (float)app::getElapsedSeconds() );
	ShaderManager::get("skelicleUpdate")->uniform( "elapsed", (float)room.getTimeDelta() );
	ShaderManager::get("skelicleUpdate")->uniform( "roomDims", room.getDims() );
	ShaderManager::get("skelicleUpdate")->uniform( "centerAttract", DBCClient::get()->getCenterAttract() );
	ShaderManager::get("skelicleUpdate")->uniform( "trackingScale", DBCClient::get()->getFloatParam("skeletonTrackingScale") );

	gl::drawSolidRect(mSkelicleFbo.getBounds());
	ShaderManager::get("skelicleUpdate")->unbind();
	mSkelicleFbo.unbindUpdate();
}

void Skeleton::drawSkelicles(Room& room, SpringCam& camera)
{
	mSkelicleFbo.bindTexture(0);
	mSkelicleFbo.bindTexture(1);
	mSkelicleFbo.bindTexture(2);
	ShaderManager::get("skelicleDraw")->bind();
	ShaderManager::get("skelicleDraw")->uniform( "positions", 0 );
	ShaderManager::get("skelicleDraw")->uniform( "velocity", 1 );
	ShaderManager::get("skelicleDraw")->uniform( "joints", 2 );
	ShaderManager::get("skelicleDraw")->uniform( "offsets", 3 );
	ShaderManager::get("skelicleDraw")->uniform( "colors", 4 );
	ShaderManager::get("skelicleDraw")->uniform( "audioLevel", (DBCClient::get()->getAudioLevel() * DBCClient::get()->getFloatParam("skeletonAudioMod")) + (1.0f - DBCClient::get()->getFloatParam("skeletonAudioMod")) );
	ShaderManager::get("skelicleDraw")->uniform( "elapsed", (float)getElapsedSeconds() );
	ShaderManager::get("skelicleDraw")->uniform( "size", DBCClient::get()->getFloatParam("particleSize") );
	ShaderManager::get("skelicleDraw")->uniform( "lifeScale", DBCClient::get()->getFloatParam("particleLifeScale") );
	ShaderManager::get("skelicleDraw")->uniform( "eyePos", camera.mEye );
	ShaderManager::get("skelicleDraw")->uniform( "roomDims", room.getDims() );
	ShaderManager::get("skelicleDraw")->uniform( "color", Vec3f(1.0f, 0.0f, 0.0f) );

	gl::draw( mSkelicleVbo );
	ShaderManager::get("skelicleDraw")->unbind();
	mSkelicleFbo.unbindTexture();
}

// add skelicles along bone
void Skeleton::addSkelicles(unsigned int jointIdx1, unsigned int jointIdx2, unsigned int count, float offsetAmount)
{
	gl::disableAlphaBlending();
	gl::setMatricesWindow( mSkelicleFbo.getSize(), false ); // false to prevent vertical flipping
	gl::setViewport( mSkelicleFbo.getBounds() );

	ShaderManager::get("skelicleAdd")->bind();
	mSkelicleFbo.bindFramebuffer(true);

	ShaderManager::get("skelicleAdd")->uniform( "elapsed", (float)getElapsedSeconds() );
	ShaderManager::get("skelicleAdd")->uniform( "life", 0.0f );
	ShaderManager::get("skelicleAdd")->uniform( "color", mColor );

	for (unsigned int i=0; i < count; i++)
	{
		int index = mSkelicleSpawnIdx % (SKELICLES_SIDE*SKELICLES_SIDE);
		int xPos = (int)(index / SKELICLES_SIDE);
		int yPos = index % SKELICLES_SIDE;

		float skeletonIndex = (float)mIndex/255.0f;
		float joint1Idx = (float)jointIdx1/255.0f;
		float joint2Idx = (float)jointIdx2/255.0f;
		float jointLerp = (float)i/(float)count;

		ShaderManager::get("skelicleAdd")->uniform( "pos", Rand::randVec3f() * 100.0f);

		ShaderManager::get("skelicleAdd")->uniform( "skeletonIdx", skeletonIndex );
		ShaderManager::get("skelicleAdd")->uniform( "jointType1", joint1Idx );
		ShaderManager::get("skelicleAdd")->uniform( "jointType2", joint2Idx );
		ShaderManager::get("skelicleAdd")->uniform( "jointLerp", jointLerp );

		ShaderManager::get("skelicleAdd")->uniform( "offset", Rand::randVec3f() * offsetAmount);
		ShaderManager::get("skelicleAdd")->uniform( "rotation", Rand::randFloat());
		ShaderManager::get("skelicleAdd")->uniform( "scale", Rand::randFloat());

		gl::drawSolidRect(Rectf((float)xPos, (float)yPos, (float)xPos+1, (float)yPos+1));
		mSkelicleSpawnIdx++;
	}

	mSkelicleFbo.unbindFramebuffer(true);
	ShaderManager::get("skelicleAdd")->unbind();
}