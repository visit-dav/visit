// ************************************************************************* //
//                            avtDatasetDatabase.C                           //
// ************************************************************************* //

#include <avtDatasetDatabase.h>

#include <avtSourceFromDatabase.h>


// ****************************************************************************
//  Method: avtDatasetDatabase constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetDatabase::avtDatasetDatabase()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetDatabase destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetDatabase::~avtDatasetDatabase()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetDatabase::CreateSource
//
//  Purpose:
//      Creates a source object that will be returned to the avtDatabase
//      (base class) free of type information.
//
//  Arguments:
//      var   The name of the variable for the source.
//      ts    The timestep associated with the source.
//
//  Returns:  A new source object.
//
//  Notes:    The base class will manage memory issues.
//
//  Programmer: Hank Childs
//  Creation:   August 17, 2001
//
// ****************************************************************************

avtDataObjectSource *
avtDatasetDatabase::CreateSource(const char *var, int ts)
{
    return new avtSourceFromDatabase(this, var, ts);
}


