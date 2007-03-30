// ***************************************************************************
//                              avtHexFileFormat.C                           
//
//  Purpose:  Sample database which reads test files of hexahedrons
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2002
//
// ****************************************************************************


#include <avtHexFileFormat.h>

#include <vector>
#include <string>

#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabaseMetaData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

using std::vector;
using std::string;

#if defined(_WIN32)
// Define >> for strings since Windows does not seem to have it.
ifstream &operator >> (ifstream &s, string &str)
{
    char tmp[1000];
    s >> tmp;
    str = string(tmp);
    return s;
}
#endif


avtHexFileFormat::avtHexFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    filename = fname;

    ifstream ifile(filename.c_str());

    if (ifile.fail())
    {
        debug1 << "Unable to open file " << fname << endl;
    }
    else
    {
        ReadFile(ifile);
    }
}

avtHexFileFormat::~avtHexFileFormat()
{
}

vtkDataSet *
avtHexFileFormat::GetMesh(int dom, const char *name)
{
    if (dom < 0 || dom >= 1)
    {
        EXCEPTION2(BadIndexException, dom, 1);
    }

    if (strcmp(name, "Mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    vtkUnstructuredGrid *dataset = vtkUnstructuredGrid::New(); 

    int i, npts = points.size()/3;
    vtkPoints *vtkpoints  = vtkPoints::New();
    vtkpoints->SetNumberOfPoints(npts);
    float *pts = (float *) vtkpoints->GetVoidPointer(0);

    for (i=0; i<npts*3; i++)
    {
        pts[i] = points[i];
    }
     
    int nhexes = hexnodes.size() / 8;
    dataset->Allocate(nhexes);
    for (i=0; i<nhexes; i++)
    {
        vtkIdType hex[8] =
          {hexnodes[i*8+0], hexnodes[i*8+1], hexnodes[i*8+2], hexnodes[i*8+3],
           hexnodes[i*8+4], hexnodes[i*8+5], hexnodes[i*8+6], hexnodes[i*8+7]};
        dataset->InsertNextCell(VTK_HEXAHEDRON, 8, hex);
    }

    dataset->SetPoints(vtkpoints);
    vtkpoints->Delete();

    return dataset;
}



vtkDataArray *
avtHexFileFormat::GetVar(int, const char *name)
{
    int i, npts = points.size()/3;
    int nhexes = hexnodes.size() / 8;

    for (i = 0 ; i < cellvarnames.size(); i++)
    {
        if (cellvarnames[i] == string(name))
        {
            vtkFloatArray   *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(nhexes);
            float *ptr = (float*)scalars->GetVoidPointer(0);
            memcpy(ptr, &cellvars[i][0], sizeof(float) * nhexes);
            return scalars;
        }
    }

    for (i = 0 ; i < pointvarnames.size(); i++)
    {
        if (pointvarnames[i] == string(name))
        {
            vtkFloatArray   *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(npts);
            float *ptr = (float*)scalars->GetVoidPointer(0);
            memcpy(ptr, &pointvars[i][0], sizeof(float) * npts);
            return scalars;
        }
    }

    EXCEPTION1(InvalidVariableException, name);
}


void
avtHexFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!points.size() || !hexnodes.size())
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    int i;
    for (i=0; i < cellvarnames.size(); i++)
    {
        md->Add(new avtScalarMetaData(cellvarnames[i], "Mesh", AVT_ZONECENT));
    }
    for (i=0; i < pointvarnames.size(); i++)
    {
        md->Add(new avtScalarMetaData(pointvarnames[i], "Mesh", AVT_NODECENT));
    }
}


void
avtHexFileFormat::ReadFile(ifstream &ifile)
{
    int i,j;

    int npoints;
    int nhexes;
    int ncellvars;
    int npointvars;

    string buff;

    // points
    ifile >> buff >> npoints;

    points.resize(npoints*3);
    for (i = 0 ; i < npoints*3 ; i++)
    {
        ifile >> points[i];
    }

    // connectivity
    ifile >> buff >> nhexes;

    hexnodes.resize(nhexes*8);
    for (i = 0 ; i < nhexes*8 ; i++)
    {
        ifile >> hexnodes[i];
    }

    // cell vars
    ifile >> buff >> ncellvars;

    cellvars.resize(ncellvars);
    cellvarnames.resize(ncellvars);
    for (j = 0 ; j < ncellvars ; j++)
    {
        ifile >> cellvarnames[j];

        cellvars[j].resize(nhexes);
        for (i = 0 ; i < nhexes ; i++)
        {
            ifile >> cellvars[j][i];
        }
    }

    // point vars
    ifile >> buff >> npointvars;

    pointvars.resize(npointvars);
    pointvarnames.resize(npointvars);
    for (j = 0 ; j < npointvars ; j++)
    {
        ifile >> pointvarnames[j];
        cerr << "found pointvar " << pointvarnames[j].c_str() << endl;

        pointvars[j].resize(npoints);
        for (i = 0 ; i < npoints ; i++)
        {
            ifile >> pointvars[j][i];
        }
    }
}
