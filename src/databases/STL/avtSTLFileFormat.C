/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtSTLFileFormat.C                             //
// ************************************************************************* //

#include <avtSTLFileFormat.h>

#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkVisItSTLReader.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <avtDatabase.h>


//
// Define the static const's
//

const char   *avtSTLFileFormat::MESHNAME = "STL_mesh";


// ****************************************************************************
//  Method: avtSTLFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    May 24, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jan  7 12:20:01 EST 2010
//    Initialize checkedFile.
//
// ****************************************************************************

avtSTLFileFormat::avtSTLFileFormat(const char *fname) 
    : avtSTSDFileFormat(fname)
{
    dataset = NULL;
    readInDataset = false;
    checkedFile = false;
}


// ****************************************************************************
//  Method: avtSTLFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

avtSTLFileFormat::~avtSTLFileFormat()
{
    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }
}


// ****************************************************************************
//  Method: avtSTLFileFormat::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed May 17 14:03:29 PDT 2006
//    Remove call to SetSource(NULL), as it now removes information necessary
//    to the dataset.
//
//    Brad Whitlock, Thu Apr  2 16:15:31 PDT 2009
//    I renamed the STL reader to vtkVisItSTLReader to avoid crashes on Mac.
//
//    Jeremy Meredith, Thu Jan  7 11:28:44 EST 2010
//    Added new Strict option to reader, and enabled it in mdserver.
//
//    Jeremy Meredith, Fri Jan  8 16:34:36 EST 2010
//    Enable strict mode based on the new file format strict mode, not
//    blindly in the meta-data server.
//
// ****************************************************************************

void
avtSTLFileFormat::ReadInDataset(void)
{
    debug4 << "Reading in dataset from STL file " << filename << endl;

    //
    // This shouldn't ever happen (since we would already have the dataset
    // we are trying to read from the file sitting in memory), but anything
    // to prevent leaks.
    //
    if (dataset != NULL)
    {
        dataset->Delete();
    }

    //
    // Create a file reader and set our dataset to be its output.
    //
    vtkVisItSTLReader *reader = vtkVisItSTLReader::New();
    reader->SetStrict(GetStrictMode());
    reader->SetFileName(filename);
    dataset = reader->GetOutput();
    dataset->Register(NULL);

    //
    // Force the read and make sure that the reader is really gone, so we don't
    // eat up too many file descriptors.
    //
    dataset->Update();
    //dataset->SetSource(NULL);
    reader->Delete();

    readInDataset = true;
}


// ****************************************************************************
//  Method: avtSTLFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//      mesh     The desired meshname, this should be MESHNAME.
//
//  Returns:     The mesh as a STL dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jun 24 10:23:18 PDT 2004
//    Make sure to increment the reference count, since the caller will believe
//    that it owns the returned dataset.
//
// ****************************************************************************

vtkDataSet *
avtSTLFileFormat::GetMesh(const char *mesh)
{
    debug5 << "Getting mesh from STL file " << filename << endl;

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    if (!readInDataset)
    {
        ReadInDataset();
    }

    dataset->Register(NULL);
    return dataset;
}


// ****************************************************************************
//  Method: avtSTLFileFormat::GetVar
//
//  Purpose:
//      Gets the variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as STL scalars.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

vtkDataArray *
avtSTLFileFormat::GetVar(const char *var)
{
    EXCEPTION1(InvalidVariableException, var);
}


// ****************************************************************************
//  Method: avtSTLFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

void
avtSTLFileFormat::FreeUpResources(void)
{
    debug4 << "STL file " << filename 
           << " forced to free up resources." << endl;

    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }

    readInDataset = false;
}


// ****************************************************************************
//  Method: avtSTLFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar  7 09:21:53 PST 2003 
//    Do not bother reading in the dataset, since we learn nothing meaningful
//    from it anyway.  The VTK routines assume that the dataset being read
//    in is truly an STL file.
//
//    Jeremy Meredith, Thu Jan  7 11:29:01 EST 2010
//    Okay, we're reading it now despite the penalty.  It's necessary to
//    make sure this really is an STL file in the meta-data server.
//
// ****************************************************************************

void
avtSTLFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (avtDatabase::OnlyServeUpMetaData())
    {
        if (!checkedFile)
        {
            ReadInDataset();
            if (!dataset ||
                dataset->GetNumberOfCells() == 0 ||
                dataset->GetNumberOfPoints() == 0)
            {
                EXCEPTION2(InvalidFilesException,filename,
                           "Empty dataset assumed to be erroneous file."); 
            }
            dataset->Delete();
            dataset = NULL;
            readInDataset = false;
            checkedFile = true;
        }
    }

    int spat = 3;
    int topo = 2;

    avtMeshType type = AVT_SURFACE_MESH;;

    AddMeshToMetaData(md, MESHNAME, type, NULL, 1, 0, spat, topo);
}


