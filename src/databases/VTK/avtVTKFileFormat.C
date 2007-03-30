// ************************************************************************* //
//                            avtVTKFileFormat.C                             //
// ************************************************************************* //

#include <avtVTKFileFormat.h>

#include <vtkDataSet.h>
#include <vtkDataSetReader.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>


//
// Define the static const's
//

const char   *avtVTKFileFormat::MESHNAME="mesh";
const char   *avtVTKFileFormat::VARNAME="VTKVar";


// ****************************************************************************
//  Method: avtVTKFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2001
//
// ****************************************************************************

avtVTKFileFormat::avtVTKFileFormat(const char *fname) 
    : avtSTSDFileFormat(fname)
{
    dataset = NULL;
    readInDataset = false;
}


// ****************************************************************************
//  Method: avtVTKFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
// ****************************************************************************

avtVTKFileFormat::~avtVTKFileFormat()
{
    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }
}


// ****************************************************************************
//  Method: avtVTKFileFormat::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
// ****************************************************************************

void
avtVTKFileFormat::ReadInDataset(void)
{
    debug4 << "Reading in dataset from VTK file " << filename << endl;

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
    vtkDataSetReader *reader = vtkDataSetReader::New();
    reader->SetFileName(filename);
    dataset = reader->GetOutput();
    dataset->Register(NULL);

    //
    // Force the read and make sure that the reader is really gone, so we don't
    // eat up too many file descriptors.
    //
    dataset->Update();
    dataset->SetSource(NULL);
    reader->Delete();

    readInDataset = true;
}


// ****************************************************************************
//  Method: avtVTKFileFormat::GetMesh
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
//
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
// ****************************************************************************

vtkDataSet *
avtVTKFileFormat::GetMesh(const char *mesh)
{
    debug5 << "Getting mesh from VTK file " << filename << endl;

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    if (!readInDataset)
    {
        ReadInDataset();
    }

    //
    // The routine that calls this method is going to assume that it can call
    // Delete on what is returned.  That means we better add an extra
    // reference.
    //
    dataset->Register(NULL);
    return dataset;
}


// ****************************************************************************
//  Method: avtVTKFileFormat::GetVar
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
//
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
// ****************************************************************************

vtkDataArray *
avtVTKFileFormat::GetVar(const char *var)
{
    debug5 << "Getting var from VTK file " << filename << endl;

    if (!readInDataset)
    {
        ReadInDataset();
    }

    vtkDataArray *rv = NULL;
    rv = dataset->GetPointData()->GetArray(var);
    if (rv == NULL)
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

    if (rv == NULL)
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
//  Method: avtVTKFileFormat::GetVectorVar
//
//  Purpose:
//      Gets the vector variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as VTK vectors.
//
//  Programmer: Hank Childs
//  Creation:   March 20, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002 
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Aug 15 09:17:14 PDT 2002
//    Route the vector call through the scalar variable call, since there is
//    now no effective difference between the two.
//
// ****************************************************************************

vtkDataArray *
avtVTKFileFormat::GetVectorVar(const char *var)
{
    //
    // There is no difference between vectors and scalars for this class.
    //
    return GetVar(var);
}


// ****************************************************************************
//  Method: avtVTKFileFormat::FreeUpResources
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
// ****************************************************************************

void
avtVTKFileFormat::FreeUpResources(void)
{
    debug4 << "VTK file " << filename << " forced to free up resources."
           << endl;

    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }

    readInDataset = false;
}


// ****************************************************************************
//  Method: avtVTKFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
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
// ****************************************************************************

void
avtVTKFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int  i;

    if (! readInDataset)
    {
        ReadInDataset();
    }

    int spat = 3;
    int topo = 3;

    avtMeshType type;
    int  vtkType = dataset->GetDataObjectType();
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

    float bounds[6];
    dataset->GetBounds(bounds);

    if (bounds[4] == bounds[5])
    {
        spat = 2;
        topo = 2;
    }
 
    //
    // See if we have a point mesh.
    //
    if (vtkType == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) dataset;
        vtkUnsignedCharArray *types = vtkUnsignedCharArray::New();
        ugrid->GetListOfUniqueCellTypes(types);

        if (types->GetNumberOfTuples() == 1)
        {
            int myType = (int) types->GetValue(0);
            if (myType == VTK_VERTEX)
            {
                debug5 << "The VTK file format contains all points -- "
                       << "declaring this a point mesh." << endl;
                topo = 0;
            }
        }

        types->Delete();
    }
    else if (vtkType == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *) dataset;
        if (pd->GetNumberOfPolys() == 0 && pd->GetNumberOfStrips() == 0)
        {
            if (pd->GetNumberOfLines() > 0)
            {
                topo = 1;
            }
            else
            {
                topo = 0;
            }
        }
    }
 

    AddMeshToMetaData(md, MESHNAME, type, bounds, 1, 0, spat, topo);

    int nvars = 0;
    for (i = 0 ; i < dataset->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = dataset->GetPointData()->GetArray(i);
        int ncomp = arr->GetNumberOfComponents();
        const char *name = arr->GetName();
        char buffer[1024];
        if (name == NULL || strcmp(name, "") == 0)
        {
            sprintf(buffer, "%s%d", VARNAME, nvars);
            name = buffer;
        }
        if (ncomp == 1)
        {
            AddScalarVarToMetaData(md, name, MESHNAME, AVT_NODECENT);
        }
        else
        {
            AddVectorVarToMetaData(md, name, MESHNAME, AVT_NODECENT, ncomp);
        }
        nvars++;
    }
    for (i = 0 ; i < dataset->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = dataset->GetCellData()->GetArray(i);
        int ncomp = arr->GetNumberOfComponents();
        const char *name = arr->GetName();
        char buffer[1024];
        if (name == NULL || strcmp(name, "") == 0)
        {
            sprintf(buffer, "%s%d", VARNAME, nvars);
            name = buffer;
        }
        if (ncomp == 1)
        {
            AddScalarVarToMetaData(md, name, MESHNAME, AVT_ZONECENT);
        }
        else
        {
            AddVectorVarToMetaData(md, name, MESHNAME, AVT_ZONECENT, ncomp);
        }
        nvars++;
    }
}


