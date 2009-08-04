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
//                            avtTecplotBinaryFileFormat.C                   //
// ************************************************************************* //

#include <avtTecplotBinaryFileFormat.h>

#include <string>
//#define COLLAPSE_DUPLICATE_NODES
#ifdef COLLAPSE_DUPLICATE_NODES
#include <set>
#endif

#include <vtkCellTypes.h>
#include <vtkFloatArray.h> 
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <vtkVisItUtility.h>

#include <avtDatabaseMetaData.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <VisItException.h>

#include <avtCallback.h>
#include <DebugStream.h>
#include <snprintf.h>
#include <map>

#include <TecplotFile.h>

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::MatchFormat
//
// Purpose: 
//   Check the file to see if it has a Tecplot binary header.
//
// Arguments:
//   filename : The name of the file to check.
//
// Returns:    True if the file looks like Tecplot binary; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 15:41:29 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
avtTecplotBinaryFileFormat::MatchFormat(const char *filename)
{
    bool isBinary = false;
    FILE *f = 0;
    if((f = fopen(filename, "rb")) != 0)
    {
        char magic[9];
        memset(magic, 0, sizeof(char) * 9);
        if(fread(magic, 1, 8, f) == 8)
        {
            debug4 << "avtTecplotBinaryFileFormat::MatchFormat: Tecplot version: "
                   << (magic+5) << endl;
            isBinary = strncmp(magic, "#!TDV", 5) == 0;
        }
        fclose(f);

        if(!isBinary)
        {
            std::string msg("Tecplot version \"");
            msg += std::string(magic + 5);
            msg += "\" binary files are not supported.";
            debug1 << msg.c_str() << endl;
        }
    }
    return isBinary;
}
 
// ****************************************************************************
//  Method: FilterTecplotNamesForVisIt
//
//  Purpose: Filter Tecplot names for VisIt to remove trailing spaces, and 
//           then characters that VisIt finds objectionable.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

static std::string
FilterTecplotNamesForVisIt(const std::string &input)
{
    // Trim trailing spaces.
    std::string output(input);
    while(output.size() > 0 &&
          output[output.size()-1] == ' ')
    {
        output = output.substr(0, output.size()-1);
    }

    // Replace characters that VisIt might replace with ugly words
    std::map<char,char> replacementChars;
    replacementChars[' '] = '_';
    replacementChars['-'] = '_';
    replacementChars['['] = 'd';
    replacementChars[']'] = 'd';
    replacementChars['{'] = 'd';
    replacementChars['}'] = 'd';
    replacementChars['<'] = 'd';
    replacementChars['>'] = 'd';
    replacementChars[':'] = '_';
    replacementChars['/'] = '_';
    replacementChars['.'] = '_';
    replacementChars['='] = '_';
    for(size_t i = 0; i < output.size(); ++i)
    {
        std::map<char,char>::const_iterator pos = 
            replacementChars.find(output[i]);
        if(pos != replacementChars.end())
        {
            if(pos->second == 'd')
            {
                output = output.substr(0,i) + output.substr(i+1,output.size()-2);
                i--;
            }
            else
                output[i] = pos->second;
        }
    }

    return output;
}

// ****************************************************************************
//  Method: avtTecplotBinaryFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

avtTecplotBinaryFileFormat::avtTecplotBinaryFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1), zoneNameToZoneList(), scalarToZone()
{
    tec = 0;
}


// ****************************************************************************
//  Method: avtTecplotBinaryFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

