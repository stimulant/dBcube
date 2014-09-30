#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "Rope.h"
#include "Emitter.h"
#include "DBCClient.h"

using namespace ci;

#define CONSTRAINT_RELAXATION_ITERATIONS 2

void Particle::verlet(float timeStep, float verletFactor)
{
	Vec3f velocity = position - lastPosition;
    lastPosition = position;

    velocity += forcePosition * (timeStep * timeStep);
    position += velocity * verletFactor;
}

void Particle::doConstraint(Constraint* constraint)
{
	Vec3f diff = position - constraint->particle->position;
	float d1 = diff.length();
    float d2 = 0;
    if (d1 != 0)
        d2 = 0.5f * (d1 - constraint->distance) / d1;
    Vec3f dPosition = diff * d2;

	if (!isFixed)
		position -= dPosition;
	if (!constraint->particle->isFixed)
		constraint->particle->applyDelta(dPosition);
    else if (!isFixed)
		position -= dPosition;
}

void Particle::satisfyConstraints( bool flip )
{
	if (flip)
	{
		for (int i = (int)constraints.size()-1; i >= 0; i--)
			doConstraint(constraints[i]);
	}
	else
	{
		for (unsigned int i = 0; i < constraints.size(); i++) 
			doConstraint(constraints[i]);
	}
}

Rope::Rope()
{
	mPos = Vec3f(0.0f, 0.0f, 0.0f);

	mWind = Vec3f(0.0f, 0.0f, 10.2f);
	mGravity = Vec3f(0.0f, -1.0f, 0.0f);
	
	mLength = 60;
	mHandParticle = new Particle(mPos, Vec3f::yAxis(), true);

	// add particles
	for( unsigned int i=0; i<mLength; i++ )
	{
		mParticles.push_back( new Particle(mPos, Vec3f::yAxis()) );
	}

	// add constraints in two directions
	for( unsigned int i=0; i<mLength; i++ )
	{
		if (i == 0)
            mParticles[i]->addConstraint( new Constraint(mHandParticle, 4) );
		else
			mParticles[i]->addConstraint( new Constraint(mParticles[i - 1], 8) );

		if (i!=mLength-1)
			mParticles[i]->addConstraint( new Constraint(mParticles[i + 1], 8) );	
	}
	
	int i = Rand::randInt(4);
	if( i == 0 ){
		mColor = Color( 0.6f, 0.1f, 0.0f );
	} else if( i == 1 ){
		mColor = Color( 1.0f, 0.5f, 0.0f );
	} else if( i == 2 ){
		mColor = Color( 0.0f, 0.0f, 0.0f );
	}
	
	mFrameCount	= 0;
	mAge		= 0.0f;
}

void Rope::setPos( Vec3f pos )
{
	mPos = pos;
	mHandParticle->position = pos;
}

void Rope::update( float dt, Emitter* emitter )
{
	mVerletFactor = DBCClient::get()->getFloatParam("ribbonVerletFactor");

	// update far hand particle to connect ropes
	if (emitter != NULL)
	{
		mParticles[mLength-1]->isFixed = true;
		mParticles[mLength-1]->position = emitter->getPos();
		mWind = Vec3f(0.0f, 0.0f, DBCClient::get()->getFloatParam("ribbonConnectedWind"));
		mGravity = Vec3f(0.0f, -DBCClient::get()->getFloatParam("ribbonConnectedGravity"), 0.0f);
	}
	else
	{
		mParticles[mLength-1]->isFixed = false;
		mWind = Vec3f(0.0f, 0.0f, DBCClient::get()->getFloatParam("ribbonDisconnectedWind"));
		mGravity = Vec3f(0.0f, -DBCClient::get()->getFloatParam("ribbonDisconnectedGravity"), 0.0f);
	}

	// accumulate forces
	for (unsigned int i = 0; i < mLength; i++) 
	{
		ci::Vec3f forcePoint = Vec3f(0, 0, 0);
		forcePoint += mWind;
		forcePoint += mGravity;
		mParticles[i]->applyForce(forcePoint);
    }

	// verlet
	for (unsigned int i = 0; i < mLength; i++)
		mParticles[i]->verlet(dt, mVerletFactor);

	// satisfy constraints
	for (unsigned int iterations = 0; iterations < CONSTRAINT_RELAXATION_ITERATIONS; iterations++) 
	{
		for (unsigned int i = 0; i < mLength; i++)
            mParticles[i]->satisfyConstraints(iterations%2==0);
    }
	
	// update normals
	for( int i=mLength-1; i>0; i-- )
		mParticles[i]->normal = mParticles[i-1]->normal;
	Vec3f normal = Vec3f::yAxis();
	normal.rotateZ( mAge * 0.02f * DBCClient::get()->getFloatParam("ribbonTwistSpeed") );
	mParticles[0]->normal = normal;
	
	mAge += dt;
}

void Rope::draw()
{
	glBegin( GL_TRIANGLE_STRIP );
	float startSize = DBCClient::get()->getFloatParam("ribbonStartSize");
	float endSize = DBCClient::get()->getFloatParam("ribbonEndSize");
	int length = mLength;

	float audioScale = 1.0f;
	if (DBCClient::get()->getBoolParam("ribbonScaleWithAudio") )
		audioScale = (DBCClient::get()->getAudioLevel() * DBCClient::get()->getFloatParam("ribbonAudioMod")) + (1.0f - DBCClient::get()->getFloatParam("ribbonAudioMod"));
	float RopeAlphaFade = DBCClient::get()->getFloatParam("ribbonAlphaFade");

	for( int i=0; i<length-1; i++ )
	{
		Vec3f p1 = mParticles[i]->position;
		Vec3f p2 = mParticles[i+1]->position;
		Vec3f dir = p2 - p1;
		dir.normalize();

		Vec3f perp1 = dir.cross( mParticles[i]->normal );
		perp1.normalize();

		glNormal3f(mParticles[i]->normal);
		gl::color(ColorA(1.0f, 1.0f, 1.0f, lmap<float>((float)i, 0, (float)(length-1), 1.0f, 1.0f - RopeAlphaFade)));
		gl::vertex( p1 - perp1 * lmap<float>((float)i, 0, (float)(length-1), startSize, endSize) * audioScale );
		
		glNormal3f(mParticles[i]->normal);
		gl::color(ColorA(1.0f, 1.0f, 1.0f, lmap<float>((float)i, 0, (float)(length-1), 1.0f, 1.0f - RopeAlphaFade)));
		gl::vertex( p1 + perp1 * lmap<float>((float)i, 0, (float)(length-1), startSize, endSize) * audioScale );
	}
	glEnd();
}