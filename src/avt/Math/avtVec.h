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

#ifndef AVT_VEC_H
#define AVT_VEC_H

#include <stdexcept>
#include <numeric>
#include <cmath>
#include <iosfwd>
#include <stdlib.h>

#include <math_exports.h>

#include <ImproperUseException.h>

class avtVecRef;
inline std::ostream& operator<<( std::ostream& out, const avtVecRef& v );

// -------------------------------------------------------------------------

class MATH_API avtVecRef
{
public:
    
    // STL interface
    typedef double         value_type;
    typedef double*        pointer;
    typedef const double*  const_pointer;
    typedef double&        reference;
    typedef const double&  const_reference;
    typedef double*        iterator;
    typedef const double*  const_iterator;
    typedef size_t         size_type;
        
    class DimensionsMismatch: public std::runtime_error
    {
    public:
        DimensionsMismatch() : std::runtime_error( "avtVecRef dimensions mismatch" )
        {
        }
    };

    // --- constructor ---

    avtVecRef( pointer data, const size_t& n ) : 
        _dim(n), _data( data )
    {
    }

    // --- assignment operator ---

    avtVecRef& operator=( const avtVecRef& other )
    {
        if( dim() != other.dim() )
            throw DimensionsMismatch();
        
        std::copy( other.begin(), other.end(), begin() );
        
        return *this;
    }
    
    avtVecRef& operator=( const double& value )
    {
        std::fill( begin(), end(), value );
    }
    
    // --- basic access ---
    
    const size_t& dim() const
    {
        return _dim;
    }

    pointer values() 
    {
        return _data;
    }

    const_pointer values() const
    {
        return _data;
    }

    // --- element access ---
    
    reference operator[]( const size_type& n )
    {
        return _data[n];
    }

    const_reference operator[]( const size_type& n ) const
    {
        return _data[n];
    }

    // --- iterators ---
    
    iterator begin() { return _data;   }
    iterator end()   { return _data+_dim; }

    const_iterator begin() const { return _data;   }
    const_iterator end()   const { return _data+_dim; }

    // --- arithmetic operators ---
 
    // unary vector-vector operators
    avtVecRef& operator+=( const avtVecRef& rhs )
    {
        if( dim() != rhs.dim() )
            throw DimensionsMismatch();

        const_iterator ri = rhs.begin();

        for( iterator i=begin(); i!=end(); ++i, ++ri )
            *i += *ri;
            
        return *this;
    }

    avtVecRef& operator+=( const double& rhs )
    {
        for( iterator i=begin(); i!=end(); ++i )
            *i += rhs;
            
        return *this;
    }

    avtVecRef& operator-=( const avtVecRef& rhs )
    {
        if( dim() != rhs.dim() )
            throw DimensionsMismatch();

        const_iterator ri = rhs.begin();

        for( iterator i=begin(); i!=end(); ++i, ++ri )
            *i -= *ri;
            
        return *this;
    }

    avtVecRef& operator-=( const double& rhs )
    {
        for( iterator i=begin(); i!=end(); ++i )
            *i -= rhs;
            
        return *this;
    }

    avtVecRef& operator*=( const avtVecRef& rhs )
    {
        if( dim() != rhs.dim() )
            throw DimensionsMismatch();

        const_iterator ri = rhs.begin();

        for( iterator i=begin(); i!=end(); ++i, ++ri )
            *i *= *ri;
            
        return *this;
    }

    avtVecRef& operator/=( const avtVecRef& rhs )
    {
        if( dim() != rhs.dim() )
            throw DimensionsMismatch();

        const_iterator ri = rhs.begin();

        for( iterator i=begin(); i!=end(); ++i, ++ri )
            *i /= *ri;
            
        return *this;
    }

    // unary scalar-vector operators

    avtVecRef& operator*=( const double& rhs )
    {
        for( iterator i=begin(); i!=end(); ++i )
            *i *= rhs;
            
        return *this;
    }

    avtVecRef& operator/=( const double& rhs )
    {
        for( iterator i=begin(); i!=end(); ++i )
            *i /= rhs;
            
        return *this;
    }

    // --- length, lengthSquared ----
    double length() const { return sqrt( length2() ); }
    double length2() const
    {
        double len2 = 0.0;
        for( const_iterator i=begin(); i!=end(); ++i )
            len2 += (*i * *i);
        return len2;
    }

protected:
    
    pointer   _data;
    size_type _dim;
};

// -------------------------------------------------------------------------

class avtVec: public avtVecRef
{
public:

    // --- constructors ---

    avtVec() : avtVecRef( 0, 0 )
    {
    }

    avtVec( const size_type& n ) : 
        avtVecRef( new double[n], n )
    {
        std::fill( begin(), end(), 0.0 );
    }

    avtVec( const_pointer data, const size_type& n ) : 
        avtVecRef( new double[n], n )
    {
        std::copy( data, data+n, begin() );
    }

    avtVec( const avtVecRef& other ) : 
        avtVecRef( new double[other.dim()], other.dim() )
    {
        std::copy( other.begin(), other.end(), begin() );
    }

    avtVec( const avtVec& other ) : 
        avtVecRef( new double[other.dim()], other.dim() )
    {
        std::copy( other.begin(), other.end(), begin() );
    }

    avtVec( const double& v0, const double& v1 ) : 
        avtVecRef( new double[2], 2 )
    {
        _data[0] = v0;
        _data[1] = v1;
    }

    avtVec( const double& v0, const double& v1, const double& v2 ) : 
        avtVecRef( new double[3], 3 )
    {
        _data[0] = v0;
        _data[1] = v1;
        _data[2] = v2;
    }

