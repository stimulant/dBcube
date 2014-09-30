#pragma once

#include "cinder/Filesystem.h"
#include "cinder/DataSource.h"
#include "cinder/gl/GlslProg.h"

class ShaderManager {
public:
    static void load( const std::string shaderName, ci::fs::path vertexShaderPath, ci::fs::path fragmentShaderPath );
	static ci::gl::GlslProgRef ShaderManager::get(const std::string shaderName);
	static std::map< std::string, ci::gl::GlslProgRef > sShaderMap;
    
protected:
    ShaderManager(){}
    static ShaderManager* instance();
    void update();
    static ShaderManager* sInstance;
};
