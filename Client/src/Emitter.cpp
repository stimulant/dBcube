#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "Emitter.h"
#include "AssetManager.h"
#include "ShaderManager.h"
#include "DBCClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define PARTICLES_SIDE 128

int					Emitter::mParticleSpawnIdx;
PingPongFbo			Emitter::mParticlesFbo;
gl::VboMesh			Emitter::mParticlesVbo;

Emitter::Emitter(int side, int user, bool driftAway)
{
	mSide = side+1;
	mUser = user+1;
	mDriftAway = driftAway;
	mConnectedEmitter = NULL;
}

Emitter::Emitter(bool driftAway)
{
	mSide = 1;
	mUser = 1;
	mDriftAway = driftAway;
	mConnectedEmitter = NULL;
}

void Emitter::updateColor()
{
	// create named string for retreiving color
	std::stringstream converter;
	converter << "emitterColorSide" << mSide << "User" << mUser;
	mColorString = converter.str();
	mColor = DBCClient::get()->getColorParam( mColorString );
}

void Emitter::update( float dt )
{
	mRope.update( dt, mConnectedEmitter );
	updateColor();
}

void Emitter::setPos( const ci::Vec3f &pos )
{
	mVel = pos - mPos;
	mPos = pos;
	mRope.setPos( mPos );
}

void Emitter::drawRope(Room& room, SpringCam& camera)
{
	gl::setMatrices( camera.getCam() );
	gl::disable( GL_TEXTURE_2D );

	ShaderManager::get("ribbon")->bind();
	ShaderManager::get("ribbon")->uniform( "audioLevel", (DBCClient::get()->getAudioLevel() * DBCClient::get()->getFloatParam("ribbonAudioMod")) + (1.0f - DBCClient::get()->getFloatParam("ribbonAudioMod")) );
	ShaderManager::get("ribbon")->uniform( "mvpMatrix", camera.mMvpMatrix );
	ShaderManager::get("ribbon")->uniform( "eyePos", camera.getEye() );
	ShaderManager::get("ribbon")->uniform( "power", room.getPower() );
	ShaderManager::get("ribbon")->uniform( "roomDims", room.getDims() );
	ShaderManager::get("ribbon")->uniform( "att", 1.015f );
	ShaderManager::get("ribbon")->uniform( "color", mColor );
	ShaderManager::get("ribbon")->uniform( "matrix", Matrix44f() );

	mRope.draw();

	ShaderManager::get("ribbon")->unbind();
}

void Emitter::setupParticleFbo()
{
    std::vector<Surface32f> surfaces;

    // Position 2D texture array
    surfaces.push_back( Surface32f( PARTICLES_SIDE, PARTICLES_SIDE, true) );
    Surface32f::Iter pixelIter = surfaces[0].getIter();
    while( pixelIter.line() ) 
	{
        while( pixelIter.pixel() ) 
		{
            // Initial particle positions are passed in as R,G,B float values. Alpha is used as particle invMass.
            surfaces[0].setPixel(pixelIter.getPos(), ColorAf(0.0f, 0.0f, 0.0f, Rand::randFloat(0.2f, 1.0f) ) );
        }
    }
    
    //Velocity 2D texture array
    surfaces.push_back( Surface32f( PARTICLES_SIDE, PARTICLES_SIDE, true) );
    pixelIter = surfaces[1].getIter();
    while( pixelIter.line() ) 
	{
        while( pixelIter.pixel() ) 
		{
            // Initial particle velocities are passed in as R,G,B float values.
            surfaces[1].setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
        }
    }

	//Color 2D texture array
    surfaces.push_back( Surface32f( PARTICLES_SIDE, PARTICLES_SIDE, true) );
    pixelIter = surfaces[2].getIter();
    while( pixelIter.line() ) 
	{
        while( pixelIter.pixel() ) 
		{
            // Initial particle velocities are passed in as R,G,B float values.
            surfaces[2].setPixel( pixelIter.getPos(), ColorAf( 0.0f, 1.0f, 0.0, 1.0f ) );
        }
    }
    mParticlesFbo = PingPongFbo( surfaces );
}

