/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtCCMFileFormat.C                             //
// ************************************************************************* //

#include <avtCCMFileFormat.h>

#include <string>

#include <vtkCellTypes.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidVariableException.h>

#include <DebugStream.h>

using     std::string;


// ****************************************************************************
//  Method: avtCCM constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
// ****************************************************************************

avtCCMFileFormat::avtCCMFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    ccmOpened = false;
    ccmErr = kCCMIONoErr;
}

// ****************************************************************************
//  Method: avtCCM destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
// ****************************************************************************

avtCCMFileFormat::~avtCCMFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtCCMFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
// ****************************************************************************

void
avtCCMFileFormat::FreeUpResources(void)
{
    if(ccmOpened)
    {
        ccmOpened = false;
        CCMIOCloseFile(&ccmErr, ccmRoot);
        ccmErr = kCCMIONoErr;
    }
}

// ****************************************************************************
// Method: avtCCMFileFormat::GetRoot
//
// Purpose: 
//   Opens the file and passes back an id for the "root" node that can be used
//   to read information about the file. We use this instead of ccmRoot so the
//   file can be implicitly opened, if needed.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 6 09:11:07 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

CCMIOID &
avtCCMFileFormat::GetRoot()
{
    const char *mName = "avtCCMFileFormat::GetRoot: ";
    if(!ccmOpened)
    {
        ccmErr = kCCMIONoErr;
        ccmErr = CCMIOOpenFile(NULL, filenames[0],
                               kCCMIORead, &ccmRoot);
        if(ccmErr == kCCMIONoErr)
            ccmOpened = true;
        else
            debug4 << mName << "Could not open CCM file " << filenames[0] << endl;
    }

    return ccmRoot;
}

// ****************************************************************************
// Method: avtCCMFileFormat::GetIDsForDomain
//
// Purpose: 
//   Gets nodes for state, processor, vertices, topology and solution that can
//   be used to query attributes for variables and meshes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 6 09:10:29 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtCCMFileFormat::GetIDsForDomain(int dom, CCMIOID &state, 
    CCMIOID &processor, CCMIOID &vertices, CCMIOID &topology,
    CCMIOID &solution, bool &hasSolution)
{
    CCMIOID desc;
    CCMIOGetState(&ccmErr, GetRoot(), "default", &desc, &state);

    // Try and get the requested processor.
    int proc = dom;
    bool ret = (
        CCMIONextEntity(NULL, state, kCCMIOProcessor, &proc, &processor) ==
        kCCMIONoErr);
    if(ret)
    {
        hasSolution = true;
        // Try and read the vertices, topology, and solution ids for this processor.
        CCMIOReadProcessor(&ccmErr, processor, &vertices, &topology, NULL, &solution);
        if(ccmErr != kCCMIONoErr)
        {
            // That didn't work. See if we can at least get the vertices and processor.
            CCMIOReadProcessor(&ccmErr, processor, &vertices, &topology, NULL, NULL);
            if(ccmErr == kCCMIONoErr)
                hasSolution = false;
            else
                ret = false;
        }
    }

    return ret;
}

// ****************************************************************************
//  Method: avtCCMFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//
// ****************************************************************************

void
avtCCMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtCCMFileFormat::PopulateDatabaseMetaData: ";
#if 0
    // Read the simulation title.
    char *title = NULL;
    ccmErr = CCMIOGetTitle(&ccmErr, GetRoot(), &title);
    if(title != NULL)
    {
        md->SetDatabaseComment(title);
        free(title);
    }
#endif

    // Count the number of processors in the file. Use that for the number of domains.
    CCMIOID state, desc, processor;    
    CCMIOGetState(&ccmErr, GetRoot(), "default", &desc, &state);
    int proc = 0;
    int nblocks = 0;
    while (CCMIONextEntity(NULL, state, kCCMIOProcessor, &proc, &processor) == kCCMIONoErr)
    {
        ++nblocks;
    }
    debug4 << mName << "Found " << nblocks << " domains in the file." << endl;

    // Determine the spatial dimensions.
    int dims = 3;
    CCMIOID vertices, topology, solution;
    bool hasSolution = true;
    if(GetIDsForDomain(0, state, processor, vertices, topology, solution,
                       hasSolution))
    {
        // Read 1 point so we can get the dimensions of the vertex. Geez!
        CCMIOID mapID;
        int dims = 1;
        float tmp[3], scale;
        CCMIOReadVerticesf(&ccmErr, vertices, &dims, &scale, &mapID, tmp, 0, 1);
    }

    // Create a mesh.
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "Mesh";
    mmd->spatialDimension = dims;
    mmd->topologicalDimension = dims;
    mmd->meshType = AVT_POINT_MESH;
    mmd->numBlocks = nblocks;
    md->Add(mmd);
}


