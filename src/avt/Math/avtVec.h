// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_VEC_H
#define AVT_VEC_H

#include <algorithm>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <numeric>
#include <stdexcept>
#include <math.h>
#include <stdlib.h>

#include <math_exports.h>

#include <ImproperUseException.h>

class avtVecRef;
inline std::ostream& operator<<( std::ostream& out, const avtVecRef& v );

//*****************************************************************************
//  Class avtVecRef
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:20:17 PDT 2008
//    Added return for operator=.
//
//*****************************************************************************

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
        _data( data ), _dim(n)
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
    
    //  Modifications:
    //    Jeremy Meredith, Thu Aug  7 14:38:59 EDT 2008
    //    Added missing return statement.
    //
    avtVecRef& operator=( const double& value )
    {
        std::fill( begin(), end(), value );
        return *this;
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

//*****************************************************************************
//  Class avtVec
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:20:17 PDT 2008
//    Changed for-loops to use size_t to eliminate signed/unsigned int 
//    comparison warnings.  Cast 'fabs' arg in call to std::transform.
//
//    Dave Pugmire, Wed Aug 13 10:58:32 EDT 2008
//    Added distSqrVecVec and distVecVec functions.
//
//    Tom Fogal, Wed Apr 29 19:34:01 MDT 2009
//    . Create a 1-elem array if needed in the copy ctor; `new x[0]' is
//      invalid.
//    . Add a parameter to the data & size constructor to detail how many
//      elements to copy from the source array.  The source array is allowed
//      (and is, in some code -- see pad()) to be smaller than the destination
//      array.
//
//*****************************************************************************

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

    avtVec( const_pointer data, const size_type& n, const size_type& ncopy ) :
        avtVecRef( new double[n], n )
    {
        std::copy( data, data+ncopy, begin() );
    }

    avtVec( const avtVecRef& other ) : 
        avtVecRef( new double[other.dim() ? other.dim() : 1],
                   other.dim() ? other.dim() : 1 )
    {
        std::copy( other.begin(), other.end(), begin() );
    }

    avtVec( const avtVec& other ) : 
        avtVecRef( new double[other.dim() > 0 ? other.dim() : 1],
                   other.dim() > 0 ? other.dim() : 1 )
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
    
    for( size_t i=0; i<v.dim(); ++i )
        result[i] = s*v[i];
        
    return result;
}

inline avtVec operator*( const avtVecRef& v, const double& s )
{
    avtVec result( v.dim() );
    
    for( size_t i=0; i<v.dim(); ++i )
        result[i] = v[i]*s;
        
    return result;
}

inline avtVec operator/( const avtVecRef& v, const double& s )
{
    avtVec result( v.dim() );
    
    for( size_t i=0; i<v.dim(); ++i )
        result[i] = v[i]/s;
        
    return result;
}

inline avtVec operator/( const double& s, const avtVecRef& v )
{
    avtVec result( v.dim() );
    
    for( size_t i=0; i<v.dim(); ++i )
        result[i] = s/v[i];
        
    return result;
}

// binary vector-vector operators

inline avtVec operator+( const avtVec& v0, const avtVec& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( size_t i=0; i<v0.dim(); ++i )
        result[i] = v0[i] + v1[i];
        
    return result;
}

inline avtVec operator-( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( size_t i=0; i<v0.dim(); ++i )
        result[i] = v0[i] - v1[i];
        
    return result;
}

inline avtVec operator*( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( size_t i=0; i<v0.dim(); ++i )
        result[i] = v0[i] * v1[i];
        
    return result;
}

inline avtVec operator/( const avtVecRef& v0, const avtVecRef& v1 )
{
    if( v0.dim() != v1.dim() )
        throw avtVec::DimensionsMismatch();
    
    avtVec result( v0.dim() );
    
    for( size_t i=0; i<v0.dim(); ++i )
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

    for( size_t i=0; i<v0.dim(); ++i )
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
    std::transform(v.begin(), v.end(), result.begin(), (double(*)(double))fabs);
    
    return result;
}

inline double norm_inf( const avtVecRef& v )
{
    return max( abs(v) );
}

// Modifications:
//
//   Tom Fogal, Wed Apr 29 19:35:57 MDT 2009
//   Use new size parameter to detail size of source array.
//
inline avtVec pad( const avtVecRef& v, const double& a )
{
    avtVec result( v.begin(), v.dim()+1, v.dim() );
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

inline double distSqrVecVec( const avtVecRef &u, const avtVecRef &v )
{
    if (u.dim() != v.dim())
        EXCEPTION0(ImproperUseException);
    double len = 0;
    
    for (size_t i = 0; i < u.dim(); i++)
    {
        double diff = (u[i]-v[i]);
        len += diff*diff;
    }
    return len;
}

inline double distVecVec( const avtVecRef &u, const avtVecRef &v )
{
    return sqrt(distSqrVecVec(u,v));
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
