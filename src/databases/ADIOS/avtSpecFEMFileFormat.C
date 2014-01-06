/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtMTMDFileFormatInterface.h>
#include <avtParallel.h>
#include "ADIOSFileObject.h"
#include "avtSpecFEMFileFormat.h"
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <avtDatabase.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTriangle.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <snprintf.h>

#include <set>
#include <string>
#include <vector>

using namespace std;

static bool doWhole = true;
static bool onLens = false;
static int DIV = 1;

//#define POINT_MESH
#define USE_IBOOL


// ****************************************************************************
//  Method: avtPixleFileFormat::Identify
//
//  Purpose:
//      Determine if this file is of this flavor.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

bool
avtSpecFEMFileFormat::Identify(const char *fname)
{
    ADIOSFileObject *f = new ADIOSFileObject(fname);
    f->Open();
    bool val = false;
    if (avtSpecFEMFileFormat::IsMeshFile(f) || avtSpecFEMFileFormat::IsDataFile(f))
    {
        string meshNm, dataNm;
        avtSpecFEMFileFormat::GenerateFileNames(f->Filename(), meshNm, dataNm);
        ifstream mFile(meshNm.c_str()), dFile(dataNm.c_str());
        if (!mFile.fail() && !dFile.fail())
            val = true;
    }
    delete f;
    cout<<"SPECFM "<<val<<endl;
    return val;
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::CreateInterface
//
//  Purpose:
//      Create an interface for this reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtFileFormatInterface *
avtSpecFEMFileFormat::CreateInterface(const char *const *list,
                                      int nList,
                                      int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtSpecFEMFileFormat(list[i*nBlock]);
    
    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtSpecFEMFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtSpecFEMFileFormat::avtSpecFEMFileFormat(const char *nm)
    : avtMTMDFileFormat(nm)
{
    string filename(nm), meshNm, dataNm;
    GenerateFileNames(filename, meshNm, dataNm);

    cout<<"Mesh: "<<meshNm<<endl;
    cout<<"Data: "<<dataNm<<endl;
    
    meshFile = new ADIOSFileObject(meshNm.c_str());
    dataFile = new ADIOSFileObject(dataNm.c_str());
    initialized = false;
    //This needs to be put into the file.
    ngllx = nglly = ngllz = 5;
    nWriters = 1;
    nRegions = 1;
    nElems = 0;
    nPts = 0;
}

// ****************************************************************************
//  Method: avtSpecFEMFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtSpecFEMFileFormat::~avtSpecFEMFileFormat()
{
    if (meshFile)
        delete meshFile;
    if (dataFile)
        delete dataFile;
    meshFile = NULL;
    dataFile = NULL;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

int
avtSpecFEMFileFormat::GetNTimesteps()
{
    return 1;
    //return file->NumTimeSteps();
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetCycles
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

void
avtSpecFEMFileFormat::GetCycles(std::vector<int> &cycles)
{
    cycles.push_back(0);
}


// ****************************************************************************
// Method:  avtSpecFEMFileFormat::GetTimes
//
// Purpose:
//   
// Programmer:  Dave Pugmire
// Creation:    January 26, 2011
//
// ****************************************************************************

void
avtSpecFEMFileFormat::GetTimes(std::vector<double> &times)
{
    times.push_back(0.0);
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

void
avtSpecFEMFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Add expressions.
//
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

void
avtSpecFEMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug5<<"avtSpecFEMFileFormat::PopulateDatabaseMetaData()"<<endl;
    Initialize();
    md->SetFormatCanDoDomainDecomposition(false);

    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "mesh";
    mmd->spatialDimension = 3;
    mmd->topologicalDimension = 3;
#ifdef POINT_MESH
    mmd->meshType = AVT_POINT_MESH;
#else
    mmd->meshType = AVT_UNSTRUCTURED_MESH;
#endif

    if (doWhole)
        mmd->numBlocks = (onLens ? 600 : 50);
    else
        mmd->numBlocks = 4;
    md->Add(mmd);

    for (int i = 0; i < variables.size(); i++)
    {
        avtScalarMetaData *sm = new avtScalarMetaData;
        sm->name = variables[i];
        sm->meshName = "mesh";
        sm->hasDataExtents = false;
        sm->centering = AVT_NODECENT;

        md->Add(sm);
    }

    AddScalarVarToMetaData(md, "pointID", "mesh", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::GetMesh
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
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

vtkDataSet *
avtSpecFEMFileFormat::GetMesh(int ts, int domain, const char *meshname)
{
    debug1 << "avtSpecFEMFileFormat::GetMesh " << meshname << endl;
    Initialize();

    vtkDataArray *x = NULL, *y = NULL, *z = NULL, *ib = NULL;
    string varPath = domainVarPaths[domain].first;
    int writerIdx = domainVarPaths[domain].second;

    meshFile->ReadVariable("reg1/xstore/array", ts, writerIdx, &x);
    meshFile->ReadVariable("reg1/ystore/array", ts, writerIdx, &y);
    meshFile->ReadVariable("reg1/zstore/array", ts, writerIdx, &z);
    meshFile->ReadVariable("reg1/ibool/array", ts, writerIdx, &ib);

    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();    
    vtkPoints *pts = vtkPoints::New();
    int N = 278000;
    int pOffset = N * domain;
    int n = x->GetNumberOfTuples();

    int *ibl = (int*)ib->GetVoidPointer(0);
    ibl = &(ibl[pOffset]);

#ifdef USE_IBOOL
    vector<bool> ptMask(N, false);
    vector<int> ptID(N);
    int ptCnt = 0;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            ptID[idx] = ptCnt;
            ptCnt++;
            ptMask[idx] = true;
        }
    }
    pts->SetNumberOfPoints(ptCnt);
    
    for (int i = 0; i < N; i++)
        ptMask[i] = false;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            pts->SetPoint(ptID[idx],
                          x->GetTuple1(i+pOffset),
                          y->GetTuple1(i+pOffset),
                          z->GetTuple1(i+pOffset));
            ptMask[idx] = true;
        }
    }

#else
    pts->SetNumberOfPoints(N);
    for (int i = 0; i < N; i++)
        pts->SetPoint(i,
                      x->GetTuple1(i+pOffset),
                      y->GetTuple1(i+pOffset),
                      z->GetTuple1(i+pOffset));
#endif

                      
    mesh->SetPoints(pts);
    
#ifdef POINT_MESH
    vtkIdType v;
    for (int i = 0; i < pts->GetNumberOfPoints(); i++)
    {
        v = i;
        mesh->InsertNextCell(VTK_VERTEX, 1, &v);
    }
#else
    int nElems;
    meshFile->GetIntScalar("reg1/nspec", nElems);
    int di=1, dj=1, dk=1;
    vtkIdType v[8];
    int eCnt = 0;
    
    #define INDEX(x,i,j,k,e) x[(i)+(j)*ngllx + (k)*ngllx*nglly + (e)*ngllx*nglly*ngllz] - 1;
    for (int e = 0; e < nElems; e++)
        for (int k = 0; k < ngllz-1; k+=dk)
            for (int j = 0; j < nglly-1; j+=dj)
                for (int i = 0; i < ngllx-1; i+=di)
                {
                    v[0] = INDEX(ibl,i,j,k,e);
                    v[1] = INDEX(ibl,i+di,j,k,e);
                    v[2] = INDEX(ibl,i+di,j+dj,k,e);
                    v[3] = INDEX(ibl,i,j+dj,k,e);
    
                    v[4] = INDEX(ibl,i,j,k+dk,e);
                    v[5] = INDEX(ibl,i+di,j,k+dk,e);
                    v[6] = INDEX(ibl,i+di,j+dj,k+dk,e);
                    v[7] = INDEX(ibl,i,j+dj,k+dk,e);
                    for (int vi = 0; vi < 8; vi++)
                    {
                        //v[vi] = ptID[v[vi]];
                        v[vi] = v[vi];
                    }
                    mesh->InsertNextCell(VTK_HEXAHEDRON, 8, v);
                    eCnt++;
                }
    
#endif

    cout<<"MESH: "<<domain<<" nPts= "<<pts->GetNumberOfPoints()<<endl;
    pts->Delete();
    x->Delete();
    y->Delete();
    z->Delete();
    ib->Delete();

    return mesh;
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::GetVar
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
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Handle time varying variables correctly.
//
// ****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVar(int ts, int domain, const char *varname)
{
    cout<<"GetVar: "<<domain<<" "<<varname<<endl;
    if (!strcmp(varname, "pointID"))
    {
        int N = 278000;
        vtkFloatArray *var = vtkFloatArray::New();
        var->SetNumberOfTuples(N);
        for (int i = 0; i < N; i++)
            var->SetTuple1(i, i);
        return var;
    }

    debug1 << "avtSpecFEMFileFormat::GetVar " << varname << endl;

    Initialize();

    string varPath = domainVarPaths[domain].first;
    int writerIdx = domainVarPaths[domain].second;

    vtkDataArray *arr = NULL, *ib = NULL;
    dataFile->ReadVariable(string("reg1/") + varname + string("/array"), ts, 0, &arr);
    meshFile->ReadVariable("reg1/ibool/array", ts, 0, &ib);

    int N = 278000;
    int pOffset = N * domain;
    vtkFloatArray *var = vtkFloatArray::New();

#ifdef USE_IBOOL
    int n = arr->GetNumberOfTuples();

    vector<bool> ptMask(N, false);
    vector<int> ptID(N);
    int *ibl = (int*)ib->GetVoidPointer(0);
    ibl = &(ibl[pOffset]);
    int ptCnt = 0;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            ptID[idx] = ptCnt;
            ptCnt++;
            ptMask[idx] = true;
        }
    }
    var->SetNumberOfTuples(ptCnt);
    for (int i = 0; i < N; i++)
        ptMask[i] = false;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            var->SetTuple1(ptID[idx], arr->GetTuple1(i+pOffset));
            ptMask[idx] = true;
        }
    }
