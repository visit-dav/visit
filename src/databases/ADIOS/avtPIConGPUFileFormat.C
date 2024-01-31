// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtSTMDFileFormatInterface.h>
#include <avtParallel.h>
#include <ADIOSFileObject.h>
#include <avtPIConGPUFileFormat.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkIdList.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <avtDatabaseMetaData.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <avtIntervalTree.h>
#include <DebugStream.h>
#include <avtDatabase.h>
#include <sys/types.h>

#include <map>
#include <string>
#include <vector>

using namespace std;

static void GetVarInfo(const string &nm, string &varName, string &meshName, avtCentering &cent);

// ****************************************************************************
//  Method: avtEMSTDFileFormat::Identify
//
//  Purpose:
//      Determine if this file is of this flavor.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

bool
avtPIConGPUFileFormat::Identify(const char *fname)
{
    vector<string> attrsToCheck;
    attrsToCheck.push_back("mue0");
    attrsToCheck.push_back("eps0");
    attrsToCheck.push_back("delta_t");
    attrsToCheck.push_back("sim_slides");
    
    ADIOSFileObject *f = new ADIOSFileObject(fname);
    f->Open();

    int attrCount = 0;
    std::map<std::string, int>::const_iterator ai;
    for (ai = f->attributes.begin(); ai != f->attributes.end(); ai++)
    {
        for (int i = 0; i < attrsToCheck.size(); i++)
            if (ai->first.find(attrsToCheck[i]) != string::npos)
            {
                attrCount++;
                break;
            }
    }
    f->Close();
    delete f;
    
    return (attrCount == attrsToCheck.size());
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::CreateInterface
//
//  Purpose:
//      Create an interface for this reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010

// ****************************************************************************

avtFileFormatInterface *
avtPIConGPUFileFormat::CreateInterface(const char *const *llist,
                                       int nList,
                                       int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtSTMDFileFormat **ffl = new avtSTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
    {
        ffl[i] = new avtPIConGPUFileFormat(llist[i*nBlock]);
    }
    
    return new avtSTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtPIConGPUFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtPIConGPUFileFormat::avtPIConGPUFileFormat(const char *nm)
    : avtSTMDFileFormat(&nm, 1)
//    : avtSTMDFileFormat((const char * const *)nm, 1)
{
    fileObj = new ADIOSFileObject(nm);
    fileObj->SetResetDimensionOrder();
    fileObj->Open();
    
    initialized = false;
}

// ****************************************************************************
//  Method: avtPIConGPUFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

avtPIConGPUFileFormat::~avtPIConGPUFileFormat()
{
    delete fileObj;
    fileObj = NULL;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetCycle
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

int
avtPIConGPUFileFormat::GetCycle()
{
    Initialize();
    return cycle;
}

double
avtPIConGPUFileFormat::GetTime()
{
    Initialize();
    return time;
}


// ****************************************************************************
//  Method: avtPIConGPUFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtPIConGPUFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtPIConGPUFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
//  Modifications:
//   Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//   Use name/originalName for names starting with /.
//
// ****************************************************************************

void
avtPIConGPUFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    Initialize();
    
    map<string, ADIOS_VARINFO*>::const_iterator it;

    int numBlocks = -1;
    bool haveFields = false;
    static const string fieldKey = "/fields/";
    for (it = fileObj->variables.begin(); it != fileObj->variables.end(); it++)
    {
        if (it->first.find(fieldKey) != string::npos)
        {
            ADIOS_VARINFO *avi = it->second;
            numBlocks = avi->sum_nblocks;
            haveFields = true;
            break;
        }
    }

    if (haveFields)
    {
        //Add field mesh
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "fieldMesh";
        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->numBlocks = numBlocks;
        mesh->blockOrigin = 0;
        mesh->spatialDimension = 3;
        mesh->topologicalDimension = 3;
        mesh->blockTitle = "bricks";
        mesh->blockPieceName = "brick";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);
    }

    vector<string> particleMesh;
    vector<int> particleBlocks;
    static const string partKey = "/particles/";
    for (it = fileObj->variables.begin(); it != fileObj->variables.end(); it++)
    {
        size_t i0 = it->first.find(partKey);
        if (i0 == string::npos)
            continue;

        avtCentering cent;
        string varNm, meshNm;
        GetVarInfo(it->first, varNm, meshNm, cent);

        bool found = false;
        for (int i = 0; !found && i < particleMesh.size(); i++)
            found = particleMesh[i] == meshNm;
        if (!found)
        {
            particleMesh.push_back(meshNm);
            ADIOS_VARINFO *avi = it->second;
            particleBlocks.push_back(avi->sum_nblocks);
        }
    }

    for (int i = 0; i < particleMesh.size(); i++)
    {
        //Add particle mesh
        avtMeshMetaData *mesh = new avtMeshMetaData;    
        mesh->name = particleMesh[i];
        mesh->meshType = AVT_POINT_MESH;
        mesh->numBlocks = particleBlocks[i];
        mesh->blockOrigin = 0;
        mesh->spatialDimension = 3;
        mesh->topologicalDimension = 1;
        mesh->blockTitle = "bricks";
        mesh->blockPieceName = "brick";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);
    }

    //Add variables.
    for (it = fileObj->variables.begin(); it != fileObj->variables.end(); it++)
    {
        avtCentering cent;
        string varName, meshName;
        GetVarInfo(it->first, varName, meshName, cent);
        avtScalarMetaData *scalar = new avtScalarMetaData();
        if (it->first.find(partKey) != string::npos)
            scalar->name = meshName + "/" + varName;
        else
            scalar->name = varName;
        scalar->originalName = it->first;
        scalar->meshName = meshName;
        scalar->centering = cent;
        md->Add(scalar);
    }
}