void Emitter::setupParticleVbo()
{
	int totalVertices = PARTICLES_SIDE * PARTICLES_SIDE * 12 * 3;
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

   
    mParticlesVbo = gl::VboMesh( totalVertices, totalVertices, layout, GL_TRIANGLES);
	int	index = 0, cube = 0;
    for( int x = 0; x < PARTICLES_SIDE; ++x ) 
	{
        for( int y = 0; y < PARTICLES_SIDE; ++y ) 
		{
			// 12 verts for each cube
			for( int i=0; i<12; i++ )
			{
				posCoords.push_back( verts[vIndices[i][0]] );
				posCoords.push_back( verts[vIndices[i][1]] );
				posCoords.push_back( verts[vIndices[i][2]] );

				texCoords.push_back( Vec2f( x/(float)PARTICLES_SIDE, y/(float)PARTICLES_SIDE ) );
				texCoords.push_back( Vec2f( x/(float)PARTICLES_SIDE, y/(float)PARTICLES_SIDE ) );
				texCoords.push_back( Vec2f( x/(float)PARTICLES_SIDE, y/(float)PARTICLES_SIDE ) );

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

    mParticlesVbo.bufferIndices( indices );
	mParticlesVbo.bufferPositions( posCoords );
	mParticlesVbo.bufferTexCoords2d( 0, texCoords );
	mParticlesVbo.bufferNormals( normals );
}

void Emitter::setupParticles()
{
	mParticleSpawnIdx = 0;

	Emitter::setupParticleVbo();
	Emitter::setupParticleFbo();
}

void Emitter::updateParticles(Room& room)
{
	if (DBCClient::get()->getBoolParam("particleDraw"))
	{
		gl::disableAlphaBlending();
		gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
		gl::setViewport( mParticlesFbo.getBounds() );
		mParticlesFbo.bindUpdate();
		ShaderManager::get("particleUpdate")->bind();
		ShaderManager::get("particleUpdate")->uniform( "audioLevel", DBCClient::get()->getAudioLevel() );
		ShaderManager::get("particleUpdate")->uniform( "time", (float)app::getElapsedSeconds() );
		ShaderManager::get("particleUpdate")->uniform( "elapsed", (float)room.getTimeDelta() );
		ShaderManager::get("particleUpdate")->uniform( "bounce", DBCClient::get()->getFloatParam("particleBounce") );
		ShaderManager::get("particleUpdate")->uniform( "noiseAmount", DBCClient::get()->getFloatParam("particleNoiseAmount") );
		ShaderManager::get("particleUpdate")->uniform( "positions", 0 );
		ShaderManager::get("particleUpdate")->uniform( "velocities", 1 );
		ShaderManager::get("particleUpdate")->uniform( "colors", 2 );
		ShaderManager::get("particleUpdate")->uniform( "roomDims", room.getDims() );
		gl::drawSolidRect(mParticlesFbo.getBounds());
		ShaderManager::get("particleUpdate")->unbind();
		mParticlesFbo.unbindUpdate();
	}
}

void Emitter::drawParticles(Room& room, SpringCam& camera)
{
	if (DBCClient::get()->getBoolParam("particleDraw"))
	{
		mParticlesFbo.bindTexture(0);
		mParticlesFbo.bindTexture(1);
		mParticlesFbo.bindTexture(2);
		ShaderManager::get("particleDraw")->bind();
		ShaderManager::get("particleDraw")->uniform("displacementMap", 0 );
		ShaderManager::get("particleDraw")->uniform("velocityMap", 1);
		ShaderManager::get("particleDraw")->uniform("colorMap", 2);

		ShaderManager::get("particleDraw")->uniform( "cubeMap", 0 );
		ShaderManager::get("particleDraw")->uniform( "size", DBCClient::get()->getFloatParam("particleSize") );
		ShaderManager::get("particleDraw")->uniform( "lifeScale", DBCClient::get()->getFloatParam("particleLifeScale") );
		ShaderManager::get("particleDraw")->uniform( "eyePos", camera.mEye );
		ShaderManager::get("particleDraw")->uniform( "roomDims", room.getDims() );
		ShaderManager::get("particleDraw")->uniform( "color", Vec3f(1.0f, 0.0f, 0.0f) );

		gl::draw( mParticlesVbo );
		ShaderManager::get("particleDraw")->unbind();
		mParticlesFbo.unbindTexture();
	}
}

void Emitter::addParticles(int count)
{
	if (DBCClient::get()->getBoolParam("particleDraw"))
	{
		gl::disableAlphaBlending();
		gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
		gl::setViewport( mParticlesFbo.getBounds() );
		ShaderManager::get("particleAdd")->bind();
		ShaderManager::get("particleAdd")->uniform( "audioLevel", DBCClient::get()->getAudioLevel() );

		float particleAwaySpeed = DBCClient::get()->getFloatParam("particleAwaySpeed");
		mParticlesFbo.bindFramebuffer(true);
		for (int i=0; i < count; i++)
		{
			int index = mParticleSpawnIdx % (PARTICLES_SIDE*PARTICLES_SIDE);
			int xPos = (int)(index / PARTICLES_SIDE);
			int yPos = index % PARTICLES_SIDE;

			ShaderManager::get("particleAdd")->uniform( "pos", mPos );
			ShaderManager::get("particleAdd")->uniform( "elapsed", (float)getElapsedSeconds() );
			ShaderManager::get("particleAdd")->uniform( "life", 0.0f );
			ShaderManager::get("particleAdd")->uniform( "color", mColor );
			ShaderManager::get("particleAdd")->uniform( "vel", Vec3f(0, 0, mDriftAway ? particleAwaySpeed : -particleAwaySpeed) + Rand::randVec3f() * 2.0f);

			gl::drawSolidRect(Rectf((float)xPos, (float)yPos, (float)xPos+1, (float)yPos+1));
			mParticleSpawnIdx++;
		}

		mParticlesFbo.unbindFramebuffer(true);
		ShaderManager::get("particleAdd")->unbind();
	}
}