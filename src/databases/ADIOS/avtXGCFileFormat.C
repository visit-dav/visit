/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <avtDatabase.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTriangle.h>

using     std::string;

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
avtXGCFileFormat::Identify(ADIOSFileObject *f)
{
    if (avtXGCFileFormat::IsFieldPFile(f) || avtXGCFileFormat::IsFieldIFile(f))
    {
        //Make sure there is a mesh file.
        string meshfile = avtXGCFileFormat::CreateMeshName(f->Filename());
        ifstream ifile(meshfile.c_str());
        if (ifile.fail())
            return false;
        
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
avtXGCFileFormat::CreateInterface(ADIOSFileObject *f,
                                  const char *const *list,
                                  int nList,
                                  int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtXGCFileFormat(list[i*nBlock], (i==0)?f:NULL);
    
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

avtXGCFileFormat::avtXGCFileFormat(const char *nm, ADIOSFileObject *f)
    : avtMTMDFileFormat(nm)
{
    file = f;
    meshFile = NULL;
    initialized = false;
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
    meshFile = NULL;
    initialized = false;
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
    delete file;
    if (meshFile)
        delete meshFile;
    file = NULL;
    meshFile = NULL;
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
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 2;
    mesh->topologicalDimension = 2;
    mesh->blockTitle = "blocks";
    mesh->blockPieceName = "block";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    int numVars = 0;
    if (!file->GetIntAttr("/nnode_data", numVars))
        EXCEPTION1(InvalidVariableException, "nnode_data");

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

    //Build the mesh.
    int numNodes, numTris;
    meshFile->GetIntAttr("/nnodes", numNodes);
    meshFile->GetIntAttr("/cell_set[0]/ncells", numTris);

    vtkPoints *pts = NULL;
    meshFile->ReadCoordinates("/coordinates/values", timestate, &pts);
    vtkUnstructuredGrid *grid = vtkUnstructuredGrid::New();

    vtkDataArray *conn = NULL;
    meshFile->ReadVariable("/cell_set[0]/node_connect_list", timestate, &conn);

    int *connPtr = (int *)(conn->GetVoidPointer(0));
    vtkIdType tri[3];
    for (int i = 0; i < numTris*3; i+=3)
    {
        tri[0] = connPtr[i];
        tri[1] = connPtr[i+1];
        tri[2] = connPtr[i+2];
        grid->InsertNextCell(VTK_TRIANGLE, 3, tri);
    }

    grid->SetPoints(pts);
    pts->Delete();
    return grid;
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

    std::map<std::string, std::string>::const_iterator s = labelToVar.find(varname);
    if (s != labelToVar.end())
    {
        if (! file->ReadVariable(s->second, timestate, &arr))
            EXCEPTION1(InvalidVariableException, s->second);
    }
        
    return arr;
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
    if (! meshFile->Open())
        EXCEPTION0(ImproperUseException);

    //Create the label to variable mapping.
    int numVars = 0;
    if (!file->GetIntAttr("/nnode_data", numVars))
        EXCEPTION1(InvalidVariableException, "nnode_data");
    
    for (int i = 0; i < numVars; i++)
    {
        char var[512], label[512];
        sprintf(var, "/node_data[%d]/values", i);
        sprintf(label, "/node_data[%d]/labels", i);
        std::string labelNm;
        if (file->GetStringAttr(label, labelNm))
            labelToVar[labelNm] = var;
    }
    
    initialized = true;
}