// ****************************************************************************
//  Method: avtPIConGPUFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkDataSet *
avtPIConGPUFileFormat::GetMesh(int domain, const char *meshname)
{
    Initialize();
    if (strcmp(meshname, "fieldMesh") == 0)
        return GetFieldMesh(domain);

    //Make the particle mesh...
    string mn(meshname);
    size_t i0 = mn.find("particles_") + 10;
    string meshNm = mn.substr(i0, mn.size()-i0);
    return GetParticleMesh(meshNm, domain);
}

vtkDataSet *
avtPIConGPUFileFormat::GetParticleMesh(string &nm, int domain)
{
    char xName[128], yName[128], zName[128];
    sprintf(xName, "/data/%d/particles/%s/position/x", cycle, nm.c_str());
    sprintf(yName, "/data/%d/particles/%s/position/y", cycle, nm.c_str());
    sprintf(zName, "/data/%d/particles/%s/position/z", cycle, nm.c_str());

    map<string, ADIOS_VARINFO*>::const_iterator itx = fileObj->variables.find(xName);
    map<string, ADIOS_VARINFO*>::const_iterator ity = fileObj->variables.find(yName);
    map<string, ADIOS_VARINFO*>::const_iterator itz = fileObj->variables.find(zName);

    if (itx == fileObj->variables.end())
        EXCEPTION1(InvalidVariableException, xName);
    if (ity == fileObj->variables.end())
        EXCEPTION1(InvalidVariableException, yName);
    if (itz == fileObj->variables.end())
        EXCEPTION1(InvalidVariableException, zName);
    
    ADIOS_VARINFO *avix = itx->second;
    ADIOS_VARINFO *aviy = itx->second;
    ADIOS_VARINFO *aviz = itx->second;

    vtkDataArray *xArr = NULL, *yArr = NULL, *zArr = NULL;
    fileObj->ReadScalarData(xName, 0, domain, &xArr);
    fileObj->ReadScalarData(yName, 0, domain, &yArr);
    fileObj->ReadScalarData(zName, 0, domain, &zArr);

    int nPts = xArr->GetNumberOfTuples();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nPts);
    vtkUnstructuredGrid *grid = vtkUnstructuredGrid::New();
    grid->SetPoints(pts);
    
    vtkIdType vid;
    double pt[3];
    for (int i = 0; i < nPts; i++)
    {
        pt[0] = xArr->GetTuple1(i);
        pt[1] = yArr->GetTuple1(i);
        pt[2] = zArr->GetTuple1(i);
        pts->SetPoint(i, pt);

        vid = i;
        grid->InsertNextCell(VTK_VERTEX, 1, &vid);
    }
    xArr->Delete();
    yArr->Delete();
    zArr->Delete();
    pts->Delete();
    
    return grid;
}

