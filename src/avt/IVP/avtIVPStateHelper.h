// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtIVPStateHelper.h                             //
// ************************************************************************* //

#ifndef AVT_IVPSTATEHELPER_H
#define AVT_IVPSTATEHELPER_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>
#include <avtVector.h>

// ****************************************************************************
//  Struct: avtIVPStateHelper
//
//  Purpose:
//      A module that helps store state for the avtIVPState.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

struct IVP_API avtIVPStateHelper
{
public:
    enum Mode
    {
        GET,
        PUT
    };
    
                avtIVPStateHelper( Mode mode, void* data )
                             { Reset( mode, data ); };

    void        Reset( Mode mode, void* data )
                             { _size = 0; _mode = mode; _data = data; };

    template<typename T>
    avtIVPStateHelper& Accept( const T& t )
    {
        return Accept( &t, 1 );        
    }
    
    template<typename T>
    avtIVPStateHelper& Accept( const T* t, size_t size )
    {
        if( _data )
        {
            switch( _mode )
            {
            case GET:
                memcpy( (char*)_data+_size, (void*)t, size*sizeof(T) );
                break;
            case PUT:
                memcpy( (void*)t, (char*)_data + _size, size*sizeof(T) );
                break;
            }
        }
        
        _size += sizeof(T)*size;    
        return *this;
    }
    
    avtIVPStateHelper& Accept(avtVector &v)
    {
        return Accept(v.x).Accept(v.y).Accept(v.z);
        return *this;
        return *this;
    }
    /*
    avtIVPStateHelper& Accept( avtVecArray& va )
    {
        size_t dim  = va.dim();
        size_t size = va.size();
        
        Accept( dim ).Accept( size );
        
        if( _mode == PUT )
            va = avtVecArray( dim, size );
            
        return Accept( va.data(), size*dim );
    }
    */
    size_t size() const
    {
        return _size;
    }
    
private:
 
    Mode    _mode;
    void*   _data;
    size_t  _size;
};


#endif // AVT_IVPSTATEHELPER_H


