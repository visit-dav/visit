// ************************************************************************* //
//                            avtMTSDFileFormat.C                            //
// ************************************************************************* //

#include <avtMTSDFileFormat.h>

#include <ImproperUseException.h>
#include <InvalidFilesException.h>


using     std::vector;

// Initialize static values.
const int avtMTSDFileFormat::MAX_FILES = 1000;


// ****************************************************************************
//  Method: avtMTSDFileFormat constructor
//
//  Arguments:
//      names    The file names.
//      nNames   The number of names.
//
//  Programmer:  Hank Childs
//  Creation:    October 8, 2001
//
// ****************************************************************************

avtMTSDFileFormat::avtMTSDFileFormat(const char * const *names, int nNames)
{
    nFiles = nNames;
    filenames = new char*[MAX_FILES];
    int  i;
    for (i = 0 ; i < nFiles ; i++)
    {
        filenames[i] = new char[strlen(names[i])+1];
        strcpy(filenames[i], names[i]);
    }
    for (i = nFiles ; i < MAX_FILES ; i++)
    {
        filenames[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtMTSDFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

avtMTSDFileFormat::~avtMTSDFileFormat()
{
    if (filenames != NULL)
    {
        for (int i = 0 ; i < nFiles ; i++)
        {
            if (filenames[i] != NULL)
            {
                delete [] filenames[i];
                filenames[i] = NULL;
            }
        }
        delete [] filenames;
        filenames = NULL;
    }
}


// ****************************************************************************
//  Method: avtMTSDFileFormat::AddFile
//
//  Purpose:
//      Adds a file to the list of files this manages.
//
//  Arguments:
//      fname     The name of the new file.
//
//  Programmer:   Hank Childs
//  Creation:     October 8, 2001
//
// ****************************************************************************

int
avtMTSDFileFormat::AddFile(const char *fname)
{
    if (nFiles + 1 >= MAX_FILES)
    {
        //
        // We have run out of room, flag the first file as being bad since it
        // is most likely the table of contents.
        //
        EXCEPTION1(InvalidFilesException, fname);
    }

    filenames[nFiles] = new char[strlen(fname)+1];
    strcpy(filenames[nFiles], fname);
    nFiles++;

    //
    // nFiles-1 is the index of the file we just added, so return that.
    //
    return (nFiles-1);
}


// ****************************************************************************
//  Method: avtMTSDFileFormat::GetCycles
//
//  Purpose:
//      Gets each of the cycle numbers.
//
//  Arguments:
//      cycles   A vector to store the cycles.
//
//  Programmer:  Hank Childs
//  Creation:    October 8, 2001
//
// ****************************************************************************

void
avtMTSDFileFormat::GetCycles(vector<int> &cycles)
{
    cycles.clear();
    cycles.push_back(0);
}

// ****************************************************************************
//  Method: avtMTSDFileFormat::GetTimes
//
//  Purpose:
//      Gets each of the times.
//
//  Arguments:
//      times   A vector to store the times.
//
//  Programmer:  Brad Whitlock
//  Creation:    Mon Oct 13 14:15:28 PST 2003
//
// ****************************************************************************

void
avtMTSDFileFormat::GetTimes(vector<double> &times)
{
    times.clear();
    times.push_back(0.);
}

// ****************************************************************************
//  Method: avtMTSDFileFormat::GetNTimesteps
//
//  Purpose:
//      Gets the number of timesteps.
//
//  Returns:     The number of timesteps.
//
//  Programmer:  Hank Childs
//  Creation:    October 8, 2001
//
// ****************************************************************************

int
avtMTSDFileFormat::GetNTimesteps(void)
{
    return 1;
}


// ****************************************************************************
//  Method: avtMTSDFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      <unnamed>  The variable of interest.
//      <unnamed>  The timestep of interest.
//      <unnamed>  The type of auxiliary data.
//      <unnamed>  The arguments for that type.
//
//  Returns:    The auxiliary data.  Throws an exception if this is not a
//              supported data type.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

void *
avtMTSDFileFormat::GetAuxiliaryData(const char *, int, const char *, void *,
                                    DestructorFunction &)
{
    //
    // This is defined only so the simple file formats that have no auxiliary
    // data don't have to define this.
    //
    return NULL;
}


// ****************************************************************************
//  Method: avtMTSDFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable.  This is defined in the base class to throw
//      an exception, so formats that don't have vectors don't have to worry
//      about this.
//
//  Arguments:
//      <unnamed>    The timestep of the vector variable.
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
avtMTSDFileFormat::GetVectorVar(int, const char *)
{
    EXCEPTION0(ImproperUseException);
}



