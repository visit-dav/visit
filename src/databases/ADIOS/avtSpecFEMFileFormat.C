/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

int avtSpecFEMFileFormat::NUM_REGIONS = 3;

static inline void
convertToLatLon(double x, double y, double z, double &nx, double &ny, double &nz);

//#define POINT_MESH
#define USE_IBOOL

// ****************************************************************************
//  Method: avtSpecFEMFileFormat::Identify
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
    string meshNm, dataNm;
    bool valid = false;
    
    if (avtSpecFEMFileFormat::GenerateFileNames(fname, meshNm, dataNm))
    {
        ifstream mFile(meshNm.c_str()), dFile(dataNm.c_str());
        if (!mFile.fail() && !dFile.fail())
            valid = true;

        if (valid)
        {
            ADIOSFileObject *f = new ADIOSFileObject(meshNm);
            f->Open();
            if (! avtSpecFEMFileFormat::IsMeshFile(f))
                valid = false;
            delete f;
        }
        
        if (valid)
        {
            ADIOSFileObject *f = new ADIOSFileObject(dataNm);
            f->Open();
            if (! avtSpecFEMFileFormat::IsDataFile(f))
                valid = false;
            delete f;
        }
    }
    return valid;
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

    meshFile = new ADIOSFileObject(meshNm);
    dataFile = new ADIOSFileObject(dataNm);
    initialized = false;
    //This needs to be put into the file.
    ngllx = nglly = ngllz = 5;
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

    //Add the entire mesh.
