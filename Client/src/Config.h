#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Json.h"
#include "cinder/Function.h"
#include "cinder/gl/gl.h"

class Config;

typedef std::shared_ptr< Config > ConfigRef;

ci::Vec3f JsonToVec3f(ci::JsonTree tree);
ci::ColorAf JsonToColor(ci::JsonTree tree);

class Config : public ci::JsonTree
{
public:
	static ConfigRef create( ci::DataSourceRef dataSource, ParseOptions parseOptions = ParseOptions() );
	static ConfigRef create( const std::string &jsonString, ParseOptions parseOptions = ParseOptions() );

	Config( ci::DataSourceRef dataSource, ParseOptions parseOptions = ParseOptions() );
	Config( const std::string &jsonString, ParseOptions parseOptions = ParseOptions() );

	void	setServerConfig( ci::JsonTree &json );
};