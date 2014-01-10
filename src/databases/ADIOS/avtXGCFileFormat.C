/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
#include "avtXGCFileFormat.h"
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <avtDatabase.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTriangle.h>
#include <sstream>
#include <VisItStreamUtil.h>

#include <vtkCellLocator.h>
#include <vtkGenericCell.h>

using std::string;

// ****************************************************************************
//  Method: avtEMSTDFileFormat::Identify
//
//  Purpose:
//      Determine if this file is of this flavor.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
avtXGCFileFormat::Identify(const char *fname)
{
    string str(fname);
    if (str.find("xgc.3d") != string::npos ||
        str.find("xgc.fieldp") != string::npos ||
        str.find("xgc.fieldi") != string::npos)
    {
        return true;
    }
    return false;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::CreateInterface
//
//  Purpose:
//      Create an interface for this reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

avtFileFormatInterface *
avtXGCFileFormat::CreateInterface(const char *const *list,
                                  int nList,
                                  int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtXGCFileFormat(list[i*nBlock]);
    
    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::CreateMeshName
//
//  Purpose:
//      Form the mesh name for this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

string
avtXGCFileFormat::CreateMeshName(const string &filename)
{
    string meshname = "";
    
    string::size_type i0 = filename.rfind("xgc.");
    string::size_type i1 = filename.rfind(".bp");
    
    if (i0 != string::npos && i1 != string::npos)
    {
        meshname = filename.substr(0,i0+4) + "mesh.bp";
    }
    return meshname;
}

string
avtXGCFileFormat::CreateDiagName(const string &filename)
{
    string diagname = "";
    
    string::size_type i0 = filename.rfind("xgc.");
    string::size_type i1 = filename.rfind(".bp");
    
    if (i0 != string::npos && i1 != string::npos)
    {
        diagname = filename.substr(0,i0+4) + "oneddiag.bp";
    }
    return diagname;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::IsFieldPFile
//
//  Purpose:
//      Determine if this a fieldp file.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
avtXGCFileFormat::IsFieldPFile(ADIOSFileObject *f)
{
    if (f->Filename().find("xgc.fieldp") == string::npos)
        return false;
    
    std::string xpclass;
    f->GetStringAttr("/XP_CLASS", xpclass);
    if (xpclass != "Node_Data")
        return false;
    int val;
    if (!f->GetIntAttr("/nnode_data", val))
        return false;
    if (!f->GetIntScalar("/nphiP1", val))
        return false;
    
    return true;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::IsFieldIFile
//
//  Purpose:
//      Determine if this a fieldi file.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
avtXGCFileFormat::IsFieldIFile(ADIOSFileObject *f)
{
    if (f->Filename().find("xgc.fieldi") == string::npos)
        return false;

    std::string xpclass;
    f->GetStringAttr("/XP_CLASS", xpclass);
    if (xpclass != "Node_Data")
        return false;
    int val;
    if (!f->GetIntAttr("/nnode_data", val))
        return false;    
    return true;
}

// ****************************************************************************
//  Method: avtXGCFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

avtXGCFileFormat::avtXGCFileFormat(const char *nm)
    : avtMTMDFileFormat(nm)
{
    file = new ADIOSFileObject(nm);
    file->SetResetDimensionOrder();
    
    meshFile = NULL;
    diagFile = NULL;
    initialized = false;
    numNodes = 0;
    numTris = 0;
    numPhi = 0;
}

// ****************************************************************************
//  Method: avtXGCFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

avtXGCFileFormat::~avtXGCFileFormat()
{
    if (file)
        delete file;
    if (meshFile)
        delete meshFile;
    if (diagFile)
        delete diagFile;
    file = NULL;
    meshFile = NULL;
    diagFile = NULL;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

int
avtXGCFileFormat::GetNTimesteps()
{
    return file->NumTimeSteps();
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetCycles
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void
avtXGCFileFormat::GetCycles(std::vector<int> &cycles)
{
    file->GetCycles(cycles);
}


// ****************************************************************************
//  Method: avtXGCFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void
avtXGCFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtXGCFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void
avtXGCFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug5<<"avtXGCFileFormat::PopulateDatabaseMetaData()"<<endl;
    Initialize();
    md->SetFormatCanDoDomainDecomposition(false);

    //Mesh.
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    //mesh->meshType = AVT_POINT_MESH;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->blockTitle = "blocks";
    mesh->blockPieceName = "block";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    AddMeshToMetaData(md, "sep_mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 3, 3);
    AddScalarVarToMetaData(md, "sep", "sep_mesh", AVT_NODECENT);


    AddScalarVarToMetaData(md, "dpot", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "iden", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "eden", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "ddpotdt", "mesh", AVT_NODECENT);
    //AddScalarVarToMetaData(md, "turbulence", "mesh", AVT_NODECENT);
    
    /*

    int numVars = 0;
    if (!file->GetIntAttr("/nnode", numVars))
        EXCEPTION1(InvalidVariableException, "nnode");

    //Add variables.
    for (int i = 0; i < numVars; i++)
    {
        char str[512];
        sprintf(str, "/node_data[%d]/labels", i);
        std::string varLabel;
        if (! file->GetStringAttr(str, varLabel))
            continue;
        
        avtScalarMetaData *smd = new avtScalarMetaData();
        smd->name = varLabel;
        smd->meshName = "mesh";
        smd->centering = AVT_NODECENT;

        std::string unitLabel;
        sprintf(str, "/node_data[%d]/units", i);
        if (file->GetStringAttr(str, unitLabel))
        {
            smd->hasUnits = true;
            smd->units = unitLabel;
        }
        
        sprintf(str, "/node_data[%d]/values", i);
        ADIOSFileObject::varIter vi = file->variables.find(str);
        ADIOSVar v = vi->second;
        
        smd->hasDataExtents = true;
        smd->minDataExtents = v.extents[0];
        smd->maxDataExtents = v.extents[1];

        md->Add(smd);
    }
    */
}


// ****************************************************************************
//  Method: avtXGCFileFormat::GetMesh
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
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

vtkDataSet *
avtXGCFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    debug1 << "avtXGCFileFormat::GetMesh " << meshname << endl;
    Initialize();
    if (!strcmp(meshname, "sep_mesh"))
        return GetSepMesh();

    //Build the mesh.
    meshFile->GetIntAttr("n_n", numNodes);
    meshFile->GetIntAttr("n_t", numTris);
    file->GetIntAttr("nphi", numPhi);
    //Not being read in correctly.....
    numNodes = 56980;
    numTris = 112655;
    numPhi = 32;
    //cout<<meshFile->Filename()<<": NN, NT= "<<numNodes<<" "<<numTris<<" numPhi= "<<numPhi<<endl;

    vtkPoints *pts = NULL;
    meshFile->ReadCoordinates("coordinates/values", timestate, &pts);
    vtkUnstructuredGrid *grid = vtkUnstructuredGrid::New();
    grid->SetPoints(pts);

    vtkDataArray *conn = NULL, *nextNode = NULL;
    meshFile->ReadVariable("cell_set[0]/node_connect_list", timestate, 0, &conn);
    meshFile->ReadVariable("nextnode", timestate, 0, &nextNode);

    int *connPtr = (int *)(conn->GetVoidPointer(0));
    int *nnPtr = (int *)(nextNode->GetVoidPointer(0));
    vtkIdType tri[3];
    for (int i = 0; i < numTris*3; i+=3)
    {
        tri[0] = connPtr[i];
        tri[1] = connPtr[i+1];
        tri[2] = connPtr[i+2];
        grid->InsertNextCell(VTK_TRIANGLE, 3, tri);
    }

    //    numPhi = 1;
    //    return grid;

    pts->Delete();
    //    numPhi = 3;

    //Now, revolve it.
    vtkUnstructuredGrid *grid2 = vtkUnstructuredGrid::New();
    vtkPoints *pts2 = vtkPoints::New();
    pts2->SetNumberOfPoints(numNodes * numPhi);
    grid2->SetPoints(pts2);
    float dPhi = 2.0*M_PI/(float)numPhi;

    //dPhi = 0.25*M_PI/(float)numPhi;
    
    for (int i = 0; i < numPhi; i++)
    {
        //float phi = i*(2.0*M_PI/(float)(numPhi-2));
        float phi = (float)i * dPhi;
        phi = -phi;
        
        double pt[3];
        for (int p = 0; p < numNodes; p++)
        {
            pts->GetPoint(p, pt);

            float R = pt[0];
            float Z = pt[1];

            pt[0] = R*cos(phi);
            pt[1] = R*sin(phi);
            pt[2] = Z;
            pts2->SetPoint(p+i*numNodes, pt );
            

            /*
            float r = sqrt(pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2]);
            
            pt[0] = r*cos(phi);
            pt[1] = pt[1];
            pt[2] = r*sin(phi);
            pts2->SetPoint(p+i*numNodes, pt );
            */
        }
        //cout<<i<<" "<<phi<<" "<<phi*360/(2.0*M_PI)<<endl;
    }

    /*
    vtkIdType v[1];
    for (int i = 0; i < numPhi*numNodes; i++)
    {
        v[0] = i;
        grid2->InsertNextCell(VTK_VERTEX, 1, v);
    }
    return grid2;
    */


    vtkIdType wedge[6];
    for (int i = 0; i < numPhi; i++)
    {
        for (int p = 0; p < numTris*3; p+=3)
        {
            int off = i*(numNodes);
            wedge[0] = connPtr[p+0] + off;
            wedge[1] = connPtr[p+1] + off;
            wedge[2] = connPtr[p+2] + off;

            off = (i+1)*(numNodes);
            int p0 = connPtr[p+0];
            int p1 = connPtr[p+1];
            int p2 = connPtr[p+2];

            p0 = nnPtr[p0];
            p1 = nnPtr[p1];
            p2 = nnPtr[p2];

            if (i == numPhi-1)
                off = 0;
            
            wedge[3] = p0 + off;
            wedge[4] = p1 + off;
            wedge[5] = p2 + off;
            //cout<<"["<<wedge[0]<<" "<<wedge[1]<<" "<<wedge[2]<<" "<<wedge[3]<<" "<<wedge[4]<<" "<<wedge[5]<<"]"<<endl;
            grid2->InsertNextCell(VTK_WEDGE, 6, wedge);
        }
    }
    /*
    nextNode->Delete();
    pts2->Delete();
    grid->Delete();
    conn->Delete();
    */
    return grid2;
}


// ****************************************************************************
//  Method: avtXGCFileFormat::GetVar
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
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

vtkDataArray *
avtXGCFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    debug1 << "avtXGCFileFormat::GetVar " << varname << endl;

    Initialize();
    vtkDataArray *arr = NULL;

    if (!strcmp(varname, "turbulence"))
        return GetTurbulence(timestate, domain);
    if (!strcmp(varname, "sep"))
        return GetSep();

    std::map<std::string, std::string>::const_iterator s = labelToVar.find(varname);
    if (s != labelToVar.end())
    {
        if (! file->ReadVariable(s->second, timestate, 0, &arr))
            EXCEPTION1(InvalidVariableException, s->second);
    }
    
    /*
    cout<<__FILE__<<" "<<__LINE__<<endl;

    vtkDataArray *arr2 = vtkFloatArray::New();
    cout<<__FILE__<<" "<<__LINE__<<endl;
    cout<<"sz= "<<arr->GetNumberOfTuples()<<endl;
    arr2->SetNumberOfTuples(56980);
    cout<<__FILE__<<" "<<__LINE__<<endl;
    for (int i = 0; i < 56980; i++)
        arr2->SetTuple1(i, arr->GetTuple1(i));
    cout<<__FILE__<<" "<<__LINE__<<endl;
    return arr2;
    */
        
    return arr;
}

static std::vector<double>
interpolate(std::vector<double> &x,
            std::vector<double> &y,
            double *xi, int ni)
{
    int n = x.size();
    std::vector<double> yi(ni);
    
    for (int i = 0; i < ni; i++)
    {
        yi[i] = 0.0;
        double val = xi[i];
        if (val <= x[0])
        {
            yi[i] = y[0];
            continue;
        }
        else if ( val >= x[n-1])
        {
            yi[i] = y[n-1];
            continue;
        }
        for (int j = 0; j < n-1; j++)
        {
            if (val >= x[j] && val <= x[j+1])
            {
                double dy = y[j+1]-y[j];
                double t = (xi[i]-x[j])/(x[j+1]-x[j]);
                yi[i] = y[j] + t*dy;
                break;
            }
        }
    }

    return yi;
}

static void 
readTxtFile(std::string fname, std::vector<double> &arr)
{
    std::ifstream ifile(fname.c_str());
    double v;
    while (!ifile.fail())
    {
        ifile >> v;
        arr.push_back(v);
    }
}

vtkDataSet *
avtXGCFileFormat::GetSepMesh()
{
    vtkUnstructuredGrid *ds = vtkUnstructuredGrid::New();

    std::ifstream ifile("sep.txt");
    int n;
    ifile >> n;
    std::vector<double> R, Z;
    double v;
    
    for (int i = 0; i < n; i++)
    {
        ifile >> v;
        R.push_back(v);
        ifile >> v;
        Z.push_back(v);
    }

    numPhi = 32;
    int nPts = n *numPhi;
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nPts);
    ds->SetPoints(pts);

    float dPhi = 2.0*M_PI/(float)numPhi;
    for (int i = 0; i < numPhi; i++)
    {
        float phi = (float)i * dPhi;
        phi = -phi;
        for (int j = 0; j < n; j++)
        {
            pts->SetPoint(i*n+j, R[j]*cos(phi), R[j]*sin(phi), Z[j]);
        }
    }
    vtkIdType quad[4];
    for (int i = 0; i < numPhi; i++)
    {
        for (int j = 0; j < n-1; j++)
        {
            quad[0] = i*n+j + 0;
            quad[1] = i*n+j + 1;
            if (i == numPhi-1)
            {
                quad[2] = 0*n+j + 1;
                quad[3] = 0*n+j + 0;
            }
            else
            {
                quad[2] = (i+1)*n+j + 1;
                quad[3] = (i+1)*n+j + 0;
            }
            ds->InsertNextCell(VTK_QUAD, 4, quad);
        }
    }

    pts->Delete();


    //now do some tests....
    if (0)
    {
        vtkCellLocator *loc = vtkCellLocator::New();
        double p0[3] = {2.4, .2, .2};
        double pt[3] = {10,0,0};
        
        vtkIdType cellId;
        vtkGenericCell *cell = vtkGenericCell::New();
        int subId;
        double dist2, x[3], p[3], t;
        loc->SetDataSet(ds);
        loc->BuildLocator();
        vtkPoints *iPoints = vtkPoints::New();
        vtkIdList *iCells = vtkIdList::New();
        /*
        int numInters = loc->IntersectWithLine(p0, pt, 0.01, t, x, p, subId, cellId, cell);
        cout<<"Num Inters: "<<numInters<<endl;
        */
        loc->FindCellsAlongLine(p0, pt, 0.0, iCells);
        iCells->Print(cout);
        
        //loc->FindClosestPoint(pt, cp, cellID, id, dist2);
        
        //cout<<"Find: "<<dist2<<" "<<cellID<<" "<<id<<endl;
        //cell->Print(cout);

        /*
        vtkPoints *points = vtkPoints::New();
        vtkPolyData *pointsPD = vtkPolyData::New();

        points->InsertNextPoint(pt);
        
        pointsPD->SetPoints(points);
        vtkDataSetSurfaceFilter *srfF = vtkDataSetSurfaceFilter::New();
        srfF->SetInputData(ds);
        srfF->Update();
        srfF->GetOutput()->Print(cout);
        
        vtkSelectEnclosedPoints *sel = vtkSelectEnclosedPoints::New();
        sel->SetSurfaceData(srfF->GetOutput());
        sel->SetInputData(pointsPD);
        for (int i = 0; i < points->GetNumberOfPoints(); i++)
        {
            cout<<i<<" isInside= "<<sel->IsInside(i)<<endl;
        }
        */
    }
    
    return ds;
}

vtkDataArray *
avtXGCFileFormat::GetSep()
{
    vtkFloatArray *var = vtkFloatArray::New();

    vtkDataSet *ds = GetSepMesh();
    int nPts = ds->GetNumberOfPoints();
    ds->Delete();

    var->SetNumberOfTuples(nPts);
    for (int i = 0; i < nPts; i++)
        var->SetTuple1(i, 0.0);
    
    return var;
}

vtkDataArray *
avtXGCFileFormat::GetTurbulence(int ts, int dom)
{
    vtkDataArray *pot0 = NULL, *potm0 = NULL, *eden = NULL, *dpot = NULL, *psi = NULL;
    //vtkDataArray *dpsi = NULL, *temp = NULL, *dens = NULL;
    vtkFloatArray *arr = vtkFloatArray::New();

    file->ReadVariable("dpot", ts, 0, &dpot);
    file->ReadVariable("pot0", ts, 0, &pot0);
    file->ReadVariable("potm0", ts, 0, &potm0);
    file->ReadVariable("eden", ts, 0, &eden);
    meshFile->ReadVariable("psi", ts, 0, &psi);

    std::vector<double> psid, dens, temp1, temp2, temp;
    readTxtFile("psi.txt", psid);
    readTxtFile("dens.txt", dens);
    readTxtFile("temp1.txt", temp1);
    readTxtFile("temp2.txt", temp2);
    temp.resize(temp1.size());
    for (int i = 0; i <temp.size(); i++)
        temp[i] = (temp1[i]+temp2[i])/3.0 * 2.0;

    std::vector<double> te, de;

    cout<<"INTERPOLATE: "<<psid.size()<<" "<<psi->GetNumberOfTuples()<<endl;
    te = interpolate(psid, temp, (double*)psi->GetVoidPointer(0), psi->GetNumberOfTuples());
    de = interpolate(psid, dens, (double*)psi->GetVoidPointer(0), psi->GetNumberOfTuples());

    int nTuples = eden->GetNumberOfTuples();
    arr->SetNumberOfTuples(nTuples);

    std::vector<double> meanEden(numNodes);
    for (int i = 0; i < numNodes; i++)
        meanEden[i] = 0.0f;
    for (int i = 0; i < numPhi; i++)
        for (int j = 0; j < numNodes; j++)
            meanEden[j] += eden->GetTuple1(i*numNodes + j);
    for (int i = 0; i < numNodes; i++)
        meanEden[i] /= (double)numPhi;


    for (int i = 0; i < numPhi; i++)
    {
        for (int j = 0; j < numNodes; j++)
        {
            int idx = i*numNodes+j;
            double v1 = (dpot->GetTuple1(idx) - (potm0->GetTuple1(j)-pot0->GetTuple1(j)));
            v1 = v1 / te[j];
            
            double v2 = (eden->GetTuple1(idx) - meanEden[j]);
            v2 = v2 / de[j];

            double val = v1+v2;
            /*      
            if (val > 1.0)
                val = 10;
            else if (val < -1.0)
                val = -10;
            else if (val > 0.0)
                val = log10(val+0.00000001);
            else if (val < 0.0)
                val = -log10(-val+0.00000001);
            */
            arr->SetTuple1(idx, val);
        }
    }

    //compute std of arr.
    double mean = 0.0;
    for (int i = 0; i < nTuples; i++)
        mean += arr->GetTuple1(i);
    mean /= (double)nTuples;
    
    double variance = 0.0;
    for (int i = 0; i < nTuples; i++)
    {
        double x = arr->GetTuple1(i) - mean;
        variance += x*x;
    }
    double stnD = sqrt(variance/(double)nTuples);

    cout<<"stnD: "<<stnD<<endl;

    pot0->Delete();
    potm0->Delete();
    eden->Delete();
    dpot->Delete();
    psi->Delete();
    

    return arr;






#if 0



    std::vector<double> potDiff(numNodes), meanEden(numNodes);
    
    // var = dpot - (potm0 - pot0)
    for (int i = 0; i < numNodes; i++)
        potDiff[i] = potm0->GetTuple1(i) - pot0->GetTuple1(i);

    for (int i = 0; i < numPhi; i++)
    {
        for (int j = 0; j < numNodes; j++)
        {
            float v = dpot->GetTuple1(i*numNodes+j) - potDiff[j];
            v /= te[j];
            arr->SetTuple1(i*numNodes + j, v);
        }
    }


    //var2 = eden - mean(eden)
    for (int i = 0; i < numNodes; i++)
        meanEden[i] = 0.0f;

    for (int i = 0; i < numPhi; i++)
    {
        for (int j = 0; j < numNodes; j++)
            meanEden[j] += eden->GetTuple1(i*numNodes + j);
    }
    
    for (int i = 0; i < numNodes; i++)
        meanEden[i] /= (double)numPhi;

    for (int i = 0; i < numPhi; i++)
    {
        for (int j = 0; j < numNodes; j++)
        {
            float e = eden->GetTuple1(i*numNodes +j) - meanEden[j];
            e = e / de[j];
            eden->SetTuple1(i*numNodes+j, e);
        }
    }

    for (int i = 0; i < numPhi; i++)
    {
        for (int j = 0; j < numNodes; j++)
        {
            float v = arr->GetTuple1(i*numNodes + j);
            v = v + eden->GetTuple1(i*numNodes + j);
            arr->SetTuple1(i*numNodes + j, v);
        }
    }

    dpot->Delete();
    pot0->Delete();
    potm0->Delete();
    eden->Delete();
    psi->Delete();
    
    return arr;
#endif
}


// ****************************************************************************
//  Method: avtXGCFileFormat::GetVectorVar
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
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

vtkDataArray *
avtXGCFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method: avtXGCFileFormat::Initialize
//
//  Purpose:
//      Initialize the reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void
avtXGCFileFormat::Initialize()
{
    if (! file->Open())
        EXCEPTION0(ImproperUseException);

    if (initialized)
        return;
    
    //Open the mesh file.
    meshFile = new ADIOSFileObject(avtXGCFileFormat::CreateMeshName(file->Filename()));
    meshFile->SetResetDimensionOrder();
    if (! meshFile->Open())
        EXCEPTION0(ImproperUseException);
    diagFile = new ADIOSFileObject(avtXGCFileFormat::CreateDiagName(file->Filename()));
    diagFile->SetResetDimensionOrder();
    if (! diagFile->Open())
        EXCEPTION0(ImproperUseException);

    labelToVar["dpot"] = "dpot";
    labelToVar["iden"] = "iden";
    labelToVar["eden"] = "eden";
    labelToVar["ddpotdt"] = "ddpotdt";

    /*
    //Create the label to variable mapping.
    int numVars = 0;
    if (!file->GetIntAttr("/nnode", numVars))
        EXCEPTION1(InvalidVariableException, "nnode");
    
    for (int i = 0; i < numVars; i++)
    {
        char var[512], label[512];
        sprintf(var, "/node_data[%d]/values", i);
        sprintf(label, "/node_data[%d]/labels", i);
        std::string labelNm;
        if (file->GetStringAttr(label, labelNm))
            labelToVar[labelNm] = var;
    }
    */
    
    initialized = true;
}
