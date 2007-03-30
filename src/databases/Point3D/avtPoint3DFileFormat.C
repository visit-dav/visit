// ************************************************************************* //
//                             avtPoint3DFileFormat.C                        //
// ************************************************************************* //

#include <avtPoint3DFileFormat.h>

#include <fstream.h>

#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabaseMetaData.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


const char      *avtPoint3DFileFormat::MESHNAME = "points";


using std::string;
using std::vector;

// ****************************************************************************
//  Method: avtPoint3DFileFormat constructor
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:21:32 PDT 2003
//    Do not open files in the constructor.
//
// ****************************************************************************

avtPoint3DFileFormat::avtPoint3DFileFormat(const char *fname)
    : avtSTSDFileFormat(fname)
{
    haveReadData = false;
    column1 = NULL;
    column1 = NULL;
    column3 = NULL;
    column4 = NULL;
    points  = NULL;
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
// ****************************************************************************

avtPoint3DFileFormat::~avtPoint3DFileFormat()
{
    if (column1 != NULL)
    {
        column1->Delete();
        column1 = NULL;
    }
    if (column2 != NULL)
    {
        column2->Delete();
        column2 = NULL;
    }
    if (column3 != NULL)
    {
        column3->Delete();
        column3 = NULL;
    }
    if (column4 != NULL)
    {
        column4->Delete();
        column4 = NULL;
    }
    if (points != NULL)
    {
        points->Delete();
        points = NULL;
    }
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh (a bunch of 3D points).
//
//  Notes:   The calling function believes that it owns the returned memory.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
// ****************************************************************************

vtkDataSet *
avtPoint3DFileFormat::GetMesh(const char *mesh)
{
    if (!haveReadData)
    {
        ReadData();
    }

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    points->Register(NULL);
    return points;
}


// ****************************************************************************
//  Method; avtPoint3DFileFormat::GetVar
//
//  Purpose:
//      Gets a variable.
//
//  Notes:   The calling function believes that it owns the returned memory.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
// ****************************************************************************

vtkDataArray *
avtPoint3DFileFormat::GetVar(const char *var)
{
    if (!haveReadData)
    {
        ReadData();
    }

    vtkDataArray *rv = NULL;
    if (varnames[0] == var)
        rv = column1;
    if (varnames[1] == var)
        rv = column2;
    if (varnames[2] == var)
        rv = column3;
    if (varnames[3] == var)
        rv = column4;

    if (rv == NULL)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    rv->Register(NULL);
    return rv;
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Declares which variables are available for this file format.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:21:32 PDT 2003
//    Read in the variable names before populating the meta-data.
//
// ****************************************************************************

void
avtPoint3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!haveReadData)
    {
        ReadData();
    }

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = MESHNAME;
    mesh->meshType = AVT_POINT_MESH;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 0;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    for (int i = 0 ; i < 4 ; i++)
    {
        AddScalarVarToMetaData(md, varnames[i], MESHNAME, AVT_NODECENT, NULL);
    }
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat::ReadData
//
//  Purpose:
//      Does the work of actually reading in the data.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:21:32 PDT 2003
//    Read in the variable names as well.
//
//    Hank Childs, Wed May  7 08:45:08 PDT 2003 
//    We are consistently adding one bogus point at the end of the array.
//    Remove that.
//
// ****************************************************************************

void
avtPoint3DFileFormat::ReadData(void)
{
    int      i;

    ifstream ifile(filename);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, filename);
    }

    for (i = 0 ; i < 4 ; i++)
    {
        char buf[1024];
        ifile >> buf;
        varnames.push_back(buf);
    }

    char     line[1024];
    vector<float> var1;
    vector<float> var2;
    vector<float> var3;
    vector<float> var4;

    while (!ifile.eof())
    {
        ifile.getline(line, 1024);
        float a, b, c, d;
        sscanf(line, "%f %f %f %f", &a, &b, &c, &d);
        var1.push_back(a);
        var2.push_back(b);
        var3.push_back(c);
        var4.push_back(d);
    }

    int npts = var1.size();
    // The EOF logic always seems to get an extra point. Take off the last one.
    npts -= 1;

    column1 = vtkFloatArray::New();
    column1->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column1->SetTuple1(i, var1[i]);
    }

    column2 = vtkFloatArray::New();
    column2->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column2->SetTuple1(i, var2[i]);
    }

    column3 = vtkFloatArray::New();
    column3->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column3->SetTuple1(i, var3[i]);
    }

    column4 = vtkFloatArray::New();
    column4->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column4->SetTuple1(i, var4[i]);
    }

    vtkPoints *p = vtkPoints::New();
    p->SetNumberOfPoints(npts);
    for (i = 0 ; i < npts ; i++)
    {
        p->SetPoint(i, var1[i], var2[i], var4[i]);  // Not a typo.
    }

    points = vtkUnstructuredGrid::New();
    points->SetPoints(p);
    p->Delete();

    points->Allocate(npts);
    vtkIdType onevertex[1];
    for (i = 0 ; i < npts ; i++)
    {
         onevertex[0] = i;
         points->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

    haveReadData = true;
}


