/***
 * Demonstrike Core
 */

#pragma once

template < class type > class SERVER_DECL Singleton
{
public:
    /// Constructor
    Singleton( ) {
        /// If you hit this assert, this singleton already exists -- you can't create another one!
        WPError( mSingleton == 0, typeid(type).name() );
        mSingleton = static_cast<type *>(this);
    }

    /// Destructor
    virtual ~Singleton( ) {
        mSingleton = 0;
    }

    HEARTHSTONE_INLINE static type & getSingleton( ) { WPError( mSingleton, typeid(type).name()); return *mSingleton; }
    HEARTHSTONE_INLINE static type * getSingletonPtr( ) { return mSingleton; }
protected:

    /// Singleton pointer, must be set to 0 prior to creating the object
    static type * mSingleton;
};

/// Should be placed in the appropriate .cpp file somewhere
#define initialiseSingleton( type ) template <> type * Singleton < type > :: mSingleton = 0

/// To be used as a replacement for initialiseSingleton( )
///  Creates a file-scoped Singleton object, to be retrieved with getSingleton
#define createFileSingleton( type ) initialiseSingleton( type ); type the##type
