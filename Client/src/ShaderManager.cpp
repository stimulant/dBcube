#include "ShaderManager.h"
#include "AssetManager.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ShaderManager* ShaderManager::sInstance;
std::map< std::string, ci::gl::GlslProgRef > ShaderManager::sShaderMap;

#define REGISTER_SHADER(file1, file2, shader) \
AssetManager::load( file1, file2, [this](DataSourceRef vert,DataSourceRef frag){ \
    try{ \
		app::console() << "compiling shader " << file1 << ", " << file2 << endl; \
        shader = gl::GlslProg( vert, frag ); \
    } catch( gl::GlslProgCompileExc exc ) { \
		app::console() << exc.what() << endl; \
	} \
} );

#define REGISTER_GEOM_SHADER(file1, file2, file3, shader) \
AssetManager::load( file1, file2, file3, [this](DataSourceRef vert,DataSourceRef frag, DataSourceRef geom){ \
    try{ \
		int32_t maxGeomOutputVertices; \
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &maxGeomOutputVertices); \
		shader = gl::GlslProg(vert, frag, geom, GL_POINTS, GL_TRIANGLE_STRIP, maxGeomOutputVertices); \
    } catch( gl::GlslProgCompileExc exc ) { \
		app::console() << exc.what() << endl; \
	} \
} );

void ShaderManager::load( const std::string shaderName, ci::fs::path vertexShaderPath, ci::fs::path fragmentShaderPath )
{
	AssetManager::load( vertexShaderPath, fragmentShaderPath, [=](DataSourceRef vert,DataSourceRef frag){
		try{
			app::console() << "compiling shader " << shaderName << ", " << vertexShaderPath << endl;
			ShaderManager::sShaderMap[shaderName] = gl::GlslProg::create( vert, frag );
		} catch( gl::GlslProgCompileExc exc ) {
			app::console() << exc.what() << endl;
		}
	} );
}

ci::gl::GlslProgRef ShaderManager::get(const std::string shaderName)
{
	return sShaderMap[shaderName];
}

ShaderManager* ShaderManager::instance()
{
    if( !sInstance ){
        sInstance = new ShaderManager();
        ci::app::App::get()->getSignalUpdate().connect( std::bind( &ShaderManager::update, sInstance ) );
    }
    return sInstance;
}

void ShaderManager::update()
{
}