/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
#include "avtPixieFileFormat.h"
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

#include <set>
#include <string>
#include <vector>

static std::string RemoveLeadingSlash(const std::string &str);

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
avtPixieFileFormat::Identify(ADIOSFileObject *f)
{
    std::string schema;
    
    return (f->GetStringAttr("/schema/name", schema) && schema == "Pixie");
}


// ****************************************************************************
//  Method: avtPixieFileFormat::CreateInterface
//
//  Purpose:
//      Create an interface for this reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtFileFormatInterface *
avtPixieFileFormat::CreateInterface(const char *const *list,
                                    int nList,
                                    int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtPixieFileFormat(list[i*nBlock]);
    
    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtPixieFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtPixieFileFormat::avtPixieFileFormat(const char *nm)
    : avtMTMDFileFormat(nm)
{
    file = new ADIOSFileObject(nm);
    initialized = false;
}

// ****************************************************************************
//  Method: avtPixieFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtPixieFileFormat::~avtPixieFileFormat()
{
    delete file;
    file = NULL;
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
avtPixieFileFormat::GetNTimesteps()
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
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

void
avtPixieFileFormat::GetCycles(std::vector<int> &cycles)
{
    std::string nm = "/itime";
    file->GetCycles(nm, cycles);
}


// ****************************************************************************
// Method:  avtPixieFileFormat::GetTimes
//
// Purpose:
//   
// Programmer:  Dave Pugmire
// Creation:    January 26, 2011
//
// ****************************************************************************

void
avtPixieFileFormat::GetTimes(std::vector<double> &times)
{
    std::string nm = "/time";
    file->GetTimes(nm, times);
}


// ****************************************************************************
//  Method: avtPixieFileFormat::FreeUpResources
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
avtPixieFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtPixieFileFormat::PopulateDatabaseMetaData
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
avtPixieFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug5<<"avtPixieFileFormat::PopulateDatabaseMetaData()"<<endl;
    Initialize();
    md->SetFormatCanDoDomainDecomposition(false);

    //Add meshes.
    MeshInfoMap::const_iterator mi;
    for (mi = meshes.begin(); mi != meshes.end(); mi++)
    {
        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = mi->first;
        if (mi->second.isRectilinear)
            mmd->meshType = AVT_RECTILINEAR_MESH;
        else
            mmd->meshType = AVT_CURVILINEAR_MESH;
        
        if (mi->second.dims[2] == 1)
        {
            mmd->spatialDimension = 2;
            mmd->topologicalDimension = 2;
        }
        else
        {
            mmd->spatialDimension = 3;
            mmd->topologicalDimension = 3;
        }
        md->Add(mmd);
    }

    //Add variables.
    VarInfoMap::const_iterator vi;
    for (vi = variables.begin(); vi != variables.end(); vi++)
    {
        avtScalarMetaData *sm = new avtScalarMetaData;
        
        sm->originalName = vi->first;
        sm->name = RemoveLeadingSlash(vi->first);
        sm->meshName = vi->second.mesh;
        sm->hasDataExtents = false;
        sm->centering = AVT_NODECENT;

        md->Add(sm);
    }

    //Add expressions.
    if (rawExpression.size())
    {
        std::string::size_type s = 0;
        while (s != std::string::npos)
        {
            std::string::size_type nexts = rawExpression.find_first_of(";", s);
            std::string exprStr;
            if (nexts != std::string::npos)
            {
                exprStr = std::string(rawExpression, s, nexts-s);
                nexts += 1;
            }
            else
            {
                exprStr = std::string(rawExpression,s, std::string::npos);
            }

            // remove offending chars from exprStr (spaces)
            std::string newExprStr;
            for (int i = 0; i < exprStr.size(); i++)
            {
                if (exprStr[i] != ' ')
                    newExprStr += exprStr[i];
            }

            std::string::size_type t = newExprStr.find_first_of(':');

            Expression vec;
            vec.SetName(std::string(newExprStr,0,t));
            vec.SetDefinition(std::string(newExprStr,t+1,std::string::npos));
            vec.SetType(Expression::VectorMeshVar);
            md->AddExpression(&vec);

            s = nexts;
        }
    }
}


// ****************************************************************************
//  Method: avtPixieFileFormat::GetMesh
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
avtPixieFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    debug1 << "avtPixieFileFormat::GetMesh " << meshname << endl;
    Initialize();

    MeshInfoMap::const_iterator mi = meshes.find(meshname);
    if (mi == meshes.end())
        EXCEPTION1(InvalidVariableException, meshname);

    if (mi->second.isRectilinear)
    {
        vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
        vtkFloatArray *coords[3];

        for (int i = 0; i < 3; i++)
        {
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(mi->second.dims[i]);

            float m = 0.0, M = (float)mi->second.dims[i];
            float c = m, dc = (M-m)/(float)(mi->second.dims[i]);
            
            float *data = (float *)coords[i]->GetVoidPointer(0);
            for (int j = 0; j < mi->second.dims[i]; j++, c+=dc)
                data[j] = c;
        }
        
        int nx = mi->second.dims[0], ny = mi->second.dims[1], nz = mi->second.dims[2];

        grid->SetDimensions(nx, ny, nz);
        grid->SetXCoordinates(coords[0]);
        grid->SetYCoordinates(coords[1]);
        grid->SetZCoordinates(coords[2]);

        coords[0]->Delete();
        coords[1]->Delete();
        coords[2]->Delete();
        return grid;
    }
    else
    {
        vtkFloatArray *coords[3];
        file->ReadVariable("/nodes/X", timestate, &coords[0]);
        file->ReadVariable("/nodes/Y", timestate, &coords[1]);
        file->ReadVariable("/nodes/Z", timestate, &coords[2]);
        
        int nx = mi->second.dims[0], ny = mi->second.dims[1], nz = mi->second.dims[2];

        vtkStructuredGrid *sgrid  = vtkStructuredGrid::New();
        vtkPoints *points = vtkPoints::New();
        sgrid->SetPoints(points);
        points->Delete();
        sgrid->SetDimensions(nx, ny, nz);

        points->SetNumberOfPoints(nx*ny*nz);

        float *pts = (float *) points->GetVoidPointer(0);
        float *tmp = pts;
        float *coord0 = (float *)coords[0]->GetVoidPointer(0);
        float *coord1 = (float *)coords[1]->GetVoidPointer(0);
        float *coord2 = (float *)coords[2]->GetVoidPointer(0);

        for (int k = 0; k < nz; k++)
        {
            for (int j = 0; j < ny; j++)
            {
                for (int i = 0; i < nx; i++)
                {
                    *tmp++ = *coord0++;
                    *tmp++ = *coord1++;
                    *tmp++ = *coord2++;
                }
            }
        }

        coords[0]->Delete();
        coords[1]->Delete();
        coords[2]->Delete();
        
        return sgrid;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtPixieFileFormat::GetVar
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
avtPixieFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    debug1 << "avtPixieFileFormat::GetVar " << varname << endl;

    Initialize();
    VarInfoMap::const_iterator vi = variables.find(varname);
    if (vi == variables.end())
        EXCEPTION1(InvalidVariableException, varname);

    std::string nm = vi->second.fileVarName;
    
    vtkFloatArray *arr = NULL;
    if ( !file->ReadVariable(nm, timestate, &arr))
        EXCEPTION1(InvalidVariableException, varname);
        
    return arr;
}


// ****************************************************************************
//  Method: avtPixieFileFormat::GetVectorVar
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
avtPixieFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method: avtPixieFileFormat::Initialize
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
avtPixieFileFormat::Initialize()
{
    if (! file->Open())
        EXCEPTION0(ImproperUseException);

    if (initialized)
        return;

    int numTS = file->NumTimeSteps();
    std::set<int> timestepsSet;
    ADIOSFileObject::varIter vi;
    
    for (vi = file->variables.begin(); vi != file->variables.end(); vi++)
    {
        const ADIOSVar &v = vi->second;

        if (!IsVariable(v.name) || v.dim == 1)
            continue;

        MeshInfo mi;
        std::string meshname = GetVarMesh(v.name, mi);
        
        VarInfo varInfo;
        varInfo.fileVarName = v.name;
        varInfo.mesh = meshname;
        std::string varName = GetVarName(v.name, varInfo.isTimeVarying);

        if (meshes.find(meshname) == meshes.end())
        {
            meshes[meshname] = mi;
        }

        if (variables.find(v.name) == variables.end())
        {
            variables[varName] = varInfo;
        }

        //Extract the timestep.
        int ts;
        if (numTS > 0 && GetTimeStep(v.name, ts))
            timestepsSet.insert(ts);
    }

    std::set<int>::const_iterator it;
    for (it=timestepsSet.begin(); it != timestepsSet.end(); it++)
        timecycles.push_back(*it);
    std::sort(timecycles.begin(), timecycles.end());

    //Get expressions.
    rawExpression = "";
    file->GetStringScalar("/visit_expressions", rawExpression);

    initialized = true;
}

// ****************************************************************************
//  Method: avtPixieFileFormat::HasCoordinates
//
//  Purpose:
//      Fill in mesh coordinates.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

bool
avtPixieFileFormat::HasCoordinates(const std::string &varNm, std::string *coords)
{
    std::string::size_type index = varNm.rfind("/");
    if (index == std::string::npos)
        return false;

    std::string stub = varNm.substr(0,index);
    
    //Add the special sauce.
    std::string c1Str = stub + "/coords/coord1";
    std::string c2Str = stub + "/coords/coord2";
    std::string c3Str = stub + "/coords/coord3";

    if (file->GetStringAttr(c1Str, coords[0]) &&
        file->GetStringAttr(c2Str, coords[1]) &&
        file->GetStringAttr(c3Str, coords[2]))
    {
        return true;
    }
    
    coords[0] = "";
    coords[1] = "";
    coords[2] = "";
    return false;
}

// ****************************************************************************
//  Method: avtPixieFileFormat::IsVariable
//
//  Purpose:
//      Is this a variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

bool
avtPixieFileFormat::IsVariable(const std::string &vname)
{
    if (vname.find("/cells/") != std::string::npos ||
        vname.find("/nodes/") != std::string::npos)
        return false;
    
    return true;
}

// ****************************************************************************
//  Method: avtPixieFileFormat::GetVarName
//
//  Purpose:
//      Extract variable name.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Handle time varying variables.
//
// ****************************************************************************

std::string
avtPixieFileFormat::GetVarName(const std::string &vname, bool &isTimeVarying)
{
    const std::string timePrefix("/Timestep_");
    std::string::size_type index;
    std::string varname = vname;
    
    if (vname.substr(0, timePrefix.size()) == timePrefix)
    {
        index = vname.find("/", 1);
        varname = vname.substr(index+1, vname.size());
        isTimeVarying = true;
    }
    else
        isTimeVarying = false;
    
    return varname;
}


// ****************************************************************************
//  Method: avtPixieFileFormat::GetVarMesh
//
//  Purpose:
//      Mesh for this variable
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

std::string
avtPixieFileFormat::GetVarMesh(const std::string &vname, MeshInfo &mi)
{
    std::string meshname;
    ADIOSFileObject::varIter vi = file->variables.find(vname);
    const ADIOSVar &v = vi->second;

    char tmp[256];
    if (HasCoordinates(vname, mi.coords))
    {
        SNPRINTF(tmp, 256, "curvemesh_%dx%dx%d",
                 int(v.global[0]),
                 int(v.global[1]),
                 int(v.global[2]));
        
        meshname = tmp;
        mi.isRectilinear = false;
    }
    else
    {
        SNPRINTF(tmp, 256, "mesh_%dx%dx%d",
                 int(v.global[0]),
                 int(v.global[1]),
                 int(v.global[2]));
        
        meshname = tmp;
        mi.isRectilinear = true;
        mi.coords[0] = "/cells/X";
        mi.coords[1] = "/cells/Y";
        mi.coords[2] = "/cells/Z";
    }
    
    for (int i = 0; i < 3; i++)
        mi.dims[i] = v.global[i];
    
    return meshname;
}


// ****************************************************************************
//  Method: avtPixieFileFormat::GetTimeStep
//
//  Purpose:
//      Extract timestep from variable name.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

bool
avtPixieFileFormat::GetTimeStep(const std::string &vname, int &ts)
{
    const std::string prefix("/Timestep_");
    
    ts = -1;
    std::string::size_type index;
    if (vname.substr(0, prefix.size()) == prefix)
    {
        index = vname.find("/", 1);
        ts = atoi(vname.substr(prefix.size(), index-prefix.size()).c_str());
        return true;
    }

    return false;
}

static std::string
RemoveLeadingSlash(const std::string &str)
{
    std::string s;
    if (str[0] == '/')
        s = str.substr(1, str.size());
    else
        s = str;
    return s;
}