#ifdef USE_IBOOL
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
    AddMeshToMetaData(md, "LatLon_mesh", AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
#endif
#ifdef POINT_MESH
    AddMeshToMetaData(md, "mesh", AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
    AddMeshToMetaData(md, "LatLon_mesh", AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
#endif
    
    for (int i = 0; i < regions.size(); i++)
    {
        if (regions[i])
        {
            avtMeshMetaData *mmd = new avtMeshMetaData;
            char nm[128], nm2[128];
            sprintf(nm, "reg%d/mesh", i+1);
            sprintf(nm2, "reg%d/LatLon_mesh", i+1);
#ifdef USE_IBOOL
            AddMeshToMetaData(md, nm, AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
            AddMeshToMetaData(md, nm2, AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
#endif
#ifdef POINT_MESH
            AddMeshToMetaData(md, nm, AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
            AddMeshToMetaData(md, nm2, AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
#endif
        }
    }

    bool allRegionsPresent = true;
    for (int i = 0; i < regions.size(); i++)
        allRegionsPresent &= regions[i];

    //Add the variables
    map<string, ADIOS_VARINFO*>::const_iterator it;
    for (it = dataFile->variables.begin(); it != dataFile->variables.end(); it++)
    {
        string vname = GetVariable(it->first);
        //Add var only if all regions present.
        if (allRegionsPresent)
        {
            AddScalarVarToMetaData(md, vname, "mesh", AVT_NODECENT);
            AddScalarVarToMetaData(md, vname, "LatLon_mesh", AVT_NODECENT);

            AddVectorVarToMetaData(md, "LatLonR_coords", "mesh", AVT_NODECENT, 3);
        }
        
        for (int i = 0; i < regions.size(); i++)
        {
            if (regions[i])
            {
                char mesh[128], mesh2[128], var[128], var2[128];
                sprintf(mesh, "reg%d/mesh", i+1);
                sprintf(mesh2, "reg%d/LatLon_mesh", i+1);
                sprintf(var, "reg%d/%s", i+1, vname.c_str());
                sprintf(var2, "reg%d/LatLon/%s", i+1, vname.c_str());
                AddScalarVarToMetaData(md, var, mesh, AVT_NODECENT);
                AddScalarVarToMetaData(md, var2, mesh2, AVT_NODECENT);

                sprintf(var, "reg%d/LatLonR_coords", i+1);
                AddVectorVarToMetaData(md, var, mesh, AVT_NODECENT, 3);
            }
        }
    }
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetWholeMesh
//
// Purpose:
//   Get mesh containing all regions.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************


vtkDataSet *
avtSpecFEMFileFormat::GetWholeMesh(int ts, int dom, bool xyzMesh)
{
    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    mesh->SetPoints(pts);

    int nPts, ptOffset = 0;
    char nP[128];
    for (int i = 0; i < avtSpecFEMFileFormat::NUM_REGIONS; i++)
    {
        AddRegionMesh(ts, dom, i+1, mesh, xyzMesh, ptOffset);

        sprintf(nP, "reg%d/nglob", i+1);
        meshFile->GetScalar(nP, nPts);
        ptOffset += nPts;
    }
    pts->Delete();
    
    return mesh;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::AddRegionMesh
//
// Purpose:
//   Get the mesh for a particular region.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

void
avtSpecFEMFileFormat::AddRegionMesh(int ts, int dom, int region, vtkDataSet *ds,
                                    bool xyzMesh, int ptOffset)
{
    vtkUnstructuredGrid *mesh = (vtkUnstructuredGrid*)ds;
    vtkPoints *pts = mesh->GetPoints();

    vtkDataArray *x = NULL, *y = NULL, *z = NULL, *ib = NULL;
    char xNm[128], yNm[128], zNm[128], iNm[128], nE[128], nP[128];
    sprintf(xNm, "reg%d/xstore/array", region);
    sprintf(yNm, "reg%d/ystore/array", region);
    sprintf(zNm, "reg%d/zstore/array", region);
    sprintf(iNm, "reg%d/ibool/array", region);
    sprintf(nE, "reg%d/nspec", region);
    sprintf(nP, "reg%d/nglob", region);

    int nElem, nPts;
    meshFile->GetScalar(nE, nElem);
    meshFile->GetScalar(nP, nPts);

    //Process which points are needed....
    meshFile->ReadScalarData(iNm, ts, dom, &ib);
    int N = ib->GetNumberOfTuples();

    int *ibl = (int*)ib->GetVoidPointer(0);
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
    for (int i = 0; i < N; i++)
        ptMask[i] = false;

    meshFile->ReadScalarData(xNm, ts, dom, &x);
    meshFile->ReadScalarData(yNm, ts, dom, &y);
    meshFile->ReadScalarData(zNm, ts, dom, &z);
    vtkIdType vid;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            double ptX = x->GetTuple1(i);
            double ptY = y->GetTuple1(i);
            double ptZ = z->GetTuple1(i);

            if (xyzMesh)
                vid = pts->InsertNextPoint(ptX, ptY, ptZ);
            else
            {
                double newX, newY, newZ;
                convertToLatLon(ptX, ptY, ptZ, newX, newY, newZ);
                vid = pts->InsertNextPoint(newX, newY, newZ);
            }
            ptMask[idx] = true;
#ifdef POINT_MESH
            mesh->InsertNextCell(VTK_VERTEX, 1, &vid);
#endif
        }
    }

#ifdef USE_IBOOL
    int di=1, dj=1, dk=1, eCnt = 0;
    vtkIdType v[8];
    
    #define INDEX(x,i,j,k,e) x[(i)+(j)*ngllx + (k)*ngllx*nglly + (e)*ngllx*nglly*ngllz] - 1;
    for (int e = 0; e < nElem; e++)
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
                        v[vi] += ptOffset;

                    if (xyzMesh)
                    {
                        mesh->InsertNextCell(VTK_HEXAHEDRON, 8, v);
                        /*
                        if (eCnt == 43011 || eCnt == 43010)
                        {
                            double verts[8][3];
                            for (int p = 0; p < 8; p++)
                                pts->GetPoint(v[p], verts[p]);
                            cout<<eCnt<<": "<<endl;
                            for (int p = 0; p < 8; p++)
                            {
                                double px = verts[p][0];
                                double py = verts[p][1];
                                double pz = verts[p][2];
                                double RR = px*px+py*py;
                                double R = sqrt(RR + pz*pz);
                                double nx = R;
                                double ny = (R==0.0 ? 0.0 : acos(pz/R));
                                double nz = (RR==0.0 ? 0.0 : M_PI + atan2(-py, -px));
                                             
                                cout<<p<<" ["<<verts[p][0]<<" "<<verts[p][1]<<" "<<verts[p][2]<<"] ["<<nx<<" "<<ny<<" "<<nz<<"] 2pi= "<<2.0*M_PI<<endl;
                            }
                        }
                        */
                        eCnt++;
                    }
                    else
                    {
                        //Check for wrap-around cells, and toss them.
                        double verts[8][3];
                        for (int p = 0; p < 8; p++)
                            pts->GetPoint(v[p], verts[p]);

                        bool cellGood = true;
                        for (int p = 0; cellGood && p < 8; p++)
                            for (int q = 0; cellGood && q < 8; q++)
                                if (p != q)
                                {
                                    double dy = fabs(verts[p][1]-verts[q][1]);
                                    double dz = fabs(verts[p][2]-verts[q][2]);
                                    if (dy > M_PI) cellGood = false;
                                    if (dz > M_PI) cellGood = false;
                                }
                        if (cellGood)
                        {
                            mesh->InsertNextCell(VTK_HEXAHEDRON, 8, v);
                            eCnt++;
                        }
                    }
                }
#endif

    x->Delete();
    y->Delete();
    z->Delete();
    ib->Delete();
}


//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetRegionMesh
//
// Purpose:
//   Get the mesh for a particular region.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************
 
vtkDataSet *
avtSpecFEMFileFormat::GetRegionMesh(int ts, int dom, int region, bool xyzMesh)
{
    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    mesh->SetPoints(pts);

    AddRegionMesh(ts, dom, region, mesh, xyzMesh);
    
    pts->Delete();
    return mesh;
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

    bool xyzMesh = string(meshname).find("LatLon") == string::npos;
    bool wholeMesh = string(meshname).find("reg") == string::npos;
    if (wholeMesh)
        return GetWholeMesh(ts, domain, xyzMesh);
    else
        return GetRegionMesh(ts, domain, GetRegion(meshname), xyzMesh);
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVarRegion
//
// Purpose:
//   Get the variable for a particular region.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVarRegion(std::string &nm, int ts, int dom)
{
    vtkDataArray *arr = NULL, *ib = NULL;
    char iNm[128];
    //Read the ibool array.
    int region = GetRegion(nm);
    sprintf(iNm, "reg%d/ibool/array", region);
    meshFile->ReadScalarData(iNm, ts, dom, &ib);
    
    int N = ib->GetNumberOfTuples();
    vector<bool> ptMask(N, false);
    vector<int> ptID(N);
    int *ibl = (int*)ib->GetVoidPointer(0);

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
    for (int i = 0; i < N; i++)
        ptMask[i] = false;

    string vname = nm+"/array";
    dataFile->ReadScalarData(vname, ts, dom, &arr);
    vtkFloatArray *var = vtkFloatArray::New();
    var->SetNumberOfTuples(ptCnt);
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            var->SetTuple1(ptID[idx], arr->GetTuple1(i));
            ptMask[idx] = true;
        }
    }
    ib->Delete();
    arr->Delete();
    
    return var;
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
    string vName = varname;
    size_t i;
    if ((i = vName.find("LatLon")) != string::npos)
        vName = vName.substr(0, i) + vName.substr((i+7), string::npos);
    
    if (vName.find("reg") != string::npos)
        return GetVarRegion(vName, ts, domain);

    //Determine how many total values.
    int n = 0, nPts;
    char tmp[128];
    vector<int> regNPts;
    for (int i = 0; i < avtSpecFEMFileFormat::NUM_REGIONS; i++)
    {
        sprintf(tmp, "reg%d/nglob", i+1);
        meshFile->GetScalar(tmp, nPts);
        regNPts.push_back(nPts);
        n += nPts;
    }
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(n);
    
    //Append in each region to the whole.
    int offset = 0;
    for (int i = 0; i < avtSpecFEMFileFormat::NUM_REGIONS; i++)
    {
        sprintf(tmp, "reg%d/%s", i+1, varname);
        string vNm = tmp;
        vtkDataArray *v = GetVarRegion(vNm, ts, domain);
        
        float *ptr = &((float *)arr->GetVoidPointer(0))[offset];
        for (int j = 0; j < v->GetNumberOfTuples(); j++)
            ptr[j] = v->GetTuple1(j);
        
        offset += regNPts[i];
        v->Delete();
    }
    
    return arr;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVectorVar
//
// Purpose:
//   Retreive a vector variable.
//
// Arguments:
//   
//
// Programmer:  Dave Pugmire
// Creation:    December 19, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVectorVar(int ts, int domain, const char *varname)
{
    Initialize();

    string vName = varname;
    if (vName.find("reg") != string::npos)
        return GetVectorVarRegion(vName, ts, domain);

    EXCEPTION0(ImproperUseException);
    return NULL;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVectorVarRegion
//
// Purpose:
//   Retreive a vector variable.
//
// Arguments:
//   
//
// Programmer:  Dave Pugmire
// Creation:    December 19, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVectorVarRegion(std::string &nm, int ts, int dom)
{
    if (nm.find("LatLonR_coords") == string::npos)
        return NULL;
    
    int region = GetRegion(nm);
    vtkDataSet *mesh = GetRegionMesh(ts, dom, region, true);

    int nPts = mesh->GetNumberOfPoints();
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfComponents(3);
    arr->SetNumberOfTuples(nPts);

    double lat = 0.0, lon = 0.0, r = 0.0;
    double pt[3];
    for (int i = 0; i < nPts; i++)
    {
        mesh->GetPoint(i, pt);
        convertToLatLon(pt[0], pt[1], pt[2], r, lat, lon);
        lat = 90.0 - lat;
        /*
        if (lon > 180.0)
            lon = lon - 360.0;
        */
        
        arr->SetComponent(i, 0, lat);
        arr->SetComponent(i, 1, lon);
        arr->SetComponent(i, 2, r);
    }

    mesh->Delete();
    return arr;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::Initialize
//
// Purpose:
//   Create world peace.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

void
avtSpecFEMFileFormat::Initialize()
{
    if (! meshFile->Open() || !dataFile->Open())
        EXCEPTION0(ImproperUseException);

    if (initialized)
        return;

    //See which regions we have.
    regions.resize(NUM_REGIONS, false);

    map<string, ADIOS_VARINFO*>::const_iterator it;
    numBlocks = 1;
    for (it = dataFile->variables.begin(); it != dataFile->variables.end(); it++)
    {
        regions[GetRegion(it->first)-1] = true;
        if (it->second->sum_nblocks > numBlocks)
            numBlocks = it->second->sum_nblocks;
    }
    initialized = true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GenerateFileNames
//
// Purpose:
//   Generate the mesh data file names.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************


bool
avtSpecFEMFileFormat::GenerateFileNames(const std::string &nm,
                                        std::string &meshNm, std::string &dataNm)
{
    //They have opened the mesh file... Can't continue.
    if (nm.find("solver_data.bp") != string::npos)
        return false;

    dataNm = nm;

    //Create the meshNm.
    string::size_type idx = dataNm.rfind("/");
    meshNm = dataNm.substr(0,idx+1) + "solver_data.bp";

    return true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::IsMeshFile
//
// Purpose:
//   See if this file contains the mesh
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
avtSpecFEMFileFormat::IsMeshFile(ADIOSFileObject *f)
{
    //Make sure we have all of these scalars...
    std::map<std::string, ADIOS_VARINFO*>::const_iterator si;
    int scalarCount = 0;
    for (si = f->scalars.begin(); si != f->scalars.end(); si++)
    {
        if (si->first.find("/nspec") != string::npos) scalarCount++;
        else if (si->first.find("/nglob") != string::npos) scalarCount++;
        else if (si->first.find("/x_global/local_dim") != string::npos) scalarCount++;
        else if (si->first.find("/x_global/global_dim") != string::npos) scalarCount++;
        else if (si->first.find("/x_global/offset") != string::npos) scalarCount++;
        else if (si->first.find("/y_global/local_dim") != string::npos) scalarCount++;
        else if (si->first.find("/y_global/global_dim") != string::npos) scalarCount++;
        else if (si->first.find("/y_global/offset") != string::npos) scalarCount++;
        else if (si->first.find("/z_global/local_dim") != string::npos) scalarCount++;
        else if (si->first.find("/z_global/global_dim") != string::npos) scalarCount++;
        else if (si->first.find("/z_global/offset") != string::npos) scalarCount++;
        else if (si->first.find("/xstore/local_dim") != string::npos) scalarCount++;
        else if (si->first.find("/xstore/global_dim") != string::npos) scalarCount++;
        else if (si->first.find("/ystore/local_dim") != string::npos) scalarCount++;
        else if (si->first.find("/ystore/global_dim") != string::npos) scalarCount++;
        else if (si->first.find("/zstore/local_dim") != string::npos) scalarCount++;
        else if (si->first.find("/zstore/global_dim") != string::npos) scalarCount++;
    }
    
    if (scalarCount < 17)
        return false;

    return true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::IsDataFile
//
// Purpose:
//   See if this file contains data
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
avtSpecFEMFileFormat::IsDataFile(ADIOSFileObject *f)
{
    return true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetRegion
//
// Purpose:
//   Extract the region number from a variable name.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

int
avtSpecFEMFileFormat::GetRegion(const string &str)
{
    int region, n;
    char t1[128];
    
    n = sscanf(str.c_str(), "reg%d/%s", &region, t1);
    if (n != 2 || region < 1 || region > avtSpecFEMFileFormat::NUM_REGIONS)
        EXCEPTION1(ImproperUseException, "Invalid region");
    
    return region;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVariable
//
// Purpose:
//   Extract the variable name from a string with region information in it.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

string
avtSpecFEMFileFormat::GetVariable(const string &str)
{
    char t1[128], t2[128], v[128];
    string::size_type i0 = str.find("/");
    string::size_type i1 = str.rfind("/");
    if (i0 == string::npos || i1 == string::npos)
        EXCEPTION1(ImproperUseException, "Invalid variable");

    return str.substr(i0+1, i1-i0-1);
}

//****************************************************************************
// Function:  convertToLatLon
//
// Purpose:
//   Convert XYZ to Lat Lon Rad.
//
// Programmer:  Dave Pugmire
// Creation:    November 12, 2014
//
// Modifications:
//
//   Dave Pugmire, Wed Dec  3 16:45:27 EST 2014
//   Fix conversion....
//
//****************************************************************************

static inline void
convertToLatLon(double x, double y, double z, double &nx, double &ny, double &nz)
{
    const double twopi = 2.0*M_PI;
    const double toDeg = 180./M_PI;

    double R = sqrt(x*x + y*y + z*z);
    nx = R;
    ny = acos(z/R);
    nz = atan2(y, x);
    if (nz < 0.0)
        nz += twopi;
    
    //nx *= 6371.0; //Convert to km
    ny *= toDeg;
    nz *= toDeg;
}