#else

    var->SetNumberOfTuples(N);
    for (int i = 0; i < N; i++)
        var->SetTuple1(i, arr->GetTuple1(i+pOffset));
#endif

    arr->Delete();
    ib->Delete();

    //cout<<"VAR: "<<domain<<" npts= "<<n<<" ptCnt= "<<ptCnt<<endl;
    return var;
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::GetVectorVar
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
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method: avtSpecFEMFileFormat::Initialize
//
//  Purpose:
//      Initialize the reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Read in expressions.
//
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

void
avtSpecFEMFileFormat::Initialize()
{
    if (! meshFile->Open() || !dataFile->Open())
        EXCEPTION0(ImproperUseException);

    if (initialized)
        return;

    //Determine how many writers there are.
    int g, l;
    meshFile->GetIntScalar("reg1/x_global/global_dim", g);
    meshFile->GetIntScalar("reg1/x_global/local_dim", l);
    nWriters = g/l;
    nRegions = 1;

    //Construct the domain to regions mapping.
    char varPath[32];
    for (int i = 0; i < nWriters; i++)
        for (int j = 0; j < nRegions; j++)
        {
            SNPRINTF(varPath, 32, "reg%d/", j+1);
            domainVarPaths.push_back(pair<string,int>(varPath, i));
        }

    //See what variables we have.
    ADIOSFileObject::varIter vi;
    for (vi = dataFile->variables.begin(); vi != dataFile->variables.end(); vi++)
    {
        const ADIOSVar &v = vi->second;
        
        string varName = v.name;
        string::size_type idx = varName.find("/array");

        //extract varname from reg?/varname/array
        varName = varName.substr(5, idx-5);

        if (find(variables.begin(), variables.end(), varName) == variables.end())
            variables.push_back(varName);
    }

    initialized = true;
}

