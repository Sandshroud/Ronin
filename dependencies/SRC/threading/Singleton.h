/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

    RONIN_INLINE static type & getSingleton( ) { WPError( mSingleton, typeid(type).name()); return *mSingleton; }
    RONIN_INLINE static type * getSingletonPtr( ) { return mSingleton; }
protected:

    /// Singleton pointer, must be set to 0 prior to creating the object
    static type * mSingleton;
};

/// Should be placed in the appropriate .cpp file somewhere
#define initialiseSingleton( type ) template <> type * Singleton < type > :: mSingleton = 0

/// To be used as a replacement for initialiseSingleton( )
///  Creates a file-scoped Singleton object, to be retrieved with getSingleton
#define createFileSingleton( type ) initialiseSingleton( type ); type the##type
