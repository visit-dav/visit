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
//                            avtRAWFileFormat.C                             //
// ************************************************************************* //

#include <avtRAWFileFormat.h>

#include <string>
#include <map>

#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <Utility.h>

#include <snprintf.h>

#include <Expression.h>
#include <DebugStream.h>
#include <TimingsManager.h>

#include <avtDatabaseMetaData.h>

#include <InvalidFilesException.h>
    
// ****************************************************************************
// Class: VertexManager
//
// Purpose:
//   This class manages inserts into a vtkPoints object to make sure that the
//   points are unique.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 08:58:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class VertexManager
{
public:
    VertexManager(vtkPoints *pts)
    {
        points = pts;
    }

    ~VertexManager()
    {
    }

    int GetVertexId(const double *vert)
    {
        char keystr[200];
        SNPRINTF(keystr, 200, "%2.6f,%2.6f,%2.6f",
                (float)vert[0], (float)vert[1], (float)vert[2]);
        std::string key(keystr);
        int ret;
        std::map<std::string, int>::const_iterator pos = 
            vertexNamesToIndex.find(key);
        if(pos != vertexNamesToIndex.end())
        {
            ret = pos->second;
        }
        else
        {
            int index = points->GetNumberOfPoints();
            points->InsertNextPoint(vert);

            vertexNamesToIndex[key] = index;
            ret = index;
        }
        return ret;
    }

private:
    vtkPoints *points; // Does not own this pointer.
    std::map<std::string, int> vertexNamesToIndex;
};

// ****************************************************************************
//  Method: avtRAWFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 28 13:43:19 PST 2007
//
// ****************************************************************************

avtRAWFileFormat::avtRAWFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1), meshes()
{
    initialized = false;
}

// ****************************************************************************
// Method: avtRAWFileFormat destructor
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 29 16:10:09 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtRAWFileFormat::~avtRAWFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtRAWFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 28 13:43:19 PST 2007
//
// ****************************************************************************

void
avtRAWFileFormat::FreeUpResources(void)
{
    for(int i = 0; i < meshes.size(); ++i)
    {
        if(meshes[i].mesh != 0)
            meshes[i].mesh->Delete();
    }
    meshes.clear();
    initialized = false;
}

// ****************************************************************************
// Method: avtRAWFileFormat::NewPD
//
// Purpose: 
//   Creates a new polydata.
//
// Arguments:
//   nCells : The number of cells to allocate.
//
// Returns:    A new polydata.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 29 16:09:43 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkPolyData *
avtRAWFileFormat::NewPD(int nCells)
{
    vtkPolyData *pd = vtkPolyData::New();

    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(nCells * 3);
    pd->SetPoints(pts);
    pts->Delete();

    pd->Allocate(nCells * 3);

    return pd;
}

// ****************************************************************************
// Method: avtRAWFileFormat::ReadFile
//
// Purpose: 
//   Reads the file and makes a list of meshes.
//
// Arguments:
//   name : The name of the file to read.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 29 16:09:20 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtRAWFileFormat::ReadFile(const char *name)
{
    const char *mName = "avtRAWFileFormat::ReadFile: ";
    int total = visitTimer->StartTimer();
    debug4 << mName << endl;

    // Open the file.
    ifstream ifile(name);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, name);
    }

    debug4 << "Opened the file: " << name << endl;

    bool trianglesAdded = false;
#ifndef MDSERVER
    // Make a guess about the number of cells and points based on
    // the size of the file.    
    int nCells = 100;
    VisItStat_t statbuf;
    VisItStat(name, &statbuf);
    VisItOff_t fileSize = statbuf.st_size;
    nCells  = fileSize / (VisItOff_t) 80;

    debug4 << mName << "Guessing there are about " << nCells << " cells" << endl;

    vtkPolyData *currentPD = NewPD(nCells);
    VertexManager *vertexMgr = new VertexManager(currentPD->GetPoints());
#else
    vtkPolyData *currentPD = 0;
