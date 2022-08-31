// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// **************************************************************************
//   avtVTKFileReaderBase.C
// **************************************************************************

#include <avtVTKFileReaderBase.h>

#include <limits.h> // for INT_MAX
#include <float.h> // for DBL_MAX

#include <avtDatabaseMetaData.h>
#include <Expression.h>
#include <avtGhostData.h>
#include <avtMaterial.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetReader.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLRectilinearGridReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include <DebugStream.h>
#include <Expression.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <StringHelpers.h>

#include <vtkVisItUtility.h>

#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
  #define strcasecmp stricmp
#endif

using std::array;
using std::map;
using std::string;
using std::vector;

//
// Define the static const's
//
const char   *avtVTKFileReaderBase::MESHNAME="mesh";
const char   *avtVTKFileReaderBase::VARNAME="VTKVar";

int    avtVTKFileReaderBase::INVALID_CYCLE = -INT_MAX;
double avtVTKFileReaderBase::INVALID_TIME = -DBL_MAX;

namespace avtVTKFRB
{
// ****************************************************************************
//  Function: ConvertStructuredPointsToRGrid
//
//  Purpose:
//     Constructs a vtkRectilinearGrid from the passed vtkStructuredPoints.
//
//  Notes: The passed in dataset will be deleted.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 9, 2004
//
//  Modifications:
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Kathleen Biagas, Fri Nov  1 13:27:44 PDT 2013
//    Changed pieceOrigin from int to double to prevent truncating.
//
//    Kathleen Biagas, Thu Sep 14 13:45:00 PDT 2017
//    Take the quick copy-array route when pieceDims == wholeDims.
//
// ****************************************************************************

vtkDataSet *
ConvertStructuredPointsToRGrid(vtkStructuredPoints *inSP, const int *extents)
{
    int wholeDims[3];
    double spacing[3];
    double wholeOrigin[3];
    inSP->GetDimensions(wholeDims);
    inSP->GetSpacing(spacing);
    inSP->GetOrigin(wholeOrigin);

    int pieceDims[3];
    double pieceOrigin[3];
    bool pieceEqualWhole = false;
    if (extents == NULL)
    {
        pieceDims[0] = wholeDims[0];
        pieceDims[1] = wholeDims[1];
        pieceDims[2] = wholeDims[2];
        pieceOrigin[0] = wholeOrigin[0];
        pieceOrigin[1] = wholeOrigin[1];
        pieceOrigin[2] = wholeOrigin[2];
        pieceEqualWhole = true;
    }
    else
    {
        pieceDims[0] = extents[1] - extents[0] + 1;
        pieceDims[1] = extents[3] - extents[2] + 1;
        pieceDims[2] = extents[5] - extents[4] + 1;
        pieceOrigin[0] = wholeOrigin[0] + extents[0] * spacing[0];
        pieceOrigin[1] = wholeOrigin[1] + extents[2] * spacing[1];
        pieceOrigin[2] = wholeOrigin[2] + extents[4] * spacing[2];

        pieceEqualWhole = (wholeDims[0] == pieceDims[0]) &&
                          (wholeDims[1] == pieceDims[1]) &&
                          (wholeDims[2] == pieceDims[2]);
    }

    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfComponents(1);
    x->SetNumberOfTuples(pieceDims[0]);
    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfComponents(1);
    y->SetNumberOfTuples(pieceDims[1]);
    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfComponents(1);
    z->SetNumberOfTuples(pieceDims[2]);

    vtkRectilinearGrid *outRG = vtkRectilinearGrid::New();
    outRG->SetDimensions(pieceDims);
    outRG->SetXCoordinates(x);
    outRG->SetYCoordinates(y);
    outRG->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();

    int i;
    float *ptr = x->GetPointer(0);
    for (i = 0; i < pieceDims[0]; i++, ptr++)
        *ptr = pieceOrigin[0] + i * spacing[0];

    ptr = y->GetPointer(0);
    for (i = 0; i < pieceDims[1]; i++, ptr++)
        *ptr = pieceOrigin[1] + i * spacing[1];

    ptr = z->GetPointer(0);
    for (i = 0; i < pieceDims[2]; i++, ptr++)
        *ptr = pieceOrigin[2] + i * spacing[2];

    if (extents == NULL || pieceEqualWhole)
    {
        for (i = 0; i < inSP->GetPointData()->GetNumberOfArrays(); i++)
            outRG->GetPointData()->AddArray(inSP->GetPointData()->GetArray(i));

        for (i = 0; i < inSP->GetCellData()->GetNumberOfArrays(); i++)
            outRG->GetCellData()->AddArray(inSP->GetCellData()->GetArray(i));
    }
    else
    {
        for (i = 0; i < inSP->GetPointData()->GetNumberOfArrays(); i++)
        {
            vtkDataArray *in = inSP->GetPointData()->GetArray(i);

            vtkDataArray *out = vtkDataArray::CreateDataArray(in->GetDataType());
            out->SetName(in->GetName());
            out->SetNumberOfComponents(in->GetNumberOfComponents());

            unsigned long ntuples = pieceDims[0] * pieceDims[1] * pieceDims[2];
            out->SetNumberOfTuples(ntuples);

            vtkIdType outIndex = 0;
            vtkIdType nx  = wholeDims[0];
            vtkIdType nxy = wholeDims[0] * wholeDims[1];
            for (unsigned int iZ = extents[4]; iZ < (unsigned int)extents[5]; iZ++)
            {
                for (unsigned int iY = extents[2]; iY < (unsigned int)extents[3]; iY++)
                {
                    for (unsigned int iX = extents[0]; iX < (unsigned int)extents[1]; iX++)
                    {
                        vtkIdType inIndex = iZ * nxy + iY * nx + iX;
                        out->SetTuple(outIndex, inIndex, in);
                        outIndex++;
                    }
                }
            }
            outRG->GetPointData()->AddArray(out);
        }

        for (i = 0; i < inSP->GetCellData()->GetNumberOfArrays(); i++)
        {
            vtkDataArray *in = inSP->GetCellData()->GetArray(i);

            vtkDataArray *out = vtkDataArray::CreateDataArray(in->GetDataType());
            out->SetName(in->GetName());
            out->SetNumberOfComponents(in->GetNumberOfComponents());

            int pieceZonalDims[3];
            pieceZonalDims[0] = (pieceDims[0] <= 1) ? 1 : (pieceDims[0] - 1);
            pieceZonalDims[1] = (pieceDims[1] <= 1) ? 1 : (pieceDims[1] - 1);
            pieceZonalDims[2] = (pieceDims[2] <= 1) ? 1 : (pieceDims[2] - 1);
            out->SetNumberOfTuples(pieceZonalDims[0] *
                                   pieceZonalDims[1] *
                                   pieceZonalDims[2]);

            int wholeZonalDims[3];
            wholeZonalDims[0] = (wholeDims[0] <= 1) ? 1 : (wholeDims[0] - 1);
            wholeZonalDims[1] = (wholeDims[1] <= 1) ? 1 : (wholeDims[1] - 1);
            wholeZonalDims[2] = (wholeDims[2] <= 1) ? 1 : (wholeDims[2] - 1);
            vtkIdType outIndex = 0;
            vtkIdType nX  = wholeZonalDims[0];
            vtkIdType nXY = wholeZonalDims[0] * wholeZonalDims[1];
            int zoneExtents[6];
            zoneExtents[0] = extents[0];
            zoneExtents[1] = extents[0] + pieceZonalDims[0];
            zoneExtents[2] = extents[2];
            zoneExtents[3] = extents[2] + pieceZonalDims[1];
            zoneExtents[4] = extents[4];
            zoneExtents[5] = extents[4] + pieceZonalDims[2];
            for (int iZ = zoneExtents[4]; iZ < zoneExtents[5]; iZ++)
            {
                for (int iY = zoneExtents[2]; iY < zoneExtents[3]; iY++)
                {
                    for (int iX = zoneExtents[0]; iX < zoneExtents[1]; iX++)
                    {
                        vtkIdType inIndex = iZ * nXY + iY * nX + iX;
                        out->SetTuple(outIndex, inIndex, in);
                        outIndex++;
                    }
                }
            }
            outRG->GetCellData()->AddArray(out);
        }
    }

    inSP->Delete();
    return outRG;
}

// ****************************************************************************
//  Function: GetListOfUniqueCellTypes
//
//  Purpose:
//     Gets a list of the unique cell types.
//
//  Notes: This is done externally to the similar method in
//         vtkUnstructuredGrid, since that method is buggy and can get
//         into an infinite loop.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

void
GetListOfUniqueCellTypes(vtkUnstructuredGrid *ug, vtkUnsignedCharArray *uca)
{
    int  i;
    bool   haveCellType[256];
    for (i = 0 ; i < 256 ; i++)
        haveCellType[i] = false;

    int ncells = ug->GetNumberOfCells();
    for (i = 0 ; i < ncells ; i++)
        haveCellType[ug->GetCellType(i)] = true;

    int ntypes = 0;
    for (i = 0 ; i < 256 ; i++)
        if (haveCellType[i])
            ntypes++;

    uca->SetNumberOfTuples(ntypes);
    int idx = 0;
    for (i = 0 ; i < 256 ; i++)
        if (haveCellType[i])
        {
            uca->SetValue(idx++, i);
        }
}
} // end namespace avtVTKFRB


