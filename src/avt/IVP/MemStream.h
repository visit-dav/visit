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
//                                  MemStream.h                              //
// ************************************************************************* //

#ifndef MEM_STREAM_H
#define MEM_STREAM_H

#include <ivp_exports.h>
#include <vector>
#include <list>
#include <cstring>
#include <avtVec.h>
#include <avtVecArray.h>
#include <DebugStream.h>

// ****************************************************************************
//  Struct: MemStream
//
//  Purpose:
//      A helper class for serializing a streamline.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

struct IVP_API MemStream
{
  public:
    enum Mode {
        READ = 0,
        WRITE
    };
    
    MemStream( size_t sz0= 32 );
    MemStream( size_t sz, const unsigned char *buff );
    ~MemStream();

    void rewind() { pos = 0; }

    size_t buffLen() const { return len; }
    unsigned char *buff() const { return data; }

    // General read/write routines.
    template <typename T> void io( Mode mode, T *pt, size_t num ) 
                      { return (mode == READ ? read(pt,num) : write(pt,num)); }
    template <typename T> void io( Mode mode, T& t ) 
                      { size_t s=1; io( mode, &t, s ); }
    template <typename T> void io( Mode mode, std::vector<T> &v )
                      { return (mode == READ ? read(v) : write(v)); }
    void io( Mode mode, avtVec &v ) 
                      { return (mode == READ ? read(v) : write(v)); }
    void io( Mode mode, avtVecArray &va )  
                      { return (mode == READ ? read(va) : write(va)); }
    //template <typename T> void io( Mode mode, std::list<T*> &l )  
    //                { return (mode == READ ? read(l) : write(l)); }

    //Read from buffer.
    template <typename T> void read( T& t ) { read( &t, 1 ); }
    
    template <typename T> void read( T *pt, const size_t &num )
    {
        size_t nBytes = sizeof(T)*num;
        memcpy( pt, &data[pos], nBytes );
        pos += nBytes;
    }
    
    void read( avtVec &v )
    {
        size_t dim;
        read(dim);
        v = avtVec(dim);
        read(v.begin(), v.dim());
    }

    void read( avtVecArray &va )
    {
        size_t dim, size;
        read(dim);
        read(size);
        va = avtVecArray(dim,size);
        read(va.data(),size*dim);
    }

    template <typename T> void read( std::vector<T> &v )
    {
        size_t sz;
        read(sz);
        v.resize(sz);
        for ( size_t i = 0; i < sz; i++ )
            read(v[i]);
    }

    /*
    template <typename T> void read( std::list<T*> &l )
    {
        size_t sz;
        read(sz);
        
        debug1 << "read( list sz= "<<sz << " );\n";
        for ( size_t i = 0; i < sz; i++ )
        {
            T *x = new T;
            read(*x);
            debug1 << "x sz= "<<sizeof(x)<<endl;
            l.push_back(x);
        }
        
    }
    */

    //Write to buffer.
    template <typename T> void write( const T& t ) { write( &t, 1 ); }
    
    template <typename T> void write( const T *const pt, size_t num )
    {
        size_t nBytes = sizeof(T)*num;
        CheckSize( nBytes );
        memcpy(&data[pos], pt, nBytes );
        pos += nBytes;
        
        if ( pos > len )
            len = pos;
    }
    
    template <typename T> void write( const std::vector<T> &v )
    {
        write( v.size() );
        for ( size_t i = 0; i < v.size(); i++ )
            write( v[i] );
    }

    void write( const avtVec &v )
    {
        write( v.dim() );
        write( v.begin(), v.dim() );
    }
    
    void write( const avtVecArray &va )
    {
        write(va.dim());
        write(va.size());
        write(va.data(),va.size()*va.dim());
    }

    /*    
    template <typename T> void write( const std::list<T*> &l )
    {
        debug1 << "write( list sz= "<<l.size() << " sizeof = " << sizeof(T)<<" );\n";
        write( l.size() );
        typename list<T*>::const_iterator pos;
        for ( pos = l.begin(); pos != l.end(); pos++ )
        {
            T &x = *(*pos);
            write( x );
            debug1 << "write( pos sz= " << sizeof(x)<<")\n";
        }
    }
    */

  protected:
    void CheckSize( size_t sz );

    // data members
    unsigned char *data;
    size_t len, maxLen, pos;
};

#endif