    avtVec( const double& v0, const double& v1,
                   const double& v2, const double& v3 ) : 
        avtVecRef( new double[4], 4 )
    {
        _data[0] = v0;
        _data[1] = v1;
        _data[2] = v2;
        _data[3] = v3;
    }

    avtVec( const double& v0, const double& v1,
                   const double& v2, const double& v3,
                   const double& v4 ) : 
        avtVecRef( new double[5], 5 )
    {
        _data[0] = v0;
        _data[1] = v1;
        _data[2] = v2;
        _data[3] = v3;
        _data[4] = v4;
    }

    // --- destructor ---
    
    ~avtVec()
    {
        delete[] _data;
    }

    // --- assignment operator ---

    avtVec& operator=( const avtVec& other )
    {
        if( other.dim() != _dim )
        {
            delete[] _data;
        
            _data = new double[other.dim()];
            _dim  = other.dim();
        }
        
        std::copy( other.begin(), other.end(), begin() );
        return *this;
    }
};

// -------------------------------------------------------------------------

// binary scalar-vector operators
inline avtVec operator*( const double& s, const avtVecRef& v )
{
    avtVec result( v.dim() );
    
    for( int i=0; i<v.dim(); ++i )
        result[i] = s*v[i];
        
    return result;
}

inline avtVec operator*( const avtVecRef& v, const double& s )
{
    avtVec result( v.dim() );
    
    for( int i=0; i<v.dim(); ++i )
        result[i] = v[i]*s;
        
    return result;
}

inline avtVec operator/( const avtVecRef& v, const double& s )
{
    avtVec result( v.dim() );
    
    for( int i=0; i<v.dim(); ++i )
        result[i] = v[i]/s;
        
    return result;
}

inline avtVec operator/( const double& s, const avtVecRef& v )
{
    avtVec result( v.dim() );
    
    for( int i=0; i<v.dim(); ++i )
        result[i] = s/v[i];
        
    return result;
}

// binary vector-vector operators

inline avtVec operator+( const avtVec& v0, const avtVec& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( int i=0; i<v0.dim(); ++i )
        result[i] = v0[i] + v1[i];
        
    return result;
}

inline avtVec operator-( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( int i=0; i<v0.dim(); ++i )
        result[i] = v0[i] - v1[i];
        
    return result;
}

inline avtVec operator*( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( int i=0; i<v0.dim(); ++i )
        result[i] = v0[i] * v1[i];
        
    return result;
}

inline avtVec operator/( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( int i=0; i<v0.dim(); ++i )
        result[i] = v0[i] / v1[i];
        
    return result;
}

// free functions

inline double sum( const avtVecRef& v )
{
    return std::accumulate( v.begin(), v.end(), 0.0, std::plus<double>() );
}

inline double product( const avtVecRef& v )
{
    return std::accumulate( v.begin(), v.end(), 1.0, std::multiplies<double>() );
}

inline double inner( const avtVec& v0, const avtVec& v1 )
{
    return sum( v0*v1 );
}

inline double cross2( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim()!=2 || v1.dim()!=2 )
        throw avtVec::DimensionsMismatch();
        
    return v0[0]*v1[1] - v0[1]*v1[0];
}

inline avtVec cross3( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim()!=3 || v1.dim()!=3 )
        throw avtVec::DimensionsMismatch();
        
    return avtVec( v0[1]*v1[2] - v0[2]*v1[1],
                         v0[2]*v1[0] - v0[0]*v1[2],
                         v0[0]*v1[1] - v0[1]*v1[0] );
}

inline avtVec lerp( const double& t, const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
        
    avtVec result( v0.dim() );

    for( int i=0; i<v0.dim(); ++i )
        result[i] = (1.0-t)*v0[i] + t*v1[i];
        
    return result;
}

inline double norm( const avtVecRef& v )
{
    return sqrt( inner(v,v) );
}

inline double min_index( const avtVecRef& v )
{
    return std::min_element( v.begin(), v.end() ) - v.begin();
}

inline double max_index( const avtVecRef& v )
{
    return std::max_element( v.begin(), v.end() ) - v.begin();
}

inline double min( const avtVecRef& v )
{
    return *std::min_element( v.begin(), v.end() );
}

inline double max( const avtVecRef& v )
{
    return *std::max_element( v.begin(), v.end() );
}

inline avtVec abs( const avtVecRef& v )
{
    avtVec result( v.dim() );
    std::transform( v.begin(), v.end(), result.begin(), fabs );
    
    return result;
}

inline double norm_inf( const avtVecRef& v )
{
    return max( abs(v) );
}

inline avtVec pad( const avtVecRef& v, const double& a )
{
    avtVec result( v.begin(), v.dim()+1 );
    result[v.dim()] = a;
    
    return result;
}

inline avtVec pad( const double& a, const avtVecRef& v )
{
    avtVec result( v.dim()+1 );
    result[0] = a;
    std::copy( v.begin(), v.end(), result.begin()+1 );
    
    return result;
}

inline const avtVecRef sub( const avtVecRef& v, unsigned int size )
{
    size_t dim = v.dim() - size;
    if (dim > 0)
        EXCEPTION0(ImproperUseException);

    return avtVecRef( (double*)v.begin(), dim );
}

inline const avtVecRef sub( unsigned int size, const avtVecRef& v )
{
    size_t dim = v.dim() - size;
    if (dim > 0)
        EXCEPTION0(ImproperUseException);
    
    return avtVecRef( (double*)v.begin()+size, dim );
}

// ostream operator
inline std::ostream& operator<<( std::ostream& out, const avtVecRef& v )
{
    out << "[ ";

    if( v.dim() )
    {
        avtVec::const_iterator i;
        
        for (i=v.begin(); i!=v.end()-1; ++i )
            out << *i << ", ";

        out << *i;
    }

    out << " ]";

    return out;
}

#endif // AVT_VEC_H