void
avtTecplotBinaryFileFormat::FreeUpResources(void)
{
    if(tec != 0)
    {
        delete tec;
        tec = 0;
        zoneNameToZoneList.clear();
        scalarToZone.clear();
    }
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::Initialize
//
// Purpose:
//    Called to initialize metadata and to initialize the zoneNameToZoneList
//    map, which contains the multimesh to zoneid mapping.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 6 15:32:40 PST 2008
//
//    Mark C. Miller, Wed Feb 11 17:02:41 PST 2009
//    Removed reference to centering of curve meta data.
// ****************************************************************************

void
avtTecplotBinaryFileFormat::Initialize(TecplotFile *f, avtDatabaseMetaData *md,
    std::map<std::string, std::vector<int> > &zoneToZoneIds,
    std::map<std::string, std::string> &s2z)
{
    std::map<std::vector<int>, std::vector<int> > zoneGroups;

    for(size_t z = 0; z < f->zones.size(); ++z)
    {
        // Based on the zone type, figure out the spatial dims
        int sdim = f->GetNumSpatialDimensions(z);
        int tdim = f->GetNumTopologicalDimensions(z);
        if(f->zones[z].zoneType == ORDERED && tdim == 1)
        {
            if(f->titleAndVars.varNames.size() >= 2 && md != 0)
            {
                avtCurveMetaData *cmd = new avtCurveMetaData;
                cmd->name = f->zones[z].zoneName;
                cmd->xLabel = f->titleAndVars.varNames[0];
                cmd->xUnits = f->titleAndVars.varUnits[0];
                cmd->yLabel = f->titleAndVars.varNames[1];
                cmd->yUnits = f->titleAndVars.varUnits[1];
                md->Add(cmd);
            }
        }
        else
        {
            std::vector<int> k;
            k.push_back((int)f->zones[z].zoneType);
            k.push_back(sdim);
            k.push_back(tdim);
            zoneGroups[k].push_back(z);
            debug4 << "Initialize: zoneGroups("
                   << k[0] << ", "
                   << k[1] << ", "
                   << k[2] << ") += " << z << endl;
        }
    }

    debug4 << "zoneGroups has " << zoneGroups.size() << " zone groups" << endl;
    int meshIndex = 0;
    std::map<std::vector<int>, std::vector<int> >::const_iterator pos;
    for(pos = zoneGroups.begin(); pos != zoneGroups.end(); ++pos,++meshIndex)
    {
        int z = pos->second[0];
        std::string meshName;
        if(zoneGroups.size() > 1)
        {
            char tmp[100];
            SNPRINTF(tmp, 100, "zone%d", meshIndex+1);
            meshName = tmp;
        }
        else
        {
            if(pos->second.size() > 1)
                meshName = "zone";
            else
                meshName = f->zones[z].zoneName;
        }

        avtMeshType mt = AVT_UNSTRUCTURED_MESH;
        if(f->zones[z].zoneType == ORDERED)
            mt = AVT_CURVILINEAR_MESH;

        if(md != 0)
        {
            avtMeshMetaData *mmd = new avtMeshMetaData;
            mmd->name = meshName;
            mmd->meshType = mt;
            mmd->topologicalDimension = pos->first[2];
            mmd->spatialDimension = pos->first[1];
            mmd->blockTitle = "Zones";
            mmd->blockPieceName = "zone";
            mmd->numBlocks = pos->second.size();
            for(int i = 0; i < pos->second.size(); ++i)
                mmd->blockNames.push_back(f->zones[pos->second[i]].zoneName);
            md->Add(mmd);
        }

        // Now that we've created a mesh, add its scalars.
        for(size_t i = 0; i < f->titleAndVars.varNames.size(); ++i)
        {
            std::string scalarName(f->titleAndVars.varNames[i]);
            if(meshIndex > 0)
                scalarName = meshName + "/" + scalarName;
            // Remember which mesh name is used by this scalar.
            s2z[scalarName] = meshName;

            if(md != 0)
            {
                avtScalarMetaData *smd = new avtScalarMetaData;
                smd->name = scalarName;
                smd->meshName = meshName;
                smd->centering = (f->zones[z].centering[i] == 0) ?
                    AVT_NODECENT : AVT_ZONECENT;
                if(f->titleAndVars.varUnits[i].size() > 0)
                {
                    smd->units = f->titleAndVars.varUnits[i];
                    smd->hasUnits = true;
                }

                md->Add(smd);
            }
        }

        // Remember the zoneIds that make up the multimesh.
        zoneToZoneIds[meshName] = pos->second;
    }
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::File
//
// Purpose: 
//   Returns the Tecplot file object that we use to read the data.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 14:24:36 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

TecplotFile *
avtTecplotBinaryFileFormat::File()
{
    if(tec == 0)
    {
        tec = new TecplotFile(filenames[0]);
        // Install a filter for variable and mesh names.
        tec->filterNameCB = FilterTecplotNamesForVisIt;
#ifdef MDSERVER
        // Don't read through the data section on the mdserver.
        tec->DisableReadingDataInformation();
#endif
        if(tec->Read())
        {
            debug4 << *tec;

            // Initialize the zoneNameToZoneList map, which contains the
            // mapping of multimesh names to zoneIds.
            Initialize(tec, 0, zoneNameToZoneList, scalarToZone);
        }
        else
        {
            delete tec;
            tec = 0;
            EXCEPTION1(InvalidFilesException, filenames[0]);
        }
    }
    return tec;
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::GetTime
//
// Purpose: 
//   Return the solution time.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 16:45:04 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

double
avtTecplotBinaryFileFormat::GetTime()
{
    double t = 0.;
    if(File()->zones.size() > 0)
        t = File()->zones[0].solutionTime;
    return t;
}

// ****************************************************************************
//  Method: avtTecplotBinaryFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

void
avtTecplotBinaryFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtTecplotBinaryFileFormat::PopulateDatabaseMetaData: ";
    TecplotFile *f = File();

    // Set the database comment.
    char tecver[100];
    SNPRINTF(tecver, 100, "[Tecplot binary file v%d] ", f->version);
    md->SetDatabaseComment(std::string(tecver) + f->titleAndVars.title);

    // Initialize the metadata. The maps have already been initialized so we
    // pass in some dummy values,
    std::map<std::string, std::vector<int> > tmp;
    std::map<std::string, std::string> tmp2;
    Initialize(f, md, tmp, tmp2);
}

// ****************************************************************************
//  Method: avtTecplotBinaryFileFormat::GetMesh
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
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

vtkDataSet *
avtTecplotBinaryFileFormat::GetMesh(int domain, const char *meshname)
{
#ifdef MDSERVER
    return 0;
#else
    std::map<std::string, std::vector<int> >::const_iterator pos;
    pos = zoneNameToZoneList.find(meshname);
    int zoneId = -1;
    if(pos == zoneNameToZoneList.end() || domain >= pos->second.size())
        zoneId = File()->ZoneNameToIndex(meshname);
    else
        zoneId = pos->second[domain];

    if(zoneId == -1)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    ZoneType zt = File()->zones[zoneId].zoneType;
    vtkDataSet *ds = 0;
    if(zt == ORDERED)
    {
        if(File()->GetNumTopologicalDimensions(zoneId) == 1)
            ds = GetCurve(zoneId);
        else
            ds = GetCurvilinearMesh(zoneId);
    }
    else if(zt == FEPOLYGON || zt == FEPOLYHEDRON)
        ds = GetPolyMesh(zoneId);
    else
        ds = GetUnstructuredMesh(zoneId);

    return ds;
#endif
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::GetPoints
//
// Purpose: 
//   Returns the VTK points for the specified zone.
//
// Arguments:
//   zoneId : The id of the zone whose points we want to return.
//   ndims  : The dimensionality of the mesh.
//
// Returns:    The vtkPoints for the specified mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 14:05:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkPoints *
avtTecplotBinaryFileFormat::GetPoints(int zoneId, int ndims)
{
#ifdef MDSERVER
    return 0;
#else
    // Read the ndims, nnodes, ncells, origin from file.
    int nnodes = File()->zones[zoneId].GetNumNodes();

    // Read the X coordinates from the file.
    float *xarray = new float[nnodes];
    if(!File()->ReadVariableAsFloat(zoneId, File()->CoordinateVariable(0), xarray))
        memset(xarray, 0, nnodes * sizeof(float));

    // Read the Y coordinates from the file.
    float *yarray = new float[nnodes];
    if(!File()->ReadVariableAsFloat(zoneId, File()->CoordinateVariable(1), yarray))
        memset(yarray, 0, nnodes * sizeof(float));

    // Read the Z coordinates from the file.
    float *zarray = 0;
    if(ndims > 2)
    {
        zarray = new float[nnodes];
        if(!File()->ReadVariableAsFloat(zoneId, File()->CoordinateVariable(2), zarray))
             memset(zarray, 0, nnodes * sizeof(float));
    }

    //
    // Create the vtkPoints object and copy points into it.
    //
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(nnodes);
    float *pts = (float *) points->GetVoidPointer(0);
    float *xc = xarray;
    float *yc = yarray;
    float *zc = zarray;
    if(ndims == 3)
    {
        for(int i = 0; i < nnodes; ++i)
        {
            *pts++ = *xc++;
            *pts++ = *yc++;
            *pts++ = *zc++;
        }
    }
    else if(ndims == 2)
    {
        for(int i = 0; i < nnodes; ++i)
        {
            *pts++ = *xc++;
            *pts++ = *yc++;
            *pts++ = 0.;
        }
    }

    // Delete temporary arrays.
    delete [] xarray;
    delete [] yarray;
    delete [] zarray;

    return points;
#endif
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::GetCurve
//
// Purpose: 
//   Constructs a curve
//
// Arguments:
//   zoneId : The id of the zone we want to return.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 16:10:17 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtTecplotBinaryFileFormat::GetCurve(int zoneId)
{
#ifdef MDSERVER
    return 0;
#else
    int nnodes = File()->zones[zoneId].GetNumNodes();
    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nnodes, VTK_FLOAT);
 
    vtkFloatArray *vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nnodes);
    vals->SetName(File()->zones[zoneId].zoneName.c_str());
    rg->GetPointData()->SetScalars(vals);
    vtkDataArray *xc = rg->GetXCoordinates();

    // Read data directly into the curve's arrays.
    File()->ReadVariableAsFloat(zoneId, 
                                File()->titleAndVars.varNames[0],
                                (float *)xc->GetVoidPointer(0));

    File()->ReadVariableAsFloat(zoneId, 
                                File()->titleAndVars.varNames[1],
                                (float *)vals->GetVoidPointer(0));
    vals->Delete();
    return rg;
#endif
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::GetCurvilinearMesh
//
// Purpose: 
//   Returns the zone as a curvilinear mesh
//
// Arguments:
//   zoneId : The id of the zone we want to return.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 14:08:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtTecplotBinaryFileFormat::GetCurvilinearMesh(int zoneId)
{
#ifdef MDSERVER
    return 0;
#else
    TecplotOrderedZone *ordered = (TecplotOrderedZone *)File()->zones[zoneId].zoneData;
    int sdims = File()->GetNumSpatialDimensions(zoneId);
    int ndims = File()->GetNumSpatialDimensions(zoneId);
    int dims[3];
    dims[0] = ordered->iMax;
    dims[1] = ordered->jMax;
    dims[2] = ordered->kMax;
    int nnodes = dims[0]*dims[1]*dims[2];

    //
    // Create the vtkStructuredGrid and vtkPoints objects.
    //
    vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
    vtkPoints         *points = GetPoints(zoneId, sdims);
    sgrid->SetPoints(points);
    sgrid->SetDimensions(dims);
    points->Delete();

    return sgrid;
#endif
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::GetUnstructuredMesh
//
// Purpose: 
//   Returns an unstructured mesh.
//
// Arguments:
//   zoneId : The id of the zone we want to return.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 13:53:50 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtTecplotBinaryFileFormat::GetUnstructuredMesh(int zoneId)
{
#ifdef MDSERVER
    return 0;
#else
    // Read the nnodes, ncells, origin from file.
    int sdims = File()->GetNumSpatialDimensions(zoneId);
    int ncells = File()->zones[zoneId].GetNumElements();
    int origin = (File()->version <= 100) ? 1 : 0;

    // Read in the connectivity array. This example assumes that
    // the connectivity will be stored: type, indices, type,
    // indices, ... and that there will be a type/index list
    // pair for each cell in the mesh.
    int *connectivity = 0;
    File()->ReadFEConnectivity(zoneId, &connectivity);

    //
    // Create the vtkPoints object and copy points into it.
    //
    vtkPoints *points = GetPoints(zoneId, sdims);

    //
    // Create a vtkUnstructuredGrid to contain the point cells.
    //
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
    ugrid->SetPoints(points);
    points->Delete();
    ugrid->Allocate(ncells);
    vtkIdType verts[8];
    int *conn = connectivity;

    // not always...
    int cellType;
    ZoneType zt = File()->zones[zoneId].zoneType;
    if(zt == FEBRICK)
        cellType = VTK_HEXAHEDRON;
    else if(zt == FEQUADRILATERAL)
        cellType = VTK_QUAD;
    else if(zt == FETRIANGLE)
        cellType = VTK_TRIANGLE;

    int vertToCellTypeLUT[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};
    vertToCellTypeLUT[1] = VTK_VERTEX;
    vertToCellTypeLUT[2] = VTK_LINE;
    vertToCellTypeLUT[3] = VTK_TRIANGLE;
    vertToCellTypeLUT[4] = (zt == FEBRICK || zt == FETETRAHEDRON) ? VTK_TETRA : VTK_QUAD;
    vertToCellTypeLUT[5] = VTK_PYRAMID;
    vertToCellTypeLUT[6] = VTK_WEDGE;
    vertToCellTypeLUT[8] = VTK_HEXAHEDRON;
    int maxVerts = TecplotNumNodesForZoneType(zt);

    for(int i = 0; i < ncells; ++i)
    {
        // This is not on by default since VisIt seems to do okay without
        // it and reordering some cells is prone to error.
#ifdef COLLAPSE_DUPLICATE_NODES
        // Since the node list for a cell can contain duplicates, we 
        // must make the list unique and then determine the best cell
        // type to use given the number of nodes that we have.
        int nverts = 0;
        std::set<int> nodeList;
//debug4 << "cell " << i << ": ";
        for(int j = 0; j < maxVerts; ++j)
        {
            int node = conn[j] - origin;
            if(nodeList.find(node) == nodeList.end())
            {
//debug4 << node << " ";
                verts[nverts++] = node;
                nodeList.insert(node);
            }
        }
//debug4 << endl;
#else 
        int nverts = maxVerts;
        for(int j = 0; j < maxVerts; ++j)
            verts[j] = conn[j] - origin;
#endif

        if(nverts > 0 && nverts <= 8 &&
           (cellType = vertToCellTypeLUT[nverts]) != -1)
        {
#ifdef COLLAPSE_DUPLICATE_NODES
            vtkIdType *v = verts;
            vtkIdType verts2[8];
            if(cellType == VTK_PYRAMID)
            {
                // Reorder into VTK node ordering.
                verts2[0] = verts[1];
                verts2[1] = verts[2];
                verts2[2] = verts[3];
                verts2[3] = verts[4];
                verts2[4] = verts[0];
                v = verts2;
            }
            else if(cellType == VTK_WEDGE)
            {
                // Reorder into VTK node ordering.
                verts2[0] = verts[2];
                verts2[1] = verts[0];
                verts2[2] = verts[3];
                verts2[3] = verts[5];
                verts2[4] = verts[1];
                verts2[5] = verts[4];
                v = verts2;
            }

            // Insert the cell into the mesh.
            ugrid->InsertNextCell(cellType, nverts, v);
#else
            // Insert the cell into the mesh.
            ugrid->InsertNextCell(cellType, nverts, verts);
#endif
        }
        else
        {
            delete [] connectivity;
            ugrid->Delete();
            // Other cell type - need to add a case for it.
            // In the meantime, throw exception or if you
            // know enough, skip the cell.
            EXCEPTION1(InvalidVariableException, File()->zones[zoneId].zoneName);
        }

        conn += maxVerts;
    }

    delete [] connectivity;

    return ugrid;
#endif
}

// ****************************************************************************
// Method: avtTecplotBinaryFileFormat::GetPolyMesh
//
// Purpose: 
//   Returns polygonal/polyhedral meshes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 13 13:53:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtTecplotBinaryFileFormat::GetPolyMesh(int zoneId)
{
    avtCallback::IssueWarning("FEPOLYGON and FEPOLYHEDRON not yet implemented.");
    return 0;
}

// ****************************************************************************
//  Method: avtTecplotBinaryFileFormat::GetVar
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
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

vtkDataArray *
avtTecplotBinaryFileFormat::GetVar(int domain, const char *varname)
{
#ifdef MDSERVER
    return 0;
#else
    std::string varName(varname);

    std::map<std::string, std::string>::const_iterator m;
    m = scalarToZone.find(varName);
    if(m == scalarToZone.end())
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
   
    // Now that we have the variable's mesh, look up the zoneId that
    // corresponds to the domain. 
    std::map<std::string, std::vector<int> >::const_iterator pos;
    pos = zoneNameToZoneList.find(m->second);
    if(pos == zoneNameToZoneList.end() || domain >= pos->second.size())
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Now that we have the zoneId and the var name, read the variable.
    int slash = varName.find("/");
    if(slash != -1)
        varName = varName.substr(slash+1, varName.size() - slash);
    int zoneId = pos->second[domain];
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(File()->zones[zoneId].GetNumNodes());
    File()->ReadVariableAsFloat(zoneId, varName, (float*)arr->GetVoidPointer(0));

    return arr;
#endif
}


// ****************************************************************************
//  Method: avtTecplotBinaryFileFormat::GetVectorVar
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
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

vtkDataArray *
avtTecplotBinaryFileFormat::GetVectorVar(int domain, const char *varname)
{
    return 0;
}
