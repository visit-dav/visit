// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#include <avtVecArray.h>

avtVecArray::avtVecArray() : _dim(0)
{
}
    
avtVecArray::avtVecArray( size_type n, const avtVecRef& value )
        : _data( n*value.dim() ), _dim( value.dim() )
{
    std::fill( begin(), end(), value );
}
    
avtVecArray::avtVecArray( unsigned int dim, size_type n ) :
        _data( n*dim ), _dim(dim)
{
}
    
avtVecArray::avtVecArray( const avtVecArray& other ) : 
        _data( other._data ), _dim( other.dim() )
{
}
    
// destructor 
avtVecArray::~avtVecArray() 
{
}
    