void
avtSpecFEMFileFormat::GenerateFileNames(const std::string &nm,
                                        std::string &meshNm, std::string &dataNm)
{
    if (doWhole)
    {
        if (onLens)
        {
            meshNm = "/lustre/widow/proj/csc094/pugmire/specfm/whole/solver_data.bp";
            dataNm = "/lustre/widow/proj/csc094/pugmire/specfm/whole/solver_meshfiles.bp";
        }
        else
        {
            meshNm = "/home/pugmire/proj/specfm/whole/solver_data.bp";
            dataNm = "/home/pugmire/proj/specfm/whole/solver_meshfiles.bp";
        }
        DIV = 20;
    }
    else
    {
        if (onLens)
        {
            meshNm = "/lustre/widow/proj/csc094/pugmire/specfm/small/solver_data.bp";
            dataNm = "/lustre/widow/proj/csc094/pugmire/specfm/small/solver_meshfiles.bp";
        }
        else
        {
            meshNm = "/home/pugmire/proj/specfm/regional_Greece_merged/DATABASES_MPI/solver_data.bp";
            dataNm = "/home/pugmire/proj/specfm/regional_Greece_merged/DATABASES_MPI/solver_meshfiles.bp";
        }
        DIV = 1;
    }

    return;



    string::size_type i0 = nm.rfind("_data.bp");
    string::size_type i1 = nm.rfind("_meshfiles.bp");

    if (i0 != string::npos)
    {
        meshNm = nm;
        dataNm = nm.substr(0, i0) + "_meshfiles.bp";
    }
    else if (i1 != string::npos)
    {
        meshNm = nm.substr(0, i0) + "_data.bp";
        dataNm = nm;
    }
    else
    {
        EXCEPTION1(ImproperUseException, "Data and/or mesh files not found.");
    }
}

bool
avtSpecFEMFileFormat::IsMeshFile(ADIOSFileObject *f)
{
    int x;
    
    if (!f->GetIntScalar("/nspec", x) || !f->GetIntScalar("/nglob", x) ||
        !f->GetIntScalar("/xstore/local_dim", x) || !f->GetIntScalar("/xstore/global_dim", x) ||
        !f->GetIntScalar("/ystore/local_dim", x) || !f->GetIntScalar("/ystore/global_dim", x) ||
        !f->GetIntScalar("/zstore/local_dim", x) || !f->GetIntScalar("/zstore/global_dim", x))
    {
        return false;
    }
    
    return true;
}

bool
avtSpecFEMFileFormat::IsDataFile(ADIOSFileObject *f)
{
    return true;
}
