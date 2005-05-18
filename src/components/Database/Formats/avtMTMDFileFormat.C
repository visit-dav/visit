// ************************************************************************* //
//                            avtMTMDFileFormat.C                            //
// ************************************************************************* //
#include <limits.h>

#include <snprintf.h>

#include <avtDatabaseMetaData.h>
#include <avtMTMDFileFormat.h>

#include <ImproperUseException.h>
#include <InvalidFilesException.h>


using     std::vector;


// ****************************************************************************
//  Method: avtMTMDFileFormat constructor
//
//  Arguments:
//      name     The file name.
//
//  Programmer:  Hank Childs
//  Creation:    April 4, 2003
//
// ****************************************************************************

avtMTMDFileFormat::avtMTMDFileFormat(const char *name)
{
    filename = new char[strlen(name)+1];
    strcpy(filename, name);
}


// ****************************************************************************
//  Method: avtMTMDFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

avtMTMDFileFormat::~avtMTMDFileFormat()
{
    if (filename != NULL)
    {
        delete [] filename;
        filename = NULL;
    }
}


// ****************************************************************************
//  Method: avtMTMDFileFormat::GetNTimesteps
//
//  Purpose:
//      Gets the number of timesteps.
//
//  Returns:     The number of timesteps.
//
//  Programmer:  Hank Childs
//  Creation:    April 4, 2003
//
// ****************************************************************************

int
avtMTMDFileFormat::GetNTimesteps(void)
{
    return 1;
}


// ****************************************************************************
//  Method: avtMTMDFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      <unnamed>  The variable of interest.
//      <unnamed>  The timestep of interest.
//      <unnamed>  The domain of interest.
//      <unnamed>  The type of auxiliary data.
//      <unnamed>  The arguments for that type.
//
//  Returns:    The auxiliary data.  Throws an exception if this is not a
//              supported data type.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

void *
avtMTMDFileFormat::GetAuxiliaryData(const char *, int, int, const char *,
                                    void *, DestructorFunction &)
{
    //
    // This is defined only so the simple file formats that have no auxiliary
    // data don't have to define this.
    //
    return NULL;
}


// ****************************************************************************
//  Method: avtMTMDFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable.  This is defined in the base class to throw
//      an exception, so formats that don't have vectors don't have to worry
//      about this.
//
//  Arguments:
//      <unnamed>    The timestep of the vector variable.
//      <unnamed>    The domain of the vector variable.
//      <unnamed>    The name of the vector variable.
//
//  Returns:     Normally a vector var, this actually throws an exception.
//
//  Programmer:  Hank Childs
//  Creation:    April 4, 2003
//
// ****************************************************************************

vtkDataArray *
avtMTMDFileFormat::GetVectorVar(int, int, const char *)
{
    EXCEPTION0(ImproperUseException);
}


// ****************************************************************************
//  Method: avtMTMDFileFormat::PopulateDatabaseMetaData
//
//  Purpose: Provide a default implementation for the non-time-qualified
//  request to populate metadata. We do this so the time-qualified version can
//  distinguish between our (bad) default implementation and a plugin's real
//  implementation.
//
//  Ordinarilly, we'd simply use pure virtual functions to enforce which
//  PopulateDatabaseMetaData method should be implemented. However, since
//  many plugins pre-date these changes, we decided NOT to use pure-virtual.
//
//  Programmer:  Mark C. Miller 
//  Creation:    April 7, 2005
//
// ****************************************************************************

void
avtMTMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    //
    // Do something bogus to the metadata that we would never expect a 
    // real plugin to do so that we can distinguish this method it later
    //
    md->SetNumStates(-INT_MAX);
}

// ****************************************************************************
//  Method: avtMTMDFileFormat::PopulateDatabaseMetaData
//
//  Purpose: Provide a default implementation for the time-qualified
//  request to populate metadata. This just turns around and calls the
//  non-time-qualified method. And, typically, that method has been overridden
//  by an MTXX plugin. However, it may not have been and we can catch that
//  error here, at least at run time, and do something about it.
//
//  Programmer:  Mark C. Miller 
//  Creation:    April 7, 2005
//
// ****************************************************************************
void
avtMTMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int ts)
{
    //
    // Most plugins pre-dating the existence of a time qualified request to 
    // populate database metadata implemented only this method.
    //
    PopulateDatabaseMetaData(md);

    //
    // If we're here, then a plugin has not overridden this method. However,
    // a plugin may have overriden PopulateDatabaseMetaData(md). On the other
    // hand we don't know for sure if that is indeed the case. So, we look
    // for the bogus change our default PopulateDatabaseMetaData(md) would have
    // resulted in and throw an exception if we see it.
    //
    if (md->GetNumStates() == -INT_MAX)
    {
        md->SetNumStates(0);
        char msg[512];
        SNPRINTF(msg, sizeof(msg), "It looks like the \"%s\" database plugin "
            "did not implement either of the PopulateDatabaseMetaData methods. "
            "Please contact the plugin developer. This error cannot be corrected "
            "without changes to code", GetType());
        EXCEPTION1(ImproperUseException, msg);
    }
}
