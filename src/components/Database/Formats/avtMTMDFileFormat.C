// ************************************************************************* //
//                            avtMTMDFileFormat.C                            //
// ************************************************************************* //

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
//  Method: avtMTMDFileFormat::GetCycles
//
//  Purpose:
//      Gets each of the cycle numbers.
//
//  Arguments:
//      cycles   A vector to store the cycles.
//
//  Programmer:  Hank Childs
//  Creation:    April 4, 2003
//
// ****************************************************************************

void
avtMTMDFileFormat::GetCycles(vector<int> &cycles)
{
    cycles.clear();
    int nstep = GetNTimesteps();
    for (int i = 0 ; i < nstep ; i++)
    {
        cycles.push_back(i);
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



