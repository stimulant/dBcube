#include "Config.h"
#include "cinder/app/App.h"

using namespace ci;

Vec3f JsonToVec3f(ci::JsonTree tree)
{
	return Vec3f(tree.getChild(0).getValue<float>(),
				 tree.getChild(1).getValue<float>(),
				 tree.getChild(2).getValue<float>());
}

ColorAf JsonToColor(ci::JsonTree tree)
{
	return ColorAf(tree.getChild(0).getValue<float>(),
				   tree.getChild(1).getValue<float>(),
				   tree.getChild(2).getValue<float>(),
				   tree.getChild(3).getValue<float>());
}

ConfigRef Config::create( DataSourceRef dataSource, ParseOptions parseOptions )
{
	return (ConfigRef)(new Config( dataSource, parseOptions ));
}

ConfigRef Config::create( const std::string &jsonString, ParseOptions parseOptions )
{
	return (ConfigRef)(new Config( jsonString, parseOptions ));
}


Config::Config( DataSourceRef dataSource, ParseOptions parseOptions ) :
	JsonTree( dataSource, parseOptions )
{
}
Config::Config( const std::string &jsonString, ParseOptions parseOptions ) :
	JsonTree( jsonString, parseOptions )
{
}

void Config::setServerConfig( JsonTree &json )
{
	auto subtree = makeObject( "server" );
	for ( const auto it : json ) {
		subtree.pushBack( it );
	}
	pushBack( subtree );
}