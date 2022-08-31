// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtVTKFileReader.C                             //
// ************************************************************************* //

#include <avtVTKFileReader.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <DebugStream.h>

#include <string>
#include <vector>

using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtVTKFileReader constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2001
//
//  Modifications:
//    Hank Childs, Tue May 24 12:05:52 PDT 2005
//    Added arguments.
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Initialized matvarname
//
//    Kathleen Bonnell, Thu Sep 22 15:37:13 PDT 2005
//    Save the file extension.
//
//    Kathleen Bonnell, Thu Jun 29 17:30:40 PDT 2006
//    Add vtk_time, to store time from the VTK file if it is available.
//
//    Hank Childs, Mon Jun 11 21:27:04 PDT 2007
//    Do not assume there is an extension.
//
//    Kathleen Bonnell, Wed Jul  9 17:48:21 PDT 2008
//    Add vtk_cycle, to store cycle from the VTK file if it is available.
//
//    Brad Whitlock, Tue May 11 11:13:29 PDT 2010
//    Search for file extension from the back of the filename in case
//    directories contain "." and terminate if we hit a path separator.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Kathleen Biagas, Thu Aug 13 17:29:21 PDT 2015
//    Add support for groups and block names.
//
//    Kathleen Biagas, Fri Aug 13 2021
//    pieceFileNames now a vector<string>.
//    pieceExtents now vector<array<int,6>>.
//
//    Kathleen Biagas, Fri Jun 24, 2022
//    Inherit from avtVTKFileReaderBase, to handle reading of single-block,
//    legacy and xml formatted vtk files. Other logic moved into new classes.
//
// ****************************************************************************

avtVTKFileReader::avtVTKFileReader(const char *fname,
    const DBOptionsAttributes *) : avtVTKFileReaderBase(), filename(fname)
{
    dataset = nullptr;
    haveReadDataset = false;
}


// ****************************************************************************
//  Method: avtVTKFileReader::FreeUpResources
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Mark C. Miller, Wed Jul  2 17:27:35 PDT 2014
//    Delete everything even VTK datasets read.
//
//    Kathleen Biagas, Fri Aug 13 2021
//    pieceFileNames now a vector<string>.
//    pieceExtents now vector<array<int,6>>.
//
//    Kathleen Biagas, Fri Jun 24, 2022
//    Removed all code for parallel/multiblock files.
//
// ****************************************************************************

void
avtVTKFileReader::FreeUpResources(void)
{
    debug4 << "VTK file " << filename << " forced to free up resources." << endl;

    if(dataset != nullptr)
    {
        dataset->Delete();
        dataset = nullptr;
    }
    haveReadDataset = false;

    avtVTKFileReaderBase::FreeUpResources();
}

// ****************************************************************************
//  Method: avtVTKFileReader destructor
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
//    Brad Whitlock, Wed Oct 26 11:03:14 PDT 2011
//    Delete curves in vtkCurves.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Burlen Loring, Fri Jul 11 11:19:36 PDT 2014
//    fix alloc-dealloc-mismatch (operator new [] vs free)
//
// ****************************************************************************

avtVTKFileReader::~avtVTKFileReader()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtVTKFileReader::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 12 15:52:01 PST 2004
//    Use VisIt's version of the reader, so that all variables can be read
//    into the dataset in one pass.
//
//    Kathleen Bonnell, Thu Mar 11 12:53:12 PST 2004
//    Convert StructuredPoints datasets into RectilinearGrids.
//
//    Kathleen Bonnell, Thu Sep 22 15:37:13 PDT 2005
//    Support vtk xml file formats.
//
//    Kathleen Bonnell, Wed May 17 14:03:29 PDT 2006
//    Remove call to SetSource(NULL), as it now removes information necessary
//    to the dataset.
//
//    Kathleen Bonnell, Thu Jun 29 17:30:40 PDT 2006
//    Retrieve TIME from FieldData if available.
//
//    Hank Childs, Mon Jun 11 21:27:04 PDT 2007
//    Add support for files with no extensions.
//
//    Kathleen Bonnell, Fri Feb 29 09:02:44 PST 2008
//    Support VTK_IMAGE_DATA (returned from vtkXMLImageDataReader), convert it
//    to rectilinear, as we do with StructuredPoints data.
//
//    Sean Ahern, Mon Mar 24 17:19:56 EDT 2008
//    Added better error checking to detect badly formatted VTK files.
//
//    Kathleen Bonnell, Wed Jul  9 18:13:20 PDT 2008
//    Retrieve CYCLE from FieldData if available.
//
//    Brad Whitlock, Wed Oct 26 11:04:50 PDT 2011
//    Create curves for 1D rectilinear grids.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Kathleen Biagas, Mon Jan 28 11:06:32 PST 2013
//    Remove calls to ds->Update.
//
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Mark C. Miller, Wed Jul  2 17:28:24 PDT 2014
//    Add duplicate node removal (special case). Controlling logic should
//    ensure it is rarely triggered.
//
//    Kathleen Biagas, Mon Dec 22 09:49:22 PST 2014
//    Moved logic for duplicate node removal into avtTransformManager, it
//    is now controlled by setting a global preference.
//
//    Kathleen Biagas, Fri Feb  6 06:00:16 PST 2015
//    Added ability for parsing 'MeshName' field data from vtk file.
//
//    Matt Larsen, Fri Mar 2 09:00:15 PST 2018
//    Getting image data extents correctly from vti files
//
//    Edward Rusu, Mon Oct 1 09:24:24 PST 2018
//    Added support for vtkGhostType.
//
//    Mark C. Miller, Mon Mar  9 19:53:06 PDT 2020
//    Add logic to support VisIt expressions as vtkStringArrays
//
//    Kathleen Biagas, Fri Aug 13 2021
//    Change debug message to correctly identify actual file being read in.
//    pieceFileNames now a vector<string>.
//    pieceExtents now vector<array<int,6>>.
//
//    Kathleen Biagas, Fri Jun 24, 2022
//    Call base class method for reading in single block vtk dataset. Logic
//    for  multi block/multi group moved into new classes.
//
// ****************************************************************************

void
avtVTKFileReader::ReadInDataset()
{
    debug4 << "Reading in dataset from VTK file " << filename << endl;

    if(dataset != nullptr)
        dataset->Delete();

    dataset = ReadVTKDataset(filename);
    dataset->Register(NULL);
    haveReadDataset = true;
}


// ****************************************************************************
//  Method: avtVTKFileReader::GetAuxiliaryData
//
//  Programmer: Mark C. Miller
//  Creation:   September 15, 2005
//
//  Modifications:
//    Hank Childs, Fri Feb 15 11:25:32 PST 2008
//    Fix memory leak.
//
//    Cyrus Harrison, Fri Jan  7 10:17:19 PST 2011
//    Determine the proper material variable name & material metadata
//    if PopulateDatabaseMetaData has not been called. This supports materials
//    in the case we multiple vtk files acting as separate timesteps.
//
//    Cyrus Harrison, Wed Nov 16 13:35:54 PST 2011
//    Use "MaterialIds" field data to help generate avtMaterials result object.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Kathleen Biagas, Mon Nov 20 13:04:51 PST 2017
//    Pass domain to the GetVar call when retrieving materials.
//
//    Kathleen Biagas, Fri Jun 24, 2022
//    Call base class method.
//
// ****************************************************************************

void *
avtVTKFileReader::GetAuxiliaryData(const char *var,
    const char *type, void *, DestructorFunction &df)
{
    if (!haveReadDataset)
    {
        ReadInDataset();
    }

    return GetAuxiliaryDataFromDataset(dataset, var, type, df);
}


// ****************************************************************************
//  Method: avtVTKFileReader::GetMesh
//
//  Purpose:
//      Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//      mesh     The desired meshname, this should be MESHNAME.
//
//  Returns:     The mesh as a VTK dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
//    Brad Whitlock, Wed Oct 26 11:08:31 PDT 2011
//    Return curves.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Kathleen Biagas, Fri Feb  6 06:06:24 PST 2015
//    Use meshname from file (vtk_meshname), if available.

//    Kathleen Biagas, Thu Apr  2 12:22:55 PDT 2015
//    Return NULL a dataset with 0 points is read in.
//
//    Kathleen Biagas, Fri Aug 13 2021
//    Change debug message to correctly identify actual file being read in.
//
//    Kathleen Biagas, Fri Jun 24, 2022
//    Call base class which handles vtkcurves and ensuring mesh is valid.
//
// ****************************************************************************

vtkDataSet *
avtVTKFileReader::GetMesh(const char *mesh)
{
    debug5 << "Getting mesh from VTK file: " << filename << endl;

    if (!haveReadDataset)
    {
        ReadInDataset();
    }
    vtkDataSet *meshDS = GetMeshFromDataset(dataset, mesh);
    meshDS->Register(NULL);
    return meshDS;
}


// ****************************************************************************
//  Method: avtVTKFileReader::GetVar
//
//  Purpose:
//      Gets the variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as VTK scalars.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Hank Childs, Tue Mar 20 09:23:26 PST 2001
//    Account for vector variable in error checking.
//
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Aug 15 09:07:38 PDT 2002
//    Add support for multiple variables.
//
//    Hank Childs, Sat Mar 19 11:57:19 PST 2005
//    Turn variables with name "internal_var_" back into "avt".
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Kathleen Biagas, Fri Aug 13 2021
//    Change debug message to correctly identify actual file being read in.
//
//    Kathleen Biagas, Fri Jun 24, 2022
//    Call base class method which handles internal vars and vars that were
//    given artificial names.
//
// ****************************************************************************

vtkDataArray *
avtVTKFileReader::GetVar(const char *real_name)
{
    debug5 << "Getting var from VTK file " << filename << endl;

    if (!haveReadDataset)
    {
        ReadInDataset();
    }
    vtkDataArray *var = GetVarFromDataset(dataset, real_name);
    var->Register(NULL);
    return var;
}


// ****************************************************************************
//  Method: avtVTKFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Hank Childs, Thu Mar 15 11:10:54 PST 2001
//    Better determine if the dataset is 2D or 3D.
//
//    Hank Childs, Tue Mar 20 09:13:30 PST 2001
//    Allow for vector variables.
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Hank Childs, Sun Jun 23 20:50:17 PDT 2002
//    Added support for point meshes.
//
//    Hank Childs, Sat Jun 29 23:08:51 PDT 2002
//    Add support for poly data meshes that are point meshes.
//
//    Hank Childs, Thu Aug 15 09:07:38 PDT 2002
//    Add support for multiple variables.
//
//    Hank Childs, Thu Aug 21 23:28:44 PDT 2003
//    Replace call to GetListOfUniqueCellTypes, which hangs in an infinite
//    loop if there are multiple types of cells.
//
//    Hank Childs, Wed Sep 24 08:02:08 PDT 2003
//    Add support for tensors.
//
//    Hank Childs, Thu Aug 26 08:32:09 PDT 2004
//    Only declare the mesh as 2D if Z=0 for all points.
//
//    Hank Childs, Sat Mar 19 11:57:19 PST 2005
//    Do not return variables with name "avt", since we may want to look
//    at these variables and the generic DB will throw them away.
//
//    Kathleen Bonnell, Wed Jul 13 18:27:05 PDT 2005
//    Specify whether or not scalar data should be treated as ascii.
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Added support for arrays representing materials
//
//    Brad Whitlock, Wed Nov 9 10:59:35 PDT 2005
//    Added support for color vectors (ncomps==4).
//
//    Kathleen Bonnell, Fri Feb  3 11:20:02 PST 2006
//    Added support for MeshCoordType (int in FieldData of dataset,
//    0 == XY, 1 == RZ, 2 == ZR).
//
//    Jeremy Meredith, Mon Aug 28 17:40:47 EDT 2006
//    Added support for unit cell vectors.
//
//    Jeremy Meredith, Thu Apr  2 16:08:16 EDT 2009
//    Added array variable support.
//
//    Brad Whitlock, Fri May 15 16:05:22 PDT 2009
//    I improved the array variable support, adding them for cell data and I
//    added expressions to extract their components. I also added support
//    label variables.
//
//    Jeremy Meredith, Mon Nov  9 13:03:18 EST 2009
//    Expand the test for lower topological dimensions to include
//    structured grids.
//
//    Hank Childs, Wed Sep 14 16:29:19 PDT 2011
//    Improve handling of ghost data.
//
//    Brad Whitlock, Wed Oct 26 11:12:17 PDT 2011
//    Add metadata for curves.
//
//    Cyrus Harrison, Wed Nov 16 13:35:54 PST 2011
//    Use "MaterialIds" field data to help generate materials metadata.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Mark C. Miller, Wed Jul  2 17:26:44 PDT 2014
//    FreeUpResources before leaving. This is to ensure mdserver and non-zero
//    mpi-ranks don't hang onto the VTK data read here solely for purposes
//    of populating md.
//
//    Kathleen Biagas, Fri Feb  6 06:00:16 PST 2015
//    Use 'MeshName' from file if provided (stored in vtk_meshname).
//
//    Kathleen Biagas, Thu Aug 13 17:29:21 PDT 2015
//    Add support for groups and block names.
//
//    Kathleen Biagas, Thu Nov 15 09:20:40 PST 2018
//    If unstructured grid has declared no cells (valid in xml verisons),
//    assume it is a point mesh and set topodim to 0.
//
//    Kathleen Biagas, Tue Sep 10 12:11:23 PDT 2019
//    Test UnstructedGrids and vtkPolyData for existence of Points before
//    determining if the topological dimension should be lowered. Lack of
//    points indicates an empty dataset.
//
//    Kathleen Biagas, Thu Oct 31 12:26:22 PDT 2019
//    Set mesh type to POINT_MESH when poly data contains only vertex cells.
//
//    Mark C. Miller, Mon Mar  9 19:53:47 PDT 2020
//    Add logic to define any expressions we found to metadata.
//
//    Kathleen Biagas, Fri August 13, 2021
//    Add call to ReadInDataset if pieceDataset[0] is NULL.
//
//    Kathleen Biagas, Fri June 24, 2022
//    Bulk of logic moved to new base class methods.
//
// ****************************************************************************

void
avtVTKFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!haveReadDataset)
    {
        ReadInDataset();
    }

    string useMeshName;
    if(vtk_meshname.empty())
    {
        useMeshName = MESHNAME;
    }
    else
    {
        useMeshName = vtk_meshname;
    }

    // send some dummy vars for non-used mesh meta data
    string empty;
    vector<string> vs;
    vector<int> vi;
    FillMeshMetaData(md, dataset, useMeshName, 0, empty, vs, vi, 1, empty, vs);

    // Fill in the variables
    FillVarsMetaData(md, dataset->GetPointData(), useMeshName, AVT_NODECENT);
    FillVarsMetaData(md, dataset->GetCellData(),  useMeshName, AVT_ZONECENT,
                     dataset->GetFieldData());

    // Don't hang on to all the data we've read. We might not even need it
    // if we're in mdserver or of on non-zero mpi-rank.
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtVTKFileReader::IsEmpty
//
//  Purpose:
//      Returns a flag indicating if the file contains an empty dataset.
//
//  Programmer: Eric Brugger
//  Creation:   June 20, 2017
//
//  Modifications:
//    Kathleen Biagas, Fri August 13, 2021
//    Add call to ReadInDataset if pieceDataset[0] is NULL.
//
// ****************************************************************************

bool
avtVTKFileReader::IsEmpty()
{
    if (!haveReadDataset)
    {
        ReadInDataset();
    }

    if (dataset->GetNumberOfCells() == 0 && dataset->GetNumberOfPoints() == 0)
    {
        FreeUpResources();
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtVTKFileReader::GetTime
//
//  Purpose: Return the time associated with this file
//
//  Programmer: Kathleen Bonnell
//  Creation:   Jun 29, 2006
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul  9 18:14:24 PDT 2008
//    Call ReadInDataset if not done already.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

double
avtVTKFileReader::GetTime()
{
    if (INVALID_TIME == vtk_time && !haveReadDataset)
        ReadInDataset();
    return vtk_time;
}


// ****************************************************************************
//  Method: avtVTKFileReader::GetCycle
//
//  Purpose: Return the cycle associated with this file
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 9, 2008
//
//  Modifications:
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

int
avtVTKFileReader::GetCycle()
{
    if (INVALID_CYCLE == vtk_cycle && !haveReadDataset)
        ReadInDataset();
    return vtk_cycle;
}

