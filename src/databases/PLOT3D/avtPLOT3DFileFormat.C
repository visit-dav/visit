// ************************************************************************* //
//                            avtPLOT3DFileFormat.C                          //
// ************************************************************************* //

#include <avtPLOT3DFileFormat.h>

#include <vector>
#include <string>

#include <vtkDataArray.h>
#include <vtkStructuredGrid.h>
#include <vtkVisItPLOT3DReader.h>

#include <avtDatabaseMetaData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtPLOT3DFileFormat constructor
//
//  Arguments:
//      fname    The name of the PLOT3D file.
//
//  Programmer:  Hank Childs
//  Creation:    May 3, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:37:59 PDT 2003
//    Do not read in the file in the constructor.
//
// ****************************************************************************

avtPLOT3DFileFormat::avtPLOT3DFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    char *s_file = NULL;
    char *x_file = NULL;
    if (strstr(fname, ".x") != NULL)
    {
        char soln_file[1024];
        char *q = strstr(fname, ".x");
        strncpy(soln_file, fname, q-fname);
        strcpy(soln_file + (q-fname), ".q");
        AddFile(soln_file);
        x_file = filenames[0];
        s_file = filenames[1];
    }
    else if (strstr(fname, ".q") != NULL)
    {
        char points_file[1024];
        char *x = strstr(fname, ".q");
        strncpy(points_file, fname, x-fname);
        strcpy(points_file + (x-fname), ".x");
        AddFile(points_file);
        x_file = filenames[1];
        s_file = filenames[0];
    }
    else
    {
        //
        // We cannot identify either file as a points file or a solution file,
        // so give up.
        //
        EXCEPTION0(ImproperUseException);
    }
    
    reader = vtkVisItPLOT3DReader::New();
    reader->SetXYZFileName(x_file);
    reader->SetQFileName(s_file);
    reader->SetFileFormat(VTK_WHOLE_MULTI_GRID_WITH_IBLANKING);
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
// ****************************************************************************

avtPLOT3DFileFormat::~avtPLOT3DFileFormat()
{
    reader->Delete();
    reader = NULL;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetMesh
//
//  Purpose:
//      Returns the grid associated with a domain number.
//
//  Arguments:
//      dom     The domain number.
//      name    The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002    
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
// ****************************************************************************

vtkDataSet *
avtPLOT3DFileFormat::GetMesh(int dom, const char *name)
{
    if (dom < 0 || dom >= reader->GetNumberOfGrids())
    {
        EXCEPTION2(BadIndexException, dom, reader->GetNumberOfGrids());
    }

    if (strcmp(name, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetGridNumber(dom);
    reader->SetScalarFunctionNumber(-1);
    reader->SetVectorFunctionNumber(-1);
    reader->GetOutput()->Update();
    vtkDataSet *rv = (vtkDataSet *) reader->GetOutput()->NewInstance();
    rv->ShallowCopy(reader->GetOutput());

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVar
//
//  Purpose:
//      Returns the variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 3, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002    
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVar(int dom, const char *name)
{
    int var = -1;
    reader->SetVectorFunctionNumber(-1);
    if (strcmp(name, "density") == 0)
    {
        var = 100;
    }
    else if (strcmp(name, "pressure") == 0)
    {
        var = 110;
    }
    else if (strcmp(name, "temperature") == 0)
    {
        var = 120;
    }
    else if (strcmp(name, "enthalpy") == 0)
    {
        var = 130;
    }
    else if (strcmp(name, "internal_energy") == 0)
    {
        var = 140;
    }
    else if (strcmp(name, "kinetic_energy") == 0)
    {
        var = 144;
    }
    else if (strcmp(name, "velocity_magnitude") == 0)
    {
        var = 153;
    }
    else if (strcmp(name, "stagnation_energy") == 0)
    {
        var = 163;
    }
    else if (strcmp(name, "entropy") == 0)
    {
        var = 170;
    }
    else if (strcmp(name, "swirl") == 0)
    {
        var = 184;
    }

    if (var < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetScalarFunctionNumber(var);
    reader->SetGridNumber(dom);
    reader->GetOutput()->Update();

    vtkDataArray *dat = reader->GetOutput()->GetPointData()->GetScalars();
    if (dat == NULL)
    {
        debug1 << "Internal error -- variable should be point data!" << endl;
        EXCEPTION0(ImproperUseException);
    }

    vtkDataArray *rv = dat->NewInstance();
    rv->DeepCopy(dat);
    rv->SetName(name);

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVectorVar
//
//  Purpose:
//      Returns the vector variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 6, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002    
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVectorVar(int dom, const char *name)
{
    int var = -1;
    reader->SetScalarFunctionNumber(-1);
    if (strcmp(name, "velocity") == 0)
    {
        var = 200;
    }
    else if (strcmp(name, "vorticity") == 0)
    {
        var = 201;
    }
    else if (strcmp(name, "momentum") == 0)
    {
        var = 202;
    }
    else if (strcmp(name, "pressure_gradient") == 0)
    {
        var = 210;
    }

    if (var < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetVectorFunctionNumber(var);
    reader->SetGridNumber(dom);
    reader->GetOutput()->Update();

    vtkDataArray *dat = reader->GetOutput()->GetPointData()->GetVectors();
    if (dat == NULL)
    {
        debug1 << "Internal error -- variable should be point data!" << endl;
        EXCEPTION0(ImproperUseException);
    }

    vtkDataArray *rv = dat->NewInstance();
    rv->DeepCopy(dat);
    rv->SetName(name);

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this PLOT3D file.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
// ****************************************************************************

void
avtPLOT3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;
    mesh->numBlocks = reader->GetNumberOfGrids();
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    const int NUM_SCALARS = 10;
    char *scalar_names[NUM_SCALARS] = { "density", "pressure", "temperature", 
         "enthalpy", "internal_energy", "kinetic_energy", "velocity_magnitude",
         "stagnation_energy", "entropy", "swirl" };

    int i;
    for (i = 0 ; i < NUM_SCALARS ; i++)
    {
        avtScalarMetaData *sd1 = new avtScalarMetaData;
        sd1->name = scalar_names[i];
        sd1->meshName = "mesh";
        sd1->centering = AVT_NODECENT;
        sd1->hasDataExtents = false;
        md->Add(sd1);
    }

    const int NUM_VECTORS = 4;
    char *vector_names[4] = { "velocity", "vorticity", "momentum",
                               "pressure_gradient" };
    for (i = 0 ; i < NUM_VECTORS ; i++)
    {
        avtVectorMetaData *vd1 = new avtVectorMetaData;
        vd1->name = vector_names[i];
        vd1->meshName = "mesh";
        vd1->centering = AVT_NODECENT;
        vd1->hasDataExtents = false;
        vd1->varDim = 3;
        md->Add(vd1);
    }
}


