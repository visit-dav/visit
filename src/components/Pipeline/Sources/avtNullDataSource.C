// ************************************************************************* //
//                           avtNullDataSource.C                             //
// ************************************************************************* //

#include <avtNullDataSource.h>


// ****************************************************************************
//  Method: avtNullDataSource constructor
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
// ****************************************************************************

avtNullDataSource::avtNullDataSource()
{
    nullData = new avtNullData(this);
}


// ****************************************************************************
//  Method: avtNullDataSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNullDataSource::~avtNullDataSource()
{
    ;
}


// ****************************************************************************
//  Method: avtNullDataSource::GetOutput
//
//  Purpose:
//      Gets the output avtNullData as an avtDataObject.
//
//  Returns:    The source's null data, typed as a data object.
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
// ****************************************************************************

avtDataObject_p
avtNullDataSource::GetOutput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, nullData);

    return rv;
}


// ****************************************************************************
//  Method: avtNullDataSource::SetOutput
//
//  Purpose:
//      Sets the output to be the same as the argument.
//
//  Arguments:
//      nd     The new null data.
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
// ****************************************************************************

void
avtNullDataSource::SetOutput(avtNullData_p nd)
{
    ; // no-op 
}
