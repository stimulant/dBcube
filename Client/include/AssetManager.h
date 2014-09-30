#pragma once

#include "cinder/Filesystem.h"
#include "cinder/DataSource.h"

typedef std::shared_ptr<class XAsset> XAssetRef;

class XAsset {
public:
    XAsset(){}
    XAsset( ci::fs::path relativePath, std::function<void(ci::DataSourceRef)> callback );
    
    virtual void refresh();
    virtual void notify();
    
protected:
    ci::fs::path                            mRelativePath;
    std::time_t                             mLastTimeWritten;
    std::function<void(ci::DataSourceRef)>  mCallback;
};

class XAssetPair : public XAsset {
public:
    XAssetPair( ci::fs::path firstRelativePath, ci::fs::path secondRelativePath, std::function<void(ci::DataSourceRef, ci::DataSourceRef)> callback );
    
    virtual void refresh();
    virtual void notify();
    
protected:
    std::pair<ci::fs::path,ci::fs::path>                        mRelativePath;
    std::pair<std::time_t,std::time_t>                          mLastTimeWritten;
    std::function<void(ci::DataSourceRef, ci::DataSourceRef)>   mCallback;
};

class XAssetTriple : public XAsset {
public:
    XAssetTriple( ci::fs::path firstRelativePath, ci::fs::path secondRelativePath, ci::fs::path thirdRelativePath, std::function<void(ci::DataSourceRef, ci::DataSourceRef, ci::DataSourceRef)> callback );
    
    virtual void refresh();
    virtual void notify();
    
protected:
    std::tuple<ci::fs::path,ci::fs::path,ci::fs::path>         mRelativePath;
    std::tuple<std::time_t,std::time_t,std::time_t>            mLastTimeWritten;
    std::function<void(ci::DataSourceRef, ci::DataSourceRef, ci::DataSourceRef)>  mCallback;
};

class AssetManager {
public:
    
    static void load( const ci::fs::path &relativePath, std::function<void(ci::DataSourceRef)> callback );
    static void load( const ci::fs::path &vertexRelPath, const ci::fs::path &fragmentRelPath, std::function<void(ci::DataSourceRef,ci::DataSourceRef)> callback );
	static void load( const ci::fs::path &vertexRelPath, const ci::fs::path &fragmentRelPath, const ci::fs::path &geomRelPath, std::function<void(ci::DataSourceRef,ci::DataSourceRef,ci::DataSourceRef)> callback );
    
protected:
    AssetManager(){}
    
    static AssetManager* instance();
    
    void update();
    
    std::deque< XAssetRef > mAssets;
    static AssetManager* mInstance;
};
