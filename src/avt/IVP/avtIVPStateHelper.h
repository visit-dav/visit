/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtIVPStateHelper.h                             //
// ************************************************************************* //

#ifndef AVT_IVPSTATEHELPER_H
#define AVT_IVPSTATEHELPER_H

#include <avtVec.h>
#include <avtVecArray.h>
#include <avtIVPSolver.h>
#include <ivp_exports.h>

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
        PUT,
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
    
    avtIVPStateHelper& Accept( avtVec& v )
    {
        size_t dim = v.dim();
        Accept( dim );
        if( _mode == PUT )
            v = avtVec( dim );
        
        return Accept( v.begin(), v.dim() );
    }
    
    avtIVPStateHelper& Accept( avtVecArray& va )
    {
        size_t dim  = va.dim();
        size_t size = va.size();
        
        Accept( dim ).Accept( size );
        
        if( _mode == PUT )
            va = avtVecArray( dim, size );
            
        return Accept( va.data(), size*dim );
    }
    
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