// ****************************************************************************
//  Method: avtVTKFileReaderBase constructor
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
// ****************************************************************************

avtVTKFileReaderBase::avtVTKFileReaderBase() : vtk_meshname(), matvarname()
{
    vtk_time = INVALID_TIME;
    vtk_cycle = INVALID_CYCLE;
}


// ****************************************************************************
//  Method: avtVTKFileReaderBase::FreeUpResources
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
// ****************************************************************************
void
avtVTKFileReaderBase::FreeUpResources(void)
{
    matvarname.clear();

    for(std::map<string, vtkRectilinearGrid *>::iterator pos = vtkCurves.begin();
        pos != vtkCurves.end(); ++pos)
    {
        pos->second->Delete();
    }
    vtkCurves.clear();
}

// ****************************************************************************
//  Method: avtVTKFileReaderBase destructor
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

avtVTKFileReaderBase::~avtVTKFileReaderBase()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtVTKFileReaderBase::ReadVTKDataset
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
// ****************************************************************************

vtkDataSet *
avtVTKFileReaderBase::ReadVTKDataset(const std::string &filename,
    const int *structExtents)
{
    debug4 << "Reading in VTK Dataset from file: " << filename << endl;

    // find the file extension
    string fileExtension = "none";
    size_t i = filename.rfind('.', filename.length());
    if (i != string::npos)
        fileExtension = filename.substr(i+1, filename.length() -i);

    vtkDataSet *dataset = nullptr;

    if (fileExtension == "vtk" || fileExtension == "none")
    {
        if (fileExtension == "none")
        {
            debug1 << "No extension given ... assuming legacy VTK format."
                   << endl;
        }

        //
        // Create a file reader and set our dataset to be its output.
        //
        vtkDataSetReader *reader = vtkDataSetReader::New();
        reader->ReadAllScalarsOn();
        reader->ReadAllVectorsOn();
        reader->ReadAllTensorsOn();
        reader->SetFileName(filename.c_str());
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (fileExtension == "vti")
    {
        vtkXMLImageDataReader *reader = vtkXMLImageDataReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (fileExtension == "vtr")
    {
        vtkXMLRectilinearGridReader *reader =
            vtkXMLRectilinearGridReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (fileExtension == "vts")
    {
        vtkXMLStructuredGridReader *reader =
            vtkXMLStructuredGridReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (fileExtension == "vtp")
    {
        vtkXMLPolyDataReader *reader = vtkXMLPolyDataReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (fileExtension == "vtu")
    {
        vtkXMLUnstructuredGridReader *reader =
            vtkXMLUnstructuredGridReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else
    {
        EXCEPTION2(InvalidFilesException, filename,
                   "could not match extension to a VTK file format type");
    }

    vtk_time = INVALID_TIME;
    if (dataset->GetFieldData()->GetArray("TIME") != 0)
    {
        vtk_time = dataset->GetFieldData()->GetArray("TIME")->GetTuple1(0);
    }
    vtk_cycle = INVALID_CYCLE;
    if (dataset->GetFieldData()->GetArray("CYCLE") != 0)
    {
        vtk_cycle = (int)dataset->GetFieldData()->GetArray("CYCLE")->GetTuple1(0);
    }
    vtk_meshname.clear();
    if (dataset->GetFieldData()->GetAbstractArray("MeshName") != 0)
    {
        vtkStringArray *mn = vtkStringArray::SafeDownCast(
            dataset->GetFieldData()->GetAbstractArray("MeshName"));
        if (mn)
            vtk_meshname = mn->GetValue(0);
    }
    vtk_exprs.ClearExpressions();
    if (dataset->GetFieldData()->GetAbstractArray("VisItExpressions") != 0)
    {
        vtkStringArray *ve = vtkStringArray::SafeDownCast(
            dataset->GetFieldData()->GetAbstractArray("VisItExpressions"));
        for (int i = 0; i < ve->GetNumberOfTuples(); i++)
        {
            std::vector<std::string> expr_substrs = StringHelpers::split(ve->GetValue(i),';');
            Expression::ExprType vtype = Expression::Unknown;

            if (expr_substrs.size() != 3)
            {
                debug2 << "Ignoring invalid VisItExpression entry at index " << i << endl;
                continue;
            }

            if (!strcasecmp(expr_substrs[1].c_str(),"curve"))
                vtype = Expression::CurveMeshVar;
            else if (!strcasecmp(expr_substrs[1].c_str(),"scalar"))
                vtype = Expression::ScalarMeshVar;
            else if (!strcasecmp(expr_substrs[1].c_str(),"vector"))
                vtype = Expression::VectorMeshVar;
            else if (!strcasecmp(expr_substrs[1].c_str(),"tensor"))
                vtype = Expression::TensorMeshVar;
            else if (!strcasecmp(expr_substrs[1].c_str(),"array"))
                vtype = Expression::ArrayMeshVar;
            else if (!strcasecmp(expr_substrs[1].c_str(),"material"))
                vtype = Expression::Material;
            else if (!strcasecmp(expr_substrs[1].c_str(),"species"))
                vtype = Expression::Species;

            Expression expr;
            expr.SetName(expr_substrs[0]);
            expr.SetType(vtype);
            expr.SetDefinition(expr_substrs[2]);

            vtk_exprs.AddExpressions(expr);
        }
    }

    if (dataset->GetDataObjectType() == VTK_STRUCTURED_POINTS ||
        dataset->GetDataObjectType() == VTK_IMAGE_DATA)
    {
        // The old dataset passed in will be deleted, a new one will be
        // returned.

        if((structExtents == nullptr)  &&
           dataset->GetDataObjectType() == VTK_IMAGE_DATA)
        {
          vtkImageData *img = vtkImageData::SafeDownCast(dataset);
          if(img)
          {
            int *ext  = img->GetExtent();
            dataset = avtVTKFRB::ConvertStructuredPointsToRGrid(
                          (vtkStructuredPoints*)dataset, ext);
          }
        }
        else
        {
            dataset = avtVTKFRB::ConvertStructuredPointsToRGrid(
                          (vtkStructuredPoints*)dataset,
                          structExtents);
        }
    }

    if(dataset->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(dataset);
        int dims[3];
        rgrid->GetDimensions(dims);
        if(dims[0] > 0 && dims[1] <= 1 && dims[2] <= 1)
        {
            // Make some curves from this dataset.
            CreateCurves(rgrid);
        }
    }

    // Convert vtkGhostType to avtGhostDataType
    // Rename the arrays stored in dataset->GetCellData() and dataset->GetPointData()

    vtkDataArray *zoneArray = dataset->GetCellData()->GetArray("vtkGhostType");
    if (zoneArray)
    {
        zoneArray->SetName("avtGhostZones");
        dataset->GetCellData()->AddArray(zoneArray);
    }

    vtkDataArray *nodeArray = dataset->GetPointData()->GetArray("vtkGhostType");
    if (nodeArray)
    {
        nodeArray->SetName("avtGhostNodes");
        dataset->GetPointData()->AddArray(nodeArray);
    }

    return dataset;
}


// ****************************************************************************
// Method: avtVTKFileReaderBase::CreateCurves
//
// Purpose:
//   Create curve datasets based on the input rectilinear grid.
//
// Arguments:
//   rgrid : The rectilinear grid from which to create curves.
//
// Returns:
//
// Note:       vtkCurves gets the new datasets.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 26 11:01:44 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
avtVTKFileReaderBase::CreateCurves(vtkRectilinearGrid *rgrid)
{
    vtkDataArray *xc = rgrid->GetXCoordinates();
    int nPts = xc->GetNumberOfTuples();

    for(int i = 0; i < rgrid->GetPointData()->GetNumberOfArrays(); ++i)
    {
        vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
        if(arr->GetNumberOfComponents() == 1)
        {
            vtkRectilinearGrid *curve = vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
            vtkDataArray *curve_xc = curve->GetXCoordinates();
            vtkFloatArray *curve_yc = vtkFloatArray::New();
            curve_yc->SetName(arr->GetName());
            curve_yc->SetNumberOfTuples(nPts);
            for(vtkIdType j = 0; j < nPts; ++j)
            {
                curve_xc->SetTuple1(j, xc->GetTuple1(j));
                curve_yc->SetTuple1(j, arr->GetTuple1(j));
            }
            curve->GetPointData()->SetScalars(curve_yc);
            curve_yc->Delete();

            vtkCurves[string("curve_") + string(arr->GetName())] = curve;
        }
    }

    for(int i = 0; i < rgrid->GetCellData()->GetNumberOfArrays(); ++i)
    {
        vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
        if(arr->GetNumberOfComponents() == 1)
        {
            vtkRectilinearGrid *curve = vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
            vtkDataArray *curve_xc = curve->GetXCoordinates();
            for(vtkIdType j = 0; j < nPts; ++j)
                curve_xc->SetTuple1(j, xc->GetTuple1(j));

            int nCells = nPts-1;
            int idx = 0;
            vtkFloatArray *curve_yc = vtkFloatArray::New();
            curve_yc->SetName(arr->GetName());
            curve_yc->SetNumberOfTuples(nPts);
            curve_yc->SetTuple1(idx++, arr->GetTuple1(0));
            for(vtkIdType j = 0; j < nCells-1; ++j)
                curve_yc->SetTuple1(idx++, (arr->GetTuple1(j) + arr->GetTuple1(j+1)) / 2.);
            curve_yc->SetTuple1(idx++, arr->GetTuple1(nCells-1));

            curve->GetPointData()->SetScalars(curve_yc);
            curve_yc->Delete();

            vtkCurves[string("curve_") + string(arr->GetName())] = curve;
        }
    }
}


// ****************************************************************************
//  Method: avtVTKFileReaderBase::GetAuxiliaryDataFromDataset
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
//    Kathleen Biagas, Fri June 24, 2022
//    Moved from avtVTKFileReader and renamed for use by derived classes.
//
// ****************************************************************************

void *
avtVTKFileReaderBase::GetAuxiliaryDataFromDataset(vtkDataSet *dataset,
    const char *var, const char *type, DestructorFunction &df)
{
    void *rv = NULL;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        // matvarname is only inited if we call: PopulateDatabaseMetaData().
        // If you have a series of vtk files using this variable will cause
        // a crash any time the time slider is changed (and treat all dbs
        // as time varying is off)

        if(matvarname.empty())
        {
            int ncellvars = dataset->GetCellData()->GetNumberOfArrays();
            for(int i=0; (i < ncellvars) && (matvarname.empty()); i++)
            {
                // we are looking for either "avtSubsets" or "material"
                if(strcmp(dataset->GetCellData()->GetArrayName(i), "avtSubsets") == 0)
                    matvarname = "avtSubsets";
                else if(strcmp(dataset->GetCellData()->GetArrayName(i), "material") == 0)
                    matvarname = "material";
            }
        }

        vtkIntArray *matarr = vtkIntArray::SafeDownCast(
            dataset->GetCellData()->GetArray(matvarname.c_str()));

        if(matarr)
        {
            // again, if we haven't called PopulateDatabaseMetaData().
            // this data will be bad ...
            if(matnos.size() == 0)
            {
                vtkIntArray  *iarr = NULL;
                // check for field data "MaterialIds" that can directly provide
                // us the proper set of material ids.
                vtkDataArray *mids_arr = dataset->GetFieldData()->GetArray("MaterialIds");
                if( mids_arr != NULL)
                    iarr = vtkIntArray::SafeDownCast(mids_arr);
                else
                    iarr = matarr;
                int *iptr = iarr->GetPointer(0);
                std::map<int, bool> valMap;
                int ntuples = iarr->GetNumberOfTuples();
                for (int j = 0; j < ntuples; j++)
                    valMap[iptr[j]] = true;
                std::map<int, bool>::const_iterator it;
                for (it = valMap.begin(); it != valMap.end(); it++)
                {
                    char tmpname[32];
                    snprintf(tmpname, sizeof(tmpname), "%d", it->first);
                    matnames.push_back(tmpname);
                    matnos.push_back(it->first);
                }
            }

            int ntuples = matarr->GetNumberOfTuples();
            int *matlist = matarr->GetPointer(0);

            int *matnostmp = new int[matnos.size()];
            char **matnamestmp = new char*[matnames.size()];
            for (size_t i = 0; i < matnos.size(); i++)
            {
                matnostmp[i] = matnos[i];
                matnamestmp[i] = (char*) matnames[i].c_str();
            }

            avtMaterial *mat = new avtMaterial(
                (int)matnos.size(), //silomat->nmat,
                     matnostmp,     //silomat->matnos,
                     matnamestmp,   //silomat->matnames,
                     1,             //silomat->ndims,
                     &ntuples,      //silomat->dims,
                     0,             //silomat->major_order,
                     matlist,       //silomat->matlist,
                     0,             //silomat->mixlen,
                     0,             //silomat->mix_mat,
                     0,             //silomat->mix_next,
                     0,             //silomat->mix_zone,
                     0              //silomat->mix_vf
                     );

            delete [] matnostmp;
            delete [] matnamestmp;
            //matarr->Delete();

            df = avtMaterial::Destruct;
            rv = mat;
        }
    }
    return rv;
}


// ****************************************************************************
//  Method: avtVTKFileReaderBase::GetMeshFromDataset
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
//    Kathleen Biagas, Fri June 24, 2022
//    Moved from avtVTKFileReader and renamed for use by derived classes.
//
// ****************************************************************************

vtkDataSet *
avtVTKFileReaderBase::GetMeshFromDataset(vtkDataSet *ds, const char *mesh)
{
    // performs some standard error checking before returning the dataset
    if (ds == nullptr || ds->GetNumberOfPoints() == 0)
    {
        return nullptr;
    }

    // If the requested mesh is a curve, return it.
    std::map<string, vtkRectilinearGrid *>::iterator pos = vtkCurves.find(mesh);
    if(pos != vtkCurves.end())
    {
        pos->second->Register(NULL);
        return pos->second;
    }

    if(vtk_meshname.empty())
    {
        if (strcmp(mesh, MESHNAME) != 0)
        {
            EXCEPTION1(InvalidVariableException, mesh);
        }
    }
    else
    {
        if (strcmp(mesh, vtk_meshname.c_str()) != 0)
        {
            EXCEPTION1(InvalidVariableException, mesh);
        }
    }

    //
    // The routine that calls this method is going to assume that it can call
    // Delete on what is returned.  That means we better add an extra
    // reference.
    //
    ds->Register(NULL);
    return ds;
}


// ****************************************************************************
//  Method: avtVTKFileReaderBase::GetVarFromDataset
//
//  Purpose:
//      Gets the variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The variable.
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
//    Kathleen Biagas, Fri June 24, 2022
//    Moved from avtVTKFileReader and renamed for use by derived classes.
//
// ****************************************************************************

vtkDataArray *
avtVTKFileReaderBase::GetVarFromDataset(vtkDataSet *dataset,
    const char *real_name)
{
    if(dataset == nullptr)
        return nullptr;
    const char *var = real_name;
    char buffer[1024];
    if (strncmp(var, "internal_var_", strlen("internal_var_")) == 0)
    {
        sprintf(buffer, "avt%s", var + strlen("internal_var_"));
        var = buffer;
    }

    vtkDataArray *rv = nullptr;
    rv = dataset->GetPointData()->GetArray(var);
    if (rv == nullptr)
    {
        rv = dataset->GetCellData()->GetArray(var);
    }

    //
    // See if we made up an artificial name for it.
    //
    if (strstr(var, VARNAME) != NULL)
    {
        const char *numstr = var + strlen(VARNAME);
        int num = atoi(numstr);
        int npointvars = dataset->GetPointData()->GetNumberOfArrays();
        if (num < npointvars)
        {
            rv = dataset->GetPointData()->GetArray(num);
        }
        else
        {
            rv = dataset->GetCellData()->GetArray(num-npointvars);
        }
    }

    if (rv == nullptr)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // The routine that calls this method is going to assume that it can call
    // Delete on what is returned.  That means we better add an extra
    // reference.
    //
    rv->Register(NULL);
    return rv;
}


// ****************************************************************************
//  Method: avtVTKFileReaderBase::FillMeshMetaData
//
//  Purpose:
//      Fill in the mesh related meta data based on passed in values.
//
//  Notes:
//      Derived from avtVTKFileReader::PopulateDAtabaseMetaData, moved
//      here and renamed to facilitate use by derived classes.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

void
avtVTKFileReaderBase::FillMeshMetaData(avtDatabaseMetaData *md, vtkDataSet *ds,
    const string &meshName,
    const int nGroups, const string &GroupPieceName,
    const vector<string> &GroupNames,
    const vector<int> &GroupIds,
    const int nBlocks, const string &BlockPieceName,
    const vector<string> &BlockNames)
{
    int spat = 3;
    int topo = 3;

    avtMeshType type;
    int  vtkType = ds->GetDataObjectType();
    switch (vtkType)
    {
      case VTK_RECTILINEAR_GRID:
        type = AVT_RECTILINEAR_MESH;
        break;
      case VTK_STRUCTURED_GRID:
        type = AVT_CURVILINEAR_MESH;
        break;
      case VTK_UNSTRUCTURED_GRID:
        type = AVT_UNSTRUCTURED_MESH;
        break;
      case VTK_POLY_DATA:
        topo = 2;
        type = AVT_SURFACE_MESH;
        break;
      default:
        debug1 << "Unable to identify mesh type " << vtkType << endl;
        type = AVT_UNKNOWN_MESH;
        break;
    }

    double bounds[6];
    ds->GetBounds(bounds);

    if ((bounds[4] == bounds[5]) && (bounds[5] == 0.))
    {
        spat = 2;
        topo = 2;
    }

    //
    // Some mesh types can have a lower topological dimension
    //
    if (vtkType == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) ds;

        if(ugrid->GetNumberOfPoints() > 0)
        {
            if (ugrid->GetNumberOfCells() == 0)
            {
                // no cells declared, assume  point mesh.
                debug5 << "The VTK file format contains all points -- "
                       << "declaring this a point mesh." << endl;
                type = AVT_POINT_MESH;
                topo = 0;
            }
            else
            {
                vtkUnsignedCharArray *types = vtkUnsignedCharArray::New();
                avtVTKFRB::GetListOfUniqueCellTypes(ugrid, types);

                if (types->GetNumberOfTuples() == 1)
                {
                    int myType = (int) types->GetValue(0);
                    if (myType == VTK_VERTEX)
                    {
                        debug5 << "The VTK file format contains all points -- "
                               << "declaring this a point mesh." << endl;
                        type = AVT_POINT_MESH;
                        topo = 0;
                    }
                    else if(myType == VTK_LINE)
                    {
                        debug5 << "The mesh contains all lines, set topo=1" << endl;
                        topo = 1;
                    }
                }
                types->Delete();
            }
        }
    }
    else if (vtkType == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) ds;
        int dims[3];
        sgrid->GetDimensions(dims);
        if ((dims[0] == 1 && dims[1] == 1) ||
            (dims[0] == 1 && dims[2] == 1) ||
            (dims[1] == 1 && dims[2] == 1))
        {
            topo = 1;
        }
        else if (dims[0] == 1 || dims[1] == 1 || dims[2] == 1)
        {
            topo = 2;
        }
    }
    else if (vtkType == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *) ds;
        if (pd->GetNumberOfPoints() > 0)
        {
            if (pd->GetNumberOfPolys() == 0 && pd->GetNumberOfStrips() == 0)
            {
                if (pd->GetNumberOfLines() > 0)
                {
                    topo = 1;
                }
                else
                {
                    debug3 << "The VTK file format contains all points -- "
                           << "declaring this a point mesh." << endl;
                    type = AVT_POINT_MESH;
                    topo = 0;
                }
            }
        }
    }

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = meshName;
    mesh->meshType = type;
    mesh->spatialDimension = spat;
    mesh->topologicalDimension = topo;
    if (nGroups > 0)
    {
        mesh->numGroups = nGroups;
        if (!GroupNames.empty())
            mesh->groupNames = GroupNames;
        if (!GroupPieceName.empty())
        {
            mesh->groupPieceName = GroupPieceName;
            mesh->groupTitle = GroupPieceName + string("s");
        }
        mesh->groupIds = GroupIds;
    }
    mesh->numBlocks = nBlocks;
    mesh->blockOrigin = 0;
    if (nBlocks == 1)
        mesh->SetExtents(bounds);
    else
    {
        if (!BlockPieceName.empty())
        {
            mesh->blockPieceName = BlockPieceName;
            mesh->blockTitle = BlockPieceName + string("s");
        }
        if (!BlockNames.empty() && (int)BlockNames.size() == nBlocks)
            mesh->blockNames = BlockNames;
    }
    if (ds->GetFieldData()->GetArray("MeshCoordType") != NULL)
    {
        avtMeshCoordType mct = (avtMeshCoordType)
            int(ds->GetFieldData()->GetArray("MeshCoordType")->
                                            GetComponent(0, 0));
        mesh->meshCoordType = mct;
        if (mct == AVT_RZ)
        {
            mesh->xLabel = "Z-Axis";
            mesh->yLabel = "R-Axis";
        }
        else if (mct == AVT_ZR)
        {
            mesh->xLabel = "R-Axis";
            mesh->yLabel = "Z-Axis";
        }
    }
    if (ds->GetFieldData()->GetArray("UnitCellVectors"))
    {
        vtkDataArray *ucv = ds->GetFieldData()->GetArray("UnitCellVectors");
        for (int j=0; j<3; j++)
        {
            for (int k=0; k<3; k++)
            {
                mesh->unitCellVectors[j*3+k] = ucv->GetComponent(j*3+k,0);
            }
        }
    }
    if (ds->GetCellData()->GetArray("avtGhostZones"))
    {
        mesh->containsGhostZones = AVT_HAS_GHOSTS;
        int ncells = ds->GetNumberOfCells();
        vtkUnsignedCharArray *arr =
          (vtkUnsignedCharArray *) ds->GetCellData()->GetArray("avtGhostZones");
        unsigned char *ptr = arr->GetPointer(0);
        unsigned char v = '\0';
        avtGhostData::AddGhostZoneType(v, ZONE_EXTERIOR_TO_PROBLEM);
        for (int i = 0 ; i < ncells ; i++)
            if (ptr[i] & v)
            {
                mesh->containsExteriorBoundaryGhosts = true;
                break;
            }
    }
    else
        mesh->containsGhostZones = AVT_NO_GHOSTS;

    md->Add(mesh);

    map<string, vtkRectilinearGrid *>::iterator pos;
    for(pos = vtkCurves.begin(); pos != vtkCurves.end(); ++pos)
    {
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = pos->first;
        curve->hasSpatialExtents = false;
        curve->hasDataExtents = false;
        md->Add(curve);
    }

    // Add expressions from the database
    for (int i = 0; i < vtk_exprs.GetNumExpressions(); i++)
        md->AddExpression(&vtk_exprs.GetExpressions(i));
}


// ****************************************************************************
//  Method: avtVTKFileReaderBase::FillVarsMetaData
//
//  Purpose:
//      Fill in the variable related meta data based on passed in values.
//
//  Notes:
//      Derived from avtVTKFileReader::PopulateDAtabaseMetaData, moved
//      here and renamed to facilitate use by derived classes.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

void
avtVTKFileReaderBase::FillMaterialMetaData(avtDatabaseMetaData *md,
    const std::string &meshName, const std::string &varName,
    vtkDataArray *arr, vtkDataArray *materialIds)
{
    std::map<int, bool> valMap;
    vtkIntArray  *iarr = NULL;
    // check for field data "MaterialIds" that can directly provide us
    // the proper set of material ids.
    vtkDataArray *mids_arr = nullptr;
    if(materialIds)
        iarr = vtkIntArray::SafeDownCast(materialIds);
    else
        iarr = vtkIntArray::SafeDownCast(arr);

    int *iptr = iarr->GetPointer(0);
    int ntuples = iarr->GetNumberOfTuples();
    for (int j = 0; j < ntuples; j++)
        valMap[iptr[j]] = true;

    std::map<int, bool>::const_iterator it;
    for (it = valMap.begin(); it != valMap.end(); it++)
    {
        string tmpname = std::to_string(it->first);
        matnames.push_back(tmpname);
        matnos.push_back(it->first);
    }

    avtMaterialMetaData *mmd = new avtMaterialMetaData("materials",
         meshName, int(valMap.size()), matnames);
    md->Add(mmd);

    matvarname = varName;
}

void
avtVTKFileReaderBase::FillSingleVarMetaData(avtDatabaseMetaData *md,
    const std::string &meshName, const std::string &varName,
    int ncomp, int dataType, avtCentering center)
{
    if (ncomp == 1)
    {
        bool ascii = (dataType == VTK_CHAR);
        AddScalarVarToMetaData(md, varName, meshName, center, NULL, ascii);
    }
    else if (ncomp <= 4)
    {
        AddVectorVarToMetaData(md, varName, meshName, center, ncomp);
    }
    else if (ncomp == 6)
    {
        AddSymmetricTensorVarToMetaData(md, varName, meshName, center);
    }
    else if (ncomp == 9)
    {
        AddTensorVarToMetaData(md, varName, meshName, center);
    }
    else
    {
        if(dataType == VTK_UNSIGNED_CHAR || dataType == VTK_CHAR)
        {
            md->Add(new avtLabelMetaData(varName, meshName, center));
        }
        else
        {
            AddArrayVarToMetaData(md, varName, ncomp, meshName, center);
            string baseName = varName + "/comp_";
            string baseDef = string("array_decompose(<") + varName + string(">, ");
            string exp_name;
            string exp_def;
            for(int c = 0; c < ncomp; ++c)
            {
                exp_name = baseName + std::to_string(c);
                exp_def = baseDef + std::to_string(c) + string(")");
                Expression e;
                e.SetType(Expression::ScalarMeshVar);
                e.SetName(exp_name);
                e.SetDefinition(exp_def);
                md->AddExpression(&e);
            }
        }
    }
}

void
avtVTKFileReaderBase::FillVarsMetaData(avtDatabaseMetaData *md,
    vtkDataSetAttributes *atts, const std::string &meshName,
    avtCentering center, vtkFieldData *fieldData)
{
    static int nvars=0;
    for (int i = 0 ; i < atts->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = atts->GetArray(i);
        int ncomp = arr->GetNumberOfComponents();
        char *tname = arr->GetName();
        char buffer[1024];
        if (tname == NULL || strcmp(tname, "") == 0)
        {
            sprintf(buffer, "%s%d", VARNAME, nvars);
            tname = buffer;
        }
        string name(tname);
        if (name.find("avt") == 0)
        {
            name = "internal_var_" + name.substr(3);
        }
        if (center == AVT_ZONECENT &&
           ((arr->GetDataType() == VTK_INT) && (ncomp == 1) &&
            ((name == "internal_var_Subsets")  || (name == "material"))))
        {
            vtkDataArray *mids = nullptr;
            if(fieldData)
                mids = fieldData->GetArray("MaterialIds");
            FillMaterialMetaData(md, meshName, name, arr, mids);
        }
        else
        {
            FillSingleVarMetaData(md, meshName, name, ncomp, arr->GetDataType(),
                                  center);
        }
    }
}


// ****************************************************************************
//  Method: avtFileFormat::AddScalarVarToMetaData
//
//  Purpose:
//      A convenience routine to add a scalar variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the scalar var to.
//      name      The name of the scalar variable.
//      mesh      The mesh the scalar var is defined on.
//      cent      The centering type - node vs cell.
//      extents   The extents of the scalar var. (optional)
//      treatAsASCII   Whether the var is 'ascii' (optional)
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 13 18:28:51 PDT 2005
//    Add optional bool 'treatAsASCII' arg.
//
// ****************************************************************************

void
avtVTKFileReaderBase::AddScalarVarToMetaData(avtDatabaseMetaData *md,
    string name, string mesh, avtCentering cent, const double *extents,
    const bool treatAsASCII)
{
    avtScalarMetaData *scalar = new avtScalarMetaData();
    scalar->name = name;
    scalar->meshName = mesh;
    scalar->centering = cent;
    if (extents != NULL)
    {
        scalar->hasDataExtents = true;
        scalar->SetExtents(extents);
    }
    else
    {
        scalar->hasDataExtents = false;
    }
    scalar->treatAsASCII = treatAsASCII;

    md->Add(scalar);
}


// ****************************************************************************
//  Method: avtFileFormat::AddVectorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a vector variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the vector var to.
//      name      The name of the vector variable.
//      mesh      The mesh the vector var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the vector variable. (optional = 3)
//      extents   The extents of the vector var. (optional)
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
// ****************************************************************************

void
avtVTKFileReaderBase::AddVectorVarToMetaData(avtDatabaseMetaData *md,
    string name, string mesh, avtCentering cent, int dim, const double *extents)
{
    avtVectorMetaData *vector = new avtVectorMetaData();
    vector->name = name;
    vector->meshName = mesh;
    vector->centering = cent;
    vector->varDim = dim;
    if (extents != NULL)
    {
        vector->hasDataExtents = true;
        vector->SetExtents(extents);
    }
    else
    {
        vector->hasDataExtents = false;
    }

    md->Add(vector);
}


// ****************************************************************************
//  Method: avtFileFormat::AddTensorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a tensor variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the tensor variable.
//      mesh      The mesh the tensor var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the tensor variable. (optional = 3)
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtVTKFileReaderBase::AddTensorVarToMetaData(avtDatabaseMetaData *md,
    string name, string mesh, avtCentering cent, int dim)
{
    avtTensorMetaData *tensor = new avtTensorMetaData();
    tensor->name = name;
    tensor->meshName = mesh;
    tensor->centering = cent;
    tensor->dim = dim;

    md->Add(tensor);
}


// ****************************************************************************
//  Method: avtFileFormat::AddSymmetricTensorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a symmetric tensor variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the tensor variable.
//      mesh      The mesh the tensor var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the tensor variable. (optional = 3)
//
//  Programmer: Kathleen Biagas
//  Creation:   August 31, 2022
//
// ****************************************************************************

void
avtVTKFileReaderBase::AddSymmetricTensorVarToMetaData(avtDatabaseMetaData *md,
    string name, string mesh, avtCentering cent, int dim)
{
    avtSymmetricTensorMetaData *tensor = new avtSymmetricTensorMetaData();
    tensor->name = name;
    tensor->meshName = mesh;
    tensor->centering = cent;
    tensor->dim = dim;

    md->Add(tensor);
}

// ****************************************************************************
//  Method: avtFileFormat::AddArrayVarToMetaData
//
//  Purpose:
//      A convenience routine to add a array variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the array variable.
//      ncomps    The number of components.
//      mesh      The mesh the array var is defined on.
//      cent      The centering type - node vs cell.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

void
avtVTKFileReaderBase::AddArrayVarToMetaData(avtDatabaseMetaData *md,
    string name, int ncomps, string mesh,avtCentering cent)
{
    avtArrayMetaData *st = new avtArrayMetaData();
    st->name = name;
    st->nVars = ncomps;
    st->compNames.resize(ncomps);
    for (int i = 0 ; i < ncomps ; i++)
    {
        char name[16];
        snprintf(name, 16, "comp%02d", i);
        st->compNames[i] = name;
    }
    st->meshName = mesh;
    st->centering = cent;

    md->Add(st);
}
