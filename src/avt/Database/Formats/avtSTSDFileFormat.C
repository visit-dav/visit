// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtSTSDFileFormat.C                            //
// ************************************************************************* //

#include <avtSTSDFileFormat.h>
#include <avtDatabaseMetaData.h>

#include <ImproperUseException.h>
#include <DBYieldedNoDataException.h>

#include <cstring>

// ****************************************************************************
//  Method: avtSTSDFileFormat constructor
//
//  Arguments:
//      name     The file name.
//
//  Programmer:  Hank Childs
//  Creation:    February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 20 14:15:45 PDT 2001
//    Added initialization of timestep, domain.
//
// ****************************************************************************

avtSTSDFileFormat::avtSTSDFileFormat(const char *name)
{
    filename = new char[strlen(name)+1];
    strcpy(filename, name);
    timestep = 0;
    domain   = 0;
}


// ****************************************************************************
//  Method: avtSTSDFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
// ****************************************************************************

avtSTSDFileFormat::~avtSTSDFileFormat()
{
    if (filename != NULL)
    {
        delete [] filename;
        filename = NULL;
    }
}


// ****************************************************************************
//  Method: avtSTSDFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      <unnamed>  The name of the variable.
//      <unnamed>  The type of auxiliary data.
//      <unnamed>  The arguments for that type.
//
//  Returns:    The auxiliary data.  Throws an exception if this is not a
//              supported data type.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
// ****************************************************************************

void *
avtSTSDFileFormat::GetAuxiliaryData(const char *, const char *, void *,
                                    DestructorFunction &)
{
    //
    // This is defined only so the simple file formats that have no auxiliary
    // data don't have to define this.
    //
    return NULL;
}


// ****************************************************************************
//  Method: avtSTSDFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable.  This is defined in the base class to throw
//      an exception, so formats that don't have vectors don't have to worry
//      about this.
//
//  Arguments:
//      <unnamed>    The name of the vector variable.
//
//  Returns:     Normally a vector var, this actually throws an exception.
//
//  Programmer:  Hank Childs
//  Creation:    March 19, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

vtkDataArray *
avtSTSDFileFormat::GetVectorVar(const char *)
{
    EXCEPTION0(ImproperUseException);
}

// ****************************************************************************
//  Method: avtSTSDFileFormat::SetDatabaseMetaData
//
//  Programmer:  Mark C. Miller
//  Creation:    28Oct10
//
//  Modifications:
//    Mark C. Miller, Mon Nov  1 12:19:02 PDT 2010
//    Remove strict mode test.
//
//    Mark C. Miller, Mon Nov  8 06:53:26 PST 2010
//    Predicate on whether this is a simulation or not.
// ****************************************************************************

void
avtSTSDFileFormat::SetDatabaseMetaData(avtDatabaseMetaData *md)
{
    metadata = md;
    PopulateDatabaseMetaData(metadata);
    if ((!metadata->GetIsSimulation()) && metadata->Empty())
    {
        EXCEPTION1(DBYieldedNoDataException, filename);
    }
}