#endif

    int domain = 0;
    char   line[1024];
    double pts[9] = {0.,0.,0.,0.,0.,0.,0.,0.,0.};
    int nc = 0;
    for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
    {
        // Get the line
        ifile.getline(line, 1024);

        // Skip leading spaces.
        char *cptr = line;
        while((*cptr == ' ' || *cptr == '\t') &&
              (cptr < (line + 1024)))
           ++cptr;

        if((cptr[0] >= '0' && cptr[0] <= '9') || (cptr[0] == '-')) 
        {
#ifndef MDSERVER
            // We found a line with points. Read the line and add a triangle cell.
            sscanf(cptr, "%lg %lg %lg %lg %lg %lg %lg %lg %lg",
                   &pts[0], &pts[1], &pts[2],
                   &pts[3], &pts[4], &pts[5],
                   &pts[6], &pts[7], &pts[8]);
            vtkIdType ids[3];
            ids[0] = vertexMgr->GetVertexId(pts);
            ids[1] = vertexMgr->GetVertexId(pts + 3);
            ids[2] = vertexMgr->GetVertexId(pts + 6);
            currentPD->InsertNextCell(VTK_TRIANGLE, 3, ids);
#endif
            trianglesAdded = true;
        }
        else
        {
            // If we're hitting a new object name and we've added some triangles
            // then we need to add the currentPD to the meshes vector.
            if(trianglesAdded)
            {
                if(meshes.size() == 0)
                {
                    // We get here when we've created some polydata but have not
                    // seen an object id for it yet. In that case, we add an object
                    // to the list.
                    debug4 << mName << "Adding Object mesh" << endl;
                    domain_data dom;
                    dom.domainName = "Object";
                    dom.mesh = currentPD;
                    meshes.push_back(dom);
                    ++domain;
                }
#ifndef MDSERVER
                else
                {
                    // We get here when we've created some polydata for a tag that
                    // we've seen and we're encountering a new tag.
                    debug4 << mName << "Setting mesh for mesh["
                           << (domain-1) << "]\n";
                    meshes[domain-1].mesh = currentPD;
                }
                currentPD->Squeeze();

                // Reset for a new polydata.
                currentPD = NewPD(nCells);
                delete vertexMgr;
                vertexMgr = new VertexManager(currentPD->GetPoints());
#endif
                trianglesAdded = false;
            }

            // We have the name of a new object.
            domain_data dom;
            int len = strlen(cptr);
            if(len > 0)
            {
                // If the new name is valid then add a mesh entry for it.
                if(cptr[len-1] == '\n')
                    cptr[len-1] = '\0'; // Remove the end of line.
                dom.domainName = std::string(cptr);
                dom.mesh = 0;
                meshes.push_back(dom);
                ++domain;
                debug4 << mName << "Domain " << domain << " is called: " << cptr << endl;
            }
            else
                debug4 << mName << "Bad domain name" << endl;
        }
    }

    // Print out the meshes list
    debug4 << "MESHES\n===========================\n";
    for(int i = 0; i < meshes.size(); ++i)
    {
        debug4 << mName << "Mesh " << i << ": " << meshes[i].domainName.c_str()
               << ", ptr=" << (void*)meshes[i].mesh << endl;
    }

#ifndef MDSERVER
    // Delete the vertex manager if it exists.
    if(vertexMgr != 0)
        delete vertexMgr;
#endif

    debug4 << mName << "end" << endl;

    visitTimer->StopTimer(total, "Loading RAW file");
}

// ****************************************************************************
// Method: avtRAWFileFormat::ActivateTimestep
//
// Purpose: 
//   Called on changes of time step.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 29 16:08:52 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtRAWFileFormat::ActivateTimestep()
{
    if(!initialized)
    {
        ReadFile(filenames[0]);
        initialized = true;
    }
}

// ****************************************************************************
//  Method: avtRAWFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 28 13:43:19 PST 2007
//
// ****************************************************************************

void
avtRAWFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "mesh";
    mmd->meshType = AVT_SURFACE_MESH;
    mmd->spatialDimension = 3;
    mmd->topologicalDimension = 2;
    mmd->numBlocks = meshes.size();
    stringVector names;
    for(int i = 0; i < meshes.size(); ++i)
        names.push_back(meshes[i].domainName);
    mmd->blockNames = names;
    md->Add(mmd);

    const char *expdefs[] = {"coord(mesh)[0]","coord(mesh)[1]","coord(mesh)[2]"};
    const char *expnames[] = {"x", "y", "z"};
    for(int i = 0; i < 3; ++i)
    {
        Expression expr;
        expr.SetName(expnames[i]);
        expr.SetDefinition(expdefs[i]);
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);
    }
}


// ****************************************************************************
//  Method: avtRAWFileFormat::GetMesh
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
//  Creation:   Fri Sep 28 13:43:19 PST 2007
//
// ****************************************************************************

vtkDataSet *
avtRAWFileFormat::GetMesh(int domain, const char *meshname)
{
    meshes[domain].mesh->Register(NULL);
    return meshes[domain].mesh;
}


// ****************************************************************************
//  Method: avtRAWFileFormat::GetVar
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
//  Creation:   Fri Sep 28 13:43:19 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtRAWFileFormat::GetVar(int domain, const char *varname)
{
    return 0;
}


// ****************************************************************************
//  Method: avtRAWFileFormat::GetVectorVar
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
//  Creation:   Fri Sep 28 13:43:19 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtRAWFileFormat::GetVectorVar(int domain, const char *varname)
{
    return 0;
}
