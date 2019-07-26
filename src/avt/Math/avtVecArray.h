// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_VECARRAY_H
#define AVT_VECARRAY_H

#include <avtVec.h>
#include <cstddef>
#include <iterator>
#include <vector>

// ****************************************************************************
//  Modifications:
//
//    Tom Fogal, Thu Aug 21 15:23:16 EDT 2008
//    Added some const qualifications to member functions.
//
// ****************************************************************************
template<typename Ref>
class avtVecArrayIterator
{
public:
    
    typedef std::random_access_iterator_tag iterator_category;
    
    typedef Ref           value_type;
    typedef Ref           reference;
    typedef Ref*          pointer;
    typedef ptrdiff_t     difference_type;
    
    avtVecArrayIterator()
    {
    }
    
    avtVecArrayIterator( const double* current, unsigned int dim ) :
        _cur( const_cast<double*>(current) ), _dim(dim)
    {
    }
    
    reference operator*() const
    {
        return reference( _cur, _dim );
    }
    
    avtVecArrayIterator& operator++() // preincrement
    {
        _cur += _dim;
        return *this;
    }
    
    avtVecArrayIterator& operator--() // predecrement
    {
        _cur -= _dim;
        return *this;
    }
    
    avtVecArrayIterator operator++(int) // postincrement
    {
        avtVecArrayIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    avtVecArrayIterator operator--(int) // postdecrement
    {
        avtVecArrayIterator tmp = *this;
        --(*this);
        return tmp;
    }
    
    avtVecArrayIterator& operator+=( difference_type n )
    {
        _cur += _dim * n;
        return *this;
    }
    
    avtVecArrayIterator& operator-=( difference_type n )
    {
        _cur -= _dim * n;
        return *this;
    }
    
    bool operator==( const avtVecArrayIterator& other ) const
    {
        return _cur == other._cur;
    }

    bool operator!=( const avtVecArrayIterator& other ) const
    {
        return _cur != other._cur;
    }
    
    bool operator<( const avtVecArrayIterator& other ) const
    {
        return _cur < other._cur;
    }
    
    bool operator>( const avtVecArrayIterator& other ) const
    {
        return _cur > other._cur;
    }
    
    bool operator>=( const avtVecArrayIterator& other ) const
    {
        return _cur >= other._cur;
    }
    
    bool operator<=( const avtVecArrayIterator& other ) const
    {
        return _cur <= other._cur;
    }
    
    difference_type operator-( const avtVecArrayIterator& other ) const
    {
        return (_cur - other._cur)/_dim;
    }
    
    avtVecArrayIterator operator-( difference_type n ) const
    {
        return VecArrayIterator( *this ) -= n;
    }

    avtVecArrayIterator operator+( difference_type n ) const
    {
        return avtVecArrayIterator( *this ) += n;
    }
    
    // allow conversion from non-const to const iterator
    operator avtVecArrayIterator<const Ref>()
    {
        return avtVecArrayIterator<const Ref>( _cur, _dim );
    }
    
private:
    
    double*      _cur;
    unsigned int _dim;
};

// ****************************************************************************
//
//   Modifications:
//     Kathleen Bonnell, Aug 7 08:14:22 PDT 2008
//     Moved constructors and destructor to .C file so that symbols are 
//     exported on windows.  
//     Added return for operator=.
//
// ****************************************************************************

class MATH_API avtVecArray
{
public:
    
    typedef avtVecRef                            value_type;
    typedef avtVecRef                            reference;
    typedef const avtVecRef                      const_reference;
    typedef double*                              pointer;
    typedef const double*                        const_pointer;
    typedef avtVecArrayIterator<avtVecRef>       iterator;
    typedef avtVecArrayIterator<const avtVecRef> const_iterator;

    typedef size_t                               size_type;
    typedef ptrdiff_t                            difference_type;

    // constructors
    avtVecArray();
    
    avtVecArray( size_type n, const avtVecRef& value );
    
    avtVecArray( unsigned int dim, size_type n );
    
    avtVecArray( const avtVecArray& other ); 
    
    // destructor 
    ~avtVecArray();
    
    // assignment operator
    //  Modifications:
    //    Jeremy Meredith, Thu Aug  7 14:38:59 EDT 2008
    //    Added missing return statement.
    //
    avtVecArray& operator=( const avtVecArray& other )
    {
        _data = other._data;
        _dim  = other._dim;
        return *this;
    }

    // iterators
    
    iterator begin() { return iterator( &*_data.begin(), dim() ); }
    iterator end()   { return iterator( &*_data.end(),   dim() ); }
    
    const_iterator begin() const { return const_iterator( &*_data.begin(), dim() ); }
    const_iterator end()   const { return const_iterator( &*_data.end(),   dim() ); }
    
    // raw access
    
    pointer       data()       { return &*_data.begin(); }
    const_pointer data() const { return &*_data.begin(); }
    
    // resize
    void resize( size_type n, const avtVecRef& v )
    {
        size_type old = size();
        _data.resize( n*dim() );
        
        std::fill( begin() + old, end(), v );
    }

    void resize( size_type n )
    {
        _data.resize( n*dim() );
    }

    void resize( unsigned int dim, size_type n )
    {
        _dim = dim;
        _data.resize( dim*n );
    }

    bool empty() const
    {
        return _data.empty();
    }

    size_type capacity() const
    {
        return _data.capacity() / dim();
    }

    size_type size() const
    {
        return _data.size() / dim();
    }

    unsigned int dim() const
    {
        return _dim;
    }

    void reserve( size_type n )
    {
        _data.reserve( n*dim() );
    }
    
    // element access
    
    reference       operator[]( size_type n )       { return *(begin()+n); }
    const_reference operator[]( size_type n ) const { return *(begin()+n); }

    reference       front()       { return *begin(); }
    const_reference front() const { return *begin(); }

    reference       back()       { return *(--end()); }
    const_reference back() const { return *(--end()); }
    
    // modifiers
    
    void push_back( const value_type& v )
    {
        _data.resize( _data.size() + dim() );
        back() = v;
    }
    
    void pop_back()
    {
        _data.erase( _data.end()-dim(), _data.end() );
    }
    
    template<typename InputIterator>
    void append( InputIterator begin, InputIterator end )
    {
        size_type n = std::distance( begin, end );
        size_type s = size();
        
        resize( size() + n );

        std::copy( begin, end, this->begin()+s );
    }
    
protected:
    
    std::vector<double> _data;
    unsigned int        _dim;    
};

#endif // AVT_VECARRAY_H
