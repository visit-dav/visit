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
//                        avtWavefrontOBJFileFormat.C                        //
// ************************************************************************* //

#include <avtWavefrontOBJFileFormat.h>

#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkOBJReader.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>


//
// Define the static const's
//

const char   *avtWavefrontOBJFileFormat::MESHNAME = "OBJMesh";
const char   *avtWavefrontOBJFileFormat::VARNAME1 = "OBJVar1";
const char   *avtWavefrontOBJFileFormat::VARNAME2 = "OBJVar2";


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    February 14, 2002
//
// ****************************************************************************

avtWavefrontOBJFileFormat::avtWavefrontOBJFileFormat(const char *fname) 
    : avtSTSDFileFormat(fname)
{
    dataset = NULL;
    readInDataset = false;
    nVars = 0;
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
// ****************************************************************************

avtWavefrontOBJFileFormat::~avtWavefrontOBJFileFormat()
{
    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkTCoords has been deprecated in VTK 4.0, use vtkDataArray instead. 
//
//    Hank Childs, Tue May 28 08:50:58 PDT 2002
//    Don't assume that there are texture coordinates.
//
//    Kathleen Bonnell, Wed May 17 10:58:54 PDT 2006
//    Remove call to SetSource(NULL) as it now removes information necessary
//    for the dataset. 
//
// ****************************************************************************

void
avtWavefrontOBJFileFormat::ReadInDataset(void)
{
    debug4 << "Reading in dataset from WavefrontOBJ file " << filename << endl;

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
    vtkOBJReader *reader = vtkOBJReader::New();
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

    vtkDataArray *tcoords = dataset->GetPointData()->GetTCoords();
    if (tcoords != NULL)
    {
        nVars = tcoords->GetNumberOfComponents();
    }
    else
    {
        nVars = 0;
    }

    readInDataset = true;
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//      mesh     The desired meshname, this should be MESHNAME.
//
//  Returns:     The mesh as a WavefrontOBJ dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jul 23 10:55:03 PDT 2002
//    Account for the way that the data is handled when it is returned.
//
// ****************************************************************************

vtkDataSet *
avtWavefrontOBJFileFormat::GetMesh(const char *mesh)
{
    debug5 << "Getting mesh from WavefrontOBJ file " << filename << endl;

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    if (!readInDataset)
    {
        ReadInDataset();
    }

    //
    // The generic database will assume it owns this piece of memory, so add
    // a reference so our dataset won't be deleted out from under us.
    //
    dataset->Register(NULL);

    return dataset;
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::GetVar
//
//  Purpose:
//      Gets the variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as WavefrontOBJ scalars.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Mar 20 09:23:26 PST 2001
//    Account for vector variable in error checking.
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkTCoords and vtkScalars have been deprecated in VTK 4.0, 
//    use vtkDataArray and vtkFloatArray instead. 
//
// ****************************************************************************

vtkDataArray *
avtWavefrontOBJFileFormat::GetVar(const char *var)
{
    debug5 << "Getting var from WavefrontOBJ file " << filename << endl;

    if (!readInDataset)
    {
        ReadInDataset();
    }

    if (nVars <= 0 || 
        (strcmp(var, VARNAME1) != 0 && strcmp(var, VARNAME2) != 0))
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    vtkDataArray *tcoords = dataset->GetPointData()->GetTCoords();
    if (tcoords == NULL)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    vtkFloatArray *s1 = vtkFloatArray::New();
    int varId = (strcmp(var, VARNAME1) == 0 ? 0 : 1);
    int nPts = dataset->GetNumberOfPoints();
    s1->SetNumberOfTuples(nPts);
    for (int i = 0 ; i < nPts ; i++)
    {
        double *tuple = tcoords->GetTuple(i);
        s1->SetTuple1(i, tuple[varId]);
    }

    return s1;
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
// ****************************************************************************

void
avtWavefrontOBJFileFormat::FreeUpResources(void)
{
    debug4 << "WavefrontOBJ file " << filename 
           << " forced to free up resources." << endl;

    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }

    readInDataset = false;
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkTCoords has been deprecated in VTK 4.0, use vtkDataArray instead. 
//
// ****************************************************************************

void
avtWavefrontOBJFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (! readInDataset)
    {
        ReadInDataset();
    }

    int spat = 3;
    int topo = 2;

    avtMeshType type = AVT_SURFACE_MESH;;

    double bounds[6];
    dataset->GetBounds(bounds);

    if (bounds[4] == bounds[5])
    {
        spat = 2;
        topo = 2;
    }

    AddMeshToMetaData(md, MESHNAME, type, bounds, 1, 0, spat, topo);

    vtkDataArray *tcoords = dataset->GetPointData()->GetTCoords();
    if (tcoords != NULL)
    {
        nVars = tcoords->GetNumberOfComponents();
        if (nVars != 2)
        {
            EXCEPTION0(ImproperUseException);
        }
        AddScalarVarToMetaData(md, VARNAME1, MESHNAME, AVT_NODECENT);
        AddScalarVarToMetaData(md, VARNAME2, MESHNAME, AVT_NODECENT);
    }
}