vtkDataSet *
avtPIConGPUFileFormat::GetFieldMesh(int domain)
{
    //cout<<"GetFieldMesh: domain= "<<domain<<endl;
    map<string, ADIOS_VARINFO*>::const_iterator it;
    
    char fieldV[128];
    sprintf(fieldV, "/data/%d/fields/FieldE/x", cycle);
    it = fileObj->variables.find(fieldV);
    if (it == fileObj->variables.end())
        EXCEPTION1(InvalidVariableException, fieldV);

    //Fortran ordering, so swap the X and Z.
    ADIOS_VARINFO *avi = it->second;
    int x0 = avi->blockinfo[domain].start[2];
    int y0 = avi->blockinfo[domain].start[1];
    int z0 = avi->blockinfo[domain].start[0];
    int cx = avi->blockinfo[domain].count[2];
    int cy = avi->blockinfo[domain].count[1];
    int cz = avi->blockinfo[domain].count[0];
    
    //Read these out of the file.
    int nx = cx+1, ny = cy+1, nz = cz+1;
    int d[3] = {nx, ny, nz};
    
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
    grid->SetDimensions(d);
    vtkFloatArray *x = vtkFloatArray::New();
    vtkFloatArray *y = vtkFloatArray::New();
    vtkFloatArray *z = vtkFloatArray::New();
    x->SetNumberOfTuples(nx);
    y->SetNumberOfTuples(ny);
    z->SetNumberOfTuples(nz);
    for (int i = 0; i < nx; i++)
        x->SetTuple1(i, x0+i);
    for (int i = 0; i < ny; i++)
        y->SetTuple1(i, y0+i);
    for (int i = 0; i < nz; i++)
        z->SetTuple1(i, z0+i);

    grid->SetXCoordinates(x);
    grid->SetYCoordinates(y);
    grid->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();
    
    return grid;
}

// ****************************************************************************
//  Method: avtPIConGPUFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkDataArray *
avtPIConGPUFileFormat::GetVar(int domain, const char *vn)
{
    string vname(vn);
    vtkDataArray *arr = NULL;
    fileObj->ReadScalarData(vname, 0, domain, &arr);
    return arr;
}

vtkDataArray *
avtPIConGPUFileFormat::GetParticleVar(bool isIon, std::string &vname, int domain)
{
    char p = (isIon ? 'i' : 'e');
    
    string tmp;
    if (vname.find("momentum_x") != string::npos)
        tmp = "momentum/x";
    else if (vname.find("momentum_y") != string::npos)
        tmp = "momentum/y";
    else if (vname.find("momentum_z") != string::npos)
        tmp = "momentum/z";
    else if (vname.find("weighting") != string::npos)
        tmp = "weighting";
    else if (vname.find("boundElectrons") != string::npos)
        tmp = "boundElectrons";

    char nm[128];
    sprintf(nm, "/data/%d/particles/%c/%s", cycle, p, tmp.c_str());

    //cout<<domain<<" READING: "<<nm<<endl;
    vtkDataArray *arr = NULL;
    fileObj->ReadScalarData(nm, 0, domain, &arr);
    return arr;
}

vtkDataArray *
avtPIConGPUFileFormat::GetIonSelect(int domain)
{
    vtkDataArray *px=NULL, *py=NULL, *pz=NULL, *w=NULL;
    string tmp;

    char nm[128];
    
    sprintf(nm, "/data/%d/particles/i/momentum/x", cycle);
    fileObj->ReadScalarData(nm, 0, domain, &px);
    sprintf(nm, "/data/%d/particles/i/momentum/y", cycle);
    fileObj->ReadScalarData(nm, 0, domain, &py);
    sprintf(nm, "/data/%d/particles/i/momentum/z", cycle);
    fileObj->ReadScalarData(nm, 0, domain, &pz);
    sprintf(nm, "/data/%d/particles/i/weighting", cycle);
    fileObj->ReadScalarData(nm, 0, domain, &w);
    
    if (!px || !py || !pz || !w)
        return NULL;

    int nt = px->GetNumberOfTuples();
    vtkFloatArray *res = vtkFloatArray::New();
    res->SetNumberOfTuples(nt);

    float *ppx = (float*)px->GetVoidPointer(0);
    float *ppy = (float*)py->GetVoidPointer(0);
    float *ppz = (float*)pz->GetVoidPointer(0);
    float *pw = (float*)w->GetVoidPointer(0);
    
    double um;
    sprintf(nm, "/data/%d/particles/i/momentum/x/sim_unit", cycle);
    fileObj->GetAttr(nm, um);
    double um_2 = um*um;
    static const double ionMass = 2.0 * (2.0 * 1.6726217e-27);
    static const double J2eV = 6.24150934e18;
    static const double EnePeak = 14.7e6;
    static const double dEnePeak = 1e6;
    //cout<<"UM= "<<um<<endl;
    for (int i = 0; i < nt; i++)
    {
        double p2 = (ppx[i]*ppx[i] + ppy[i]*ppy[i] + ppz[i]*ppz[i]);
        double val0 = (p2 * um_2) / (pw[i]*pw[i]);
        double val1 = fabs(val0/ionMass * J2eV-EnePeak);

        //if (i < 20) cout<<i<<": "<<val1<<endl;
        
        if (val1 <= dEnePeak)
            res->SetTuple1(i,1.0f);
        else
            res->SetTuple1(i,0.0f);
    }
    
    px->Delete();
    py->Delete();
    pz->Delete();
    w->Delete();

    return res;
}


