/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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