// ****************************************************************************
//  Method: avtCCMFileFormat::GetMesh
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
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtCCMFileFormat::GetMesh(int dom, const char *meshname)
{
    vtkUnstructuredGrid *ugrid = NULL;
    CCMIOID state, processor, vertices, topology, solution;
    bool hasSolution = true;

    if(GetIDsForDomain(dom, state, processor, vertices, topology, solution,
                       hasSolution))
    {
        // Read the size of the vertices
        CCMIOSize nnodes = 0;
        CCMIOEntitySize(&ccmErr, vertices, &nnodes, NULL);
        if(ccmErr != kCCMIONoErr)
        {
            EXCEPTION1(InvalidVariableException, meshname);
        }

        // Read 1 point so we can get the dimensions of the vertex. Geez!
        CCMIOID mapID;
        int dims = 1;
        float tmp[3], scale;
        CCMIOReadVerticesf(&ccmErr, vertices, &dims, &scale, &mapID, tmp, 0, 1);
        if(ccmErr != kCCMIONoErr)
        {
            EXCEPTION1(InvalidVariableException, meshname);
        }

        // Allocate VTK memory.
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(nnodes);
        float *pts = (float *)points->GetVoidPointer(0);

        // Read the data into the VTK points.
        if(dims == 2)
        {
            // Read 2D points and convert to 3D, storing into VTK.
            float *pts2d = new float[2 * nnodes];
            CCMIOReadVerticesf(&ccmErr, vertices, &dims, &scale, &mapID, pts2d,
                       0, nnodes);
            float *src = pts2d;
            float *dest = pts;
            for(int i = 0; i < nnodes; ++i)
            {
                *dest++ = *src++;
                *dest++ = *src++;
                *dest++ = 0.;
            }
            delete [] pts2d;
        }
        else
        {
            // Read the data directly into the VTK buffer.
            CCMIOReadVerticesf(&ccmErr, vertices, &dims, &scale, &mapID, pts,
                       0, nnodes);
        }

        CCMIOCloseFile(&ccmErr, vertices);
        CCMIOCloseFile(&ccmErr, topology);
        if(hasSolution)
            CCMIOCloseFile(&ccmErr, solution);

        // Scale the points, according to the scale factor read with the vertices.
        for(int i = 0; i < nnodes; ++i)
        {
            pts[0] *= scale;
            pts[1] *= scale;
            pts[2] *= scale;
            pts += 3;
        }

        // Create a point mesh for now... It looks like they store their cells
        // as a collection of faces to support arbitrary polyhedra. We need to 
        // figure out lists of faces that match a cell and convert them to a
        // zoo cell type or if they belong to a polyhedron then we need to
        // create tets from that cell and create the AVT right arrays to make the
        // Mesh plot and pick work right.
        ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(points);
        points->Delete();
        ugrid->Allocate(nnodes);
        vtkIdType onevertex;
        for(int i = 0; i < nnodes; ++i)
        {
            onevertex = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
        }
    }

    return ugrid;
}


// ****************************************************************************
//  Method: avtCCMFileFormat::GetVar
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
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtCCMFileFormat::GetVar(int domain, const char *varname)
{
    return 0; //YOU MUST IMPLEMENT THIS

    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, varname);
    //

    //
    // If you do have a scalar variable, here is some code that may be helpful.
    //
    // int ntuples = XXX; // this is the number of entries in the variable.
    // vtkFloatArray *rv = vtkFloatArray::New();
    // rv->SetNumberOfTuples(ntuples);
    // for (int i = 0 ; i < ntuples ; i++)
    // {
    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry.
    // }
    //
    // return rv;
    //
}


// ****************************************************************************
//  Method: avtCCMFileFormat::GetVectorVar
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
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtCCMFileFormat::GetVectorVar(int domain, const char *varname)
{
    return 0; //YOU MUST IMPLEMENT THIS
    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, varname);
    //

    //
    // If you do have a vector variable, here is some code that may be helpful.
    //
    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3.
    // int ntuples = XXX; // this is the number of entries in the variable.
    // vtkFloatArray *rv = vtkFloatArray::New();
    // int ucomps = (ncomps == 2 ? 3 : ncomps);
    // rv->SetNumberOfComponents(ucomps);
    // rv->SetNumberOfTuples(ntuples);
    // float *one_entry = new float[ucomps];
    // for (int i = 0 ; i < ntuples ; i++)
    // {
    //      int j;
    //      for (j = 0 ; j < ncomps ; j++)
    //           one_entry[j] = ...
    //      for (j = ncomps ; j < ucomps ; j++)
    //           one_entry[j] = 0.;
    //      rv->SetTuple(i, one_entry); 
    // }
    //
    // delete [] one_entry;
    // return rv;
    //
}