// ****************************************************************************
//  Method: avtPIConGPUFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

vtkDataArray *
avtPIConGPUFileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method: avtPIConGPUFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Get aux data.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void *
avtPIConGPUFileFormat::GetAuxiliaryData(const char *var, int domain,
                                        const char *type, void *,
                                        DestructorFunction &df)
{
    void *rv = NULL;

    if (!strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS))
    {
        if (!strcmp(var, "fieldMesh"))
        {
            map<string, ADIOS_VARINFO*>::const_iterator it;
            char fieldV[128];
            sprintf(fieldV, "/data/%d/fields/FieldE/x", cycle);
            it = fileObj->variables.find(fieldV);
            if (it == fileObj->variables.end())
                EXCEPTION1(InvalidVariableException, fieldV);

            //Fortran ordering, so swap the X and Z.
            ADIOS_VARINFO *avi = it->second;
            int numBlocks = avi->sum_nblocks;
            
            avtIntervalTree *itree = new avtIntervalTree(numBlocks, 3);
            for (int i = 0; i < numBlocks; i++)
            {
                double x0 = static_cast<double>(avi->blockinfo[i].start[2]);
                double y0 = static_cast<double>(avi->blockinfo[i].start[1]);
                double z0 = static_cast<double>(avi->blockinfo[i].start[0]);
                double cx = static_cast<double>(avi->blockinfo[i].count[2]);
                double cy = static_cast<double>(avi->blockinfo[i].count[1]);
                double cz = static_cast<double>(avi->blockinfo[i].count[0]);
                double nx = static_cast<double>(cx+1);
                double ny = static_cast<double>(cy+1);
                double nz = static_cast<double>(cz+1);
                
                double b[6] = {x0, x0+nx, y0, y0+ny, z0, z0+nz};
                itree->AddElement(i, b);
            }
            itree->Calculate(true);
            rv = (void *) itree;
            df = avtIntervalTree::Destruct;
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtPIConGPUFileFormat::Initialize
//
//  Purpose:
//      Initialize the reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

void
avtPIConGPUFileFormat::Initialize()
{
    if (!fileObj->Open())
        EXCEPTION0(ImproperUseException);

    if (initialized)
        return;

    cycle = CalcCycle();
    time = cycle * 1.0;

    initialized = true;
}

static bool
isaNumber(const string &str, int &n)
{
    stringstream ss(str);
    int tmp;
    if ((ss >> tmp).fail())
        return false;
    n = tmp;
    return true;
}

int
avtPIConGPUFileFormat::CalcCycle()
{
    cycle = -1;

    map<string, ADIOS_VARINFO*>::iterator it;
    for (it = fileObj->variables.begin(); it != fileObj->variables.end(); it++)
    {
        static const char *key = "/data/";
        size_t i0 = strlen(key);
        if (i0 != string::npos)
        {
            size_t i1 = it->first.find("/", i0+1);
            string str = it->first.substr(i0, i1-i0);
            if (isaNumber(str, cycle))
                return cycle;
        }
    }

    EXCEPTION1(InvalidVariableException, "cycle not found");
}

static void
GetVarInfo(const string &nm, string &varName, string &meshName, avtCentering &cent)
{
    static const string fieldKey = "/fields/";
    static const string partKey = "/particles/";
    if (nm.find(fieldKey) != string::npos)
    {
        cent = AVT_ZONECENT;
        meshName = "fieldMesh";
        size_t i0 = nm.find(fieldKey);
        size_t i1 = i0+fieldKey.size();
        size_t i2 = nm.size();
        varName = nm.substr(i1, i1-i2);
    }
    else
    {
        cent = AVT_NODECENT;
        size_t i0 = nm.find(partKey);
        size_t i1 = i0+partKey.size();
        size_t i2 = nm.find("/", i1);
        size_t i3 = nm.size();
        meshName = "particles_" + nm.substr(i1, i2-i1);
        varName = nm.substr(i2+1, i3-i2);
    }
}
