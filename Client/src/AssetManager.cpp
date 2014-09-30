#include "AssetManager.h"
#include "cinder/app/App.h"

XAsset::XAsset( ci::fs::path relativePath, std::function<void(ci::DataSourceRef)> callback )
:   mRelativePath( relativePath ),
    mCallback( callback ),
    mLastTimeWritten( ci::fs::last_write_time( ci::app::getAssetPath( relativePath ) ) )
{
}

void XAsset::refresh()
{
    std::time_t time = ci::fs::last_write_time( ci::app::getAssetPath( mRelativePath ) );
    if( time > mLastTimeWritten ){
        mLastTimeWritten   = time;
        notify();
    }
}
void XAsset::notify()
{
    mCallback( ci::app::loadAsset( mRelativePath ) );
}



XAssetPair::XAssetPair( ci::fs::path firstRelativePath, ci::fs::path secondRelativePath, std::function<void(ci::DataSourceRef, ci::DataSourceRef)> callback )
:   mRelativePath( std::make_pair( firstRelativePath, secondRelativePath ) ),
    mLastTimeWritten( std::make_pair( ci::fs::last_write_time( ci::app::getAssetPath( firstRelativePath ) ), ci::fs::last_write_time( ci::app::getAssetPath( secondRelativePath ) ) ) ),
    mCallback( callback )
{
}
    
void XAssetPair::refresh()
{
    std::time_t firstTime = ci::fs::last_write_time( ci::app::getAssetPath( mRelativePath.first ) );
    std::time_t secondTime = ci::fs::last_write_time( ci::app::getAssetPath( mRelativePath.second ) );
    if( firstTime > mLastTimeWritten.first || secondTime > mLastTimeWritten.second ){
        mLastTimeWritten.first      = firstTime;
        mLastTimeWritten.second     = secondTime;
        notify();
    }
}
void XAssetPair::notify()
{
    mCallback( ci::app::loadAsset( mRelativePath.first ), ci::app::loadAsset( mRelativePath.second ) );
}


XAssetTriple::XAssetTriple( ci::fs::path firstRelativePath, ci::fs::path secondRelativePath, ci::fs::path thirdRelativePath, std::function<void(ci::DataSourceRef, ci::DataSourceRef, ci::DataSourceRef)> callback )
:   mRelativePath( std::make_tuple( firstRelativePath, secondRelativePath, thirdRelativePath ) ),
    mLastTimeWritten( std::make_tuple( ci::fs::last_write_time( ci::app::getAssetPath( firstRelativePath ) ), ci::fs::last_write_time( ci::app::getAssetPath( secondRelativePath ) ), ci::fs::last_write_time( ci::app::getAssetPath( thirdRelativePath ) ) ) ),
    mCallback( callback )
{
}
    
void XAssetTriple::refresh()
{
    std::time_t firstTime = ci::fs::last_write_time( ci::app::getAssetPath( std::get<0>(mRelativePath) ) );
    std::time_t secondTime = ci::fs::last_write_time( ci::app::getAssetPath( std::get<1>(mRelativePath) ) );
	std::time_t thirdTime = ci::fs::last_write_time( ci::app::getAssetPath( std::get<2>(mRelativePath) ) );
    if( firstTime > std::get<0>(mLastTimeWritten) || secondTime > std::get<1>(mLastTimeWritten)  || thirdTime > std::get<2>(mLastTimeWritten) ){
        std::get<0>(mLastTimeWritten)      = firstTime;
        std::get<1>(mLastTimeWritten)     = secondTime;
		std::get<2>(mLastTimeWritten)     = thirdTime;
        notify();
    }
}
void XAssetTriple::notify()
{
    mCallback( ci::app::loadAsset( std::get<0>(mRelativePath) ), ci::app::loadAsset( std::get<1>(mRelativePath) ), ci::app::loadAsset( std::get<2>(mRelativePath) ) );
}


void AssetManager::load( const ci::fs::path &relativePath, std::function<void(ci::DataSourceRef)> callback )
{
    if( ci::fs::exists( ci::app::getAssetPath( relativePath ) ) ){
        XAssetRef xAsset = XAssetRef( new XAsset( relativePath, callback ) );
        instance()->mAssets.push_back( xAsset );
        xAsset->notify();
    }
    else
        throw ci::app::AssetLoadExc( relativePath );
};
void AssetManager::load( const ci::fs::path &vertexRelPath, const ci::fs::path &fragmentRelPath, std::function<void(ci::DataSourceRef,ci::DataSourceRef)> callback )
{
    if( ci::fs::exists( ci::app::getAssetPath( vertexRelPath ) ) && ci::fs::exists( ci::app::getAssetPath( fragmentRelPath ) ) ){
        XAssetRef xAsset = XAssetRef( new XAssetPair( vertexRelPath, fragmentRelPath, callback ) );
        instance()->mAssets.push_back( xAsset );
        xAsset->notify();
    }
    else
        throw ci::app::AssetLoadExc( vertexRelPath ); // not necessary correct!    
}
void AssetManager::load( const ci::fs::path &vertexRelPath, const ci::fs::path &fragmentRelPath, const ci::fs::path &geomRelPath, std::function<void(ci::DataSourceRef,ci::DataSourceRef,ci::DataSourceRef)> callback )
{
    if( ci::fs::exists( ci::app::getAssetPath( vertexRelPath ) ) && ci::fs::exists( ci::app::getAssetPath( fragmentRelPath ) ) && ci::fs::exists( ci::app::getAssetPath( geomRelPath ) ) ){
        XAssetRef xAsset = XAssetRef( new XAssetTriple( vertexRelPath, fragmentRelPath, geomRelPath, callback ) );
        instance()->mAssets.push_back( xAsset );
        xAsset->notify();
    }
    else
        throw ci::app::AssetLoadExc( vertexRelPath ); // not necessary correct!    
}

AssetManager* AssetManager::instance()
{
    if( !mInstance ){
        mInstance = new AssetManager();
        ci::app::App::get()->getSignalUpdate().connect( std::bind( &AssetManager::update, mInstance ) );
    }
    return mInstance;
}

void AssetManager::update()
{
    for( std::deque<XAssetRef>::iterator it = mAssets.begin(); it != mAssets.end(); ++it ){
        (*it)->refresh();
    }
}

AssetManager* AssetManager::mInstance;