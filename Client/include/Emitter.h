#pragma once
#include "cinder/Perlin.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"

#include "PingPongFbo.h"
#include "Rope.h"
#include "Room.h"
#include "SpringCam.h"

class Emitter 
{
	ci::Vec3f	mPos;
	ci::Vec3f	mVel;
	Rope		mRope;
	int			mSide;
	int			mUser;
	std::string	mColorString;
	bool		mDriftAway;
	ci::ColorAf	mColor;
	Emitter		*mConnectedEmitter;

	// Particles
	static int					mParticleSpawnIdx;
	static PingPongFbo			mParticlesFbo;
    static gl::VboMesh			mParticlesVbo;

	static void	setupParticleFbo();
	static void	setupParticleVbo();
	void Emitter::updateColor();

  public:
	Emitter(bool driftAway = true);
	Emitter(int side, int user, bool driftAway = true);

	void	connect( Emitter* emitter )	{ mConnectedEmitter = emitter; }
	void	disconnect()				{ mConnectedEmitter = NULL; }
	void	update( float dt );
	void	setPos( const ci::Vec3f &pos );
	const ci::Vec3f& getPos() { return mPos; }
	const ci::Vec3f& getVel() { return mVel; }

	void	drawRope(Room& room, SpringCam& camera);

	static void	setupParticles();
	static void	updateParticles(Room& room);
	static void	drawParticles(Room& room, SpringCam& camera);
	void	addParticles(int count);
};