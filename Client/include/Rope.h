#pragma once
#include "cinder/Perlin.h"

class Emitter;
struct Particle;

struct Constraint
{
	Constraint(Particle* p, float d) : particle(p), distance(d) {}

    Particle* particle;
    float distance;
};

struct Particle
{
	Particle() {}
	Particle(ci::Vec3f p, ci::Vec3f n, bool f = false) : position(p), lastPosition(p), forcePosition(ci::Vec3f(0,0,0)), normal(n), isFixed(f) {}

	ci::Vec3f position;
    ci::Vec3f lastPosition;
    ci::Vec3f forcePosition;
	ci::Vec3f normal;
	bool isFixed;
    std::vector<Constraint*> constraints;
    
	void addConstraint(Constraint* constraint) { constraints.push_back( constraint ); }
    void applyForce(ci::Vec3f force) { forcePosition = force; }
    void applyDelta(ci::Vec3f dPosition) { position + dPosition; }

	void verlet(float timeStep, float verletFactor);
	void doConstraint(Constraint* constraint);
    void satisfyConstraints( bool flip );
};

class Rope 
{
	unsigned int mLength;
	ci::Vec3f	mPos;
	ci::Vec3f	mWind;
	ci::Vec3f	mGravity;

	std::vector<Particle*>	mParticles;
	
	float		mRadius;
	ci::Color	mColor;
	
	float		mVerletFactor;
	int			mFrameCount;
	float		mAge;
	float		mAgePer;
	float		mLifespan;
	bool		mIsDead;

  public:
	Particle*	mHandParticle;
	Rope();

	void setPos( ci::Vec3f pos );
	void update( float dt, Emitter* emitter );
	void draw();
};
