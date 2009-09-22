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
//                            avtAdventureFileFormat.C                       //
// ************************************************************************* //

#include <avtAdventureFileFormat.h>

#include <string>

#include <avtDatabaseMetaData.h>
#include <avtVariableCache.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>

#include <DebugStream.h>

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
//  Method: avtAdventureFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
// ****************************************************************************

avtAdventureFileFormat::avtAdventureFileFormat(const char *f)
    : avtSTMDFileFormat(&f, 1), filename(f), usingAdvData(true), initialized(false),
    advData(), mshData()
{
#ifdef MDSERVER
    // We need to initialize on the mdserver so we can see if the file is an
    // Adventure file in the event that the file extension is ".inp". We have
    // to do this because the file format could be Ansys, CEAucd, PATRAN, Dyna3D
    // and we don't want to guess wrong since that would cause this format to
    // gobble up other formats.
    std::string fileExt(filename);
    if(fileExt.size() >= 3)
        fileExt = fileExt.substr(fileExt.size()-3, fileExt.size());
    if(fileExt == "inp")
        Initialize();
#endif
}

// ****************************************************************************
//  Method: avtAdventureFileFormat destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
// ****************************************************************************

avtAdventureFileFormat::~avtAdventureFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtAdventureFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
// ****************************************************************************

void
avtAdventureFileFormat::FreeUpResources(void)
{
    advData.ReleaseData();
    initialized = false;
}

// ****************************************************************************
// Method: avtAdventureFileFormat::Initialize
//
// Purpose: 
//   Initialize the advData object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:06:57 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtAdventureFileFormat::Initialize()
{
    if(!initialized)
    {
        if(filename.size() >= 3 &&
           filename.substr(filename.size()-3, filename.size()) == "msh")
        {
            usingAdvData = false;
            mshData.Open(filename);
        }
        else
        {
            usingAdvData = true;
            advData.Open(filename);
        }
        initialized = true;
    }
}

// ****************************************************************************
//  Method: avtAdventureFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtAdventureFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    Initialize();

    // Add a mesh.
    std::string meshname = "mesh";
    int cellType = 0;
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = meshname;
    mmd->meshType = AVT_UNSTRUCTURED_MESH;
    mmd->spatialDimension = 3;
    mmd->topologicalDimension = 3;
    mmd->blockOrigin = 0;
    mmd->groupOrigin = 0;
    mmd->cellOrigin = 0;
    mmd->blockPieceName = "domain";
    mmd->blockTitle = "domains";

    if(usingAdvData)
    {
        mmd->numBlocks = advData.GetNumDomains();

        // Get the metadata from the ADV files.
        AdvDataSet::VarInfoVector  vars;
        AdvDataSet::AdvElementType elementType;
        advData.GetMetaData(vars, elementType);

        if(elementType == AdvDataSet::ADVENTURE_ELEMENT_TET4)
            cellType = 0;
        else if(elementType == AdvDataSet::ADVENTURE_ELEMENT_TET10)
            cellType = 1;
        else 
            cellType = 2;

        // Add variables that live on the mesh.
        for(size_t i = 0; i < vars.size(); ++i)
        {
            int ncomps = 1;
            if(vars[i].CheckFormat(ncomps))
            {
                if(vars[i].fega_type == "AllElementConstant" ||
                   vars[i].fega_type == "AllElementVariable")
                {
                    if(ncomps == 1)
                        AddScalarVarToMetaData(md, vars[i].label, meshname, AVT_ZONECENT);
                    else if(ncomps == 3)
                        AddVectorVarToMetaData(md, vars[i].label, meshname, AVT_ZONECENT);
                    else if(ncomps == 6)
                        AddSymmetricTensorVarToMetaData(md, vars[i].label, meshname, AVT_ZONECENT, 6);
                }
                else if(vars[i].fega_type == "AllNodeConstant" ||
                        vars[i].fega_type == "AllNodeVariable")
                {
                    // Skip node indexing variables since they are a little special.
                    if(vars[i].label == "NodeIndex_PartToGlobal" ||
                       vars[i].label == "NodeIndex_SubdomainToPart")
                        continue;

                    if(ncomps == 1)
                        AddScalarVarToMetaData(md, vars[i].label, meshname, AVT_NODECENT);
                    else if(ncomps == 3)
                        AddVectorVarToMetaData(md, vars[i].label, meshname, AVT_NODECENT);
                    else if(ncomps == 6)
                        AddSymmetricTensorVarToMetaData(md, vars[i].label, meshname, AVT_NODECENT, 6);
                }
            }
        }
    }
    else
    {
        mmd->numBlocks = mshData.GetNumDomains();
        cellType = mshData.GetCellType();
    }
    md->Add(mmd);

    // Store the cell type in the database comment so we know what it was.
    if(cellType == 0)
        md->SetDatabaseComment("Tet4 mesh");
    else if(cellType == 1)
        md->SetDatabaseComment("Tet10 mesh");
    else 
        md->SetDatabaseComment("Hex8 mesh");
}

// ****************************************************************************
//  Method: avtAdventureFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtAdventureFileFormat::GetMesh(int domain, const char *meshname)
{
    Initialize();

    vtkDataSet *retval = 0;
    if(usingAdvData)
    {
        // Get the mesh
        retval = advData.GetMesh(domain);

        // If we have global node ids on the mesh then put them in the cache too.
        vtkDataArray *globalNodeId = retval->GetPointData()->GetArray("avtGlobalNodeId");
        if(globalNodeId != 0)
        {
            globalNodeId->Register(NULL);
            void_ref_ptr vr = void_ref_ptr(globalNodeId, avtVariableCache::DestructVTKObject);
            cache->CacheVoidRef(meshname, AUXILIARY_DATA_GLOBAL_NODE_IDS, timestep, 
                                domain, vr);
        }

        // We could do the same thing with global zone ids here. I'm not doing it
        // right now because my test data seems to have shoddy global zone ids.
    }
    else
        retval = mshData.GetMesh(domain);

    return retval;
}    

// ****************************************************************************
//  Method: avtAdventureFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtAdventureFileFormat::GetVar(int domain, const char *varname)
{
    Initialize();

    return usingAdvData ? advData.GetVar(domain, varname) : 0;
}

// ****************************************************************************
//  Method: avtAdventureFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
// ****************************************************************************

vtkDataArray *
avtAdventureFileFormat::GetVectorVar(int domain, const char *varname)
{
    Initialize();

    return usingAdvData ? advData.GetVar(domain, varname) : 0;
}
