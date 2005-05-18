// ************************************************************************* //
//                            avtSTMDFileFormat.C                            //
// ************************************************************************* //

#include <avtSTMDFileFormat.h>

#include <ImproperUseException.h>
#include <InvalidFilesException.h>

// Initialize the static data
const int avtSTMDFileFormat::MAX_FILES = 20;

// ****************************************************************************
//  Method: avtSTMDFileFormat constructor
//
//  Arguments:
//      names    The file names.
//      nNames   The number of names.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 20 14:15:45 PDT 2001
//    Initialized timestep.
//
//    Hank Childs, Thu Oct 18 14:21:20 PDT 2001
//    Initialized nTimesteps.
//
//    Eric Brugger, Fri Dec  7 13:54:43 PST 2001
//    I modified the class to remove the oldest open file from the open
//    file cache when it overflowed.
//
// ****************************************************************************

avtSTMDFileFormat::avtSTMDFileFormat(const char * const *names, int nNames)
{
    nFiles = 0;
    mostRecentAdded = -1;
    timestep   = 0;
    nTimesteps = 1;
    filenames = new char*[MAX_FILES];
    int  i;
    for (i = 0 ; i < nNames; i++)
    {
        AddFile(names[i]);
    }
}


// ****************************************************************************
//  Method: avtSTMDFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
// ****************************************************************************

avtSTMDFileFormat::~avtSTMDFileFormat()
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
//  Method: avtSTMDFileFormat::AddFile
//
//  Purpose:
//      Adds a file to the list of files this manages.
//
//  Arguments:
//      fname     The name of the new file.
//
//  Programmer:   Hank Childs
//  Creation:     February 26, 2001
//
//  Modifications:
//    Eric Brugger, Fri Dec  7 13:54:43 PST 2001
//    I modified the class to remove the oldest open file from the open
//    file cache when it overflowed.
//
// ****************************************************************************

int
avtSTMDFileFormat::AddFile(const char *fname)
{
    //
    // If the cache of open files is exceeded then remove one of the files
    // from the cache, otherwise just add the file to the cache.
    //
    if (nFiles >= MAX_FILES)
    {
        //
        // Determine the file to replace in the cache using a round robin
        // approach.  Do not replace file 0 since it is the root file.
        //
        mostRecentAdded++;
        if (mostRecentAdded >= MAX_FILES)
        {
            mostRecentAdded = 1;
        }

        //
        // Remove the file from the cache.
        //
        CloseFile(mostRecentAdded);
        delete [] filenames[mostRecentAdded];
        filenames[mostRecentAdded] = NULL;
    }
    else
    {
        //
        // Use the next empty slot in the cache.
        //
        mostRecentAdded++;
        nFiles++;
    }

    //
    // Add the file to the cache.
    //
    filenames[mostRecentAdded] = new char[strlen(fname)+1];
    strcpy(filenames[mostRecentAdded], fname);

    //
    // Return the index of the file we just added.
    //
    return mostRecentAdded;
}


// ****************************************************************************
//  Method: avtSTMDFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      <unnamed>  The variable of interest.
//      <unnamed>  The domain of interest.
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
avtSTMDFileFormat::GetAuxiliaryData(const char *, int, const char *, void *,
                                    DestructorFunction &)
{
    //
    // This is defined only so the simple file formats that have no auxiliary
    // data don't have to define this.
    //
    return NULL;
}


// ****************************************************************************
//  Method: avtSTMDFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable.  This is defined in the base class to throw
//      an exception, so formats that don't have vectors don't have to worry
//      about this.
//
//  Arguments:
//      <unnamed>    The domain of the vector variable.
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
avtSTMDFileFormat::GetVectorVar(int, const char *)
{
    EXCEPTION0(ImproperUseException);
}



