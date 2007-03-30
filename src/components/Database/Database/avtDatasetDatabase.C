// ************************************************************************* //
//                            avtDatasetDatabase.C                           //
// ************************************************************************* //

#include <avtDatasetDatabase.h>

#include <avtSourceFromDatabase.h>


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


