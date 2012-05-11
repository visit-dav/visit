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
//                            avtBATLFileFormat.C                           //
// ************************************************************************* //

#include <avtBATLFileFormat.h>

#include <string>
#include <vector>
#include <algorithm>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include<vtkStructuredGrid.h>
#include <vtkCellType.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <vtkPointData.h>
#include <vtkVisItUtility.h>

#include <float.h>
#include <algorithm>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtVariableCache.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtIntervalTree.h>

#include <DBOptionsAttributes.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

#include <visit-hdf5.h>

#define MDIM 3


using std::find;
using std::string;
using std::vector;

int avtBATLFileFormat::objcnt = 0;

// ****************************************************************************
//  Method:  avtBATLFileFormat::Block::Print
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 27, 2005
//
//  Changes:
//    Randy Hudson, June 12, 2007
//    Added printing of node type, block-center coordinates and processor number.
//
// ****************************************************************************
void
avtBATLFileFormat::Block::Print(ostream &out)
{
    debug5 << "Block::Print Marker 1" << endl;
    out << "---- BLOCK: "<<ID<<endl;
    out << "  level = "<<level<<endl;
    out << "  coordinates of block center = ";
    for (int c=0; c<3; c++)
        out << coords[c]<< " ";
    out << endl;
    out << "  processor number = "<<procnum<<endl;
    out << "  minSpatialExtents = "
        << minSpatialExtents[0] << " , "
        << minSpatialExtents[1] << " , "
        << minSpatialExtents[2] << endl;
    out << "  maxSpatialExtents = "
        << maxSpatialExtents[0] << " , "
        << maxSpatialExtents[1] << " , "
        << maxSpatialExtents[2] << endl;
    out << "  minGlobalLogicalExtents = "
        << minGlobalLogicalExtents[0] << " , "
        << minGlobalLogicalExtents[1] << " , "
        << minGlobalLogicalExtents[2] << endl;
    out << "  maxGlobalLogicalExtents = "
        << maxGlobalLogicalExtents[0] << " , "
        << maxGlobalLogicalExtents[1] << " , "
        << maxGlobalLogicalExtents[2] << endl;
}

// ****************************************************************************
//  Function:  avtBATLFileFormat::InitializeHDF5
//
//  Purpose:   Initialize interaction with the HDF5 library
//
//  Programmer:  Mark C. Miller
//  Creation:    Decmeber 10, 2003
//
//  Modifications:
//      Wed May 9, 2012
//      Changed to acomidate hdf5 1.8.x and changed FLASH to BATL
// ****************************************************************************
void
avtBATLFileFormat::InitializeHDF5(void)
{
    debug5 << "InitializeHDF5 Marker 1" << endl;
    debug5 << "Initializing HDF5 Library" << endl;
    H5open();
    errStack = H5Ecreate_stack();
    H5Eset_auto(errStack,NULL, NULL);
    H5Eclose_stack(errStack);

}

// ****************************************************************************
//  Function:  avtBATLFileFormat::FinalizeHDF5
//
//  Purpose:   End interaction with the HDF5 library
//
//  Programmer:  Mark C. Miller
//  Creation:    March 5, 2007
//
//  Modifications:
//
//    Mark C. Miller, Tue Apr 14 17:21:23 PDT 2009
//    Replaced call to H5close with H5garbage_collect. Calling H5close is
//    problematic as VisIt may have other plugins instantiated which are
//    still interacting with HDF5. Calling garbage collect routine achieves
//    the desired goal of reducing memory usage without effecting other
//    plugins that might still be using the HDF5 library.
//
//    Paul D. Stewart, Wed May 9 2012
//      FLASH to BATL
// ****************************************************************************
void
avtBATLFileFormat::FinalizeHDF5(void)
{
    debug5 << "FinalizeHDF5 Marker 1" << endl;
    debug5 << "Garbage collecting HDF5 Library" << endl;
    H5garbage_collect();
}

// ****************************************************************************
//  Method: avtBATL constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//
//    Mark C. Miller, Mon Mar  5 22:04:50 PST 2007
//    Added initialization of HDF5, simParamsHaveBeenRead
//
//    Randy Hudson, July 5, 2007
//    Moved initialization of numProcessors here from ReadProcessorNumbers()
//
//    Randy Hudson, July 10, 2007
//    Added initialization of file_has_procnum
//
//    Randy Hudson, January 30, 2008
//    Added initialization of fileFormatVersion
//
//    Hank Childs, Thu Dec 17 14:07:52 PST 2009
//    Added database options (for toggling between processors or levels).
//
//    Hank Childs, Sat Mar 20 20:21:47 PDT 2010
//    Initialize new database options options.
//
//    Paul D. Stewart,  Wed May 9 2012
//    Adapted to BATL from the FLASH plugin, Mostly by removing things.
// ****************************************************************************

avtBATLFileFormat::avtBATLFileFormat(const char *cfilename,
                                     DBOptionsAttributes *&opts)
    : avtSTMDFileFormat(&cfilename, 1)
{
    debug5 << "avtBATLFileFormat Marker 1" << endl;
    filename  = cfilename;
    fileId    = -1;
    dimension = 0;
    numBlocks = 0;
    numLevels = 0;
    numProcessors = 0;
    fileFormatVersion = -1;

    showProcessors = opts->GetBool("Show generating processor instead of refinement level");
    newStyleCurves = opts->GetBool("Use new style curve generation");
    addStructuredDomainBoundaries = opts->GetBool("Set up patch abutment information");

    // do HDF5 library initialization on consturction of first instance
    if (avtBATLFileFormat::objcnt == 0)
        InitializeHDF5();
    avtBATLFileFormat::objcnt++;
}


// ****************************************************************************
//  Function:  avtBATLFileFormat::~avtBATLFileFormat
//
//  Purpose:   Destructor; free up resources, including hdf5 lib
//
//  Programmer:  Mark C. Miller
//  Creation:    March 5, 2007
//
//  Modifications:
//  Paul D Stewart, May 9, 2012
//  FLASH to BATL
// ****************************************************************************
avtBATLFileFormat::~avtBATLFileFormat()
{
    debug5 << "~avtBATLFileFormat Marker 1" << endl;
    FreeUpResources();

    // handle HDF5 library termination on descrution of last instance
    avtBATLFileFormat::objcnt--;
    if (avtBATLFileFormat::objcnt == 0)
        FinalizeHDF5();
}

// ****************************************************************************
//  Method: avtBATLFileFormat::ActivateTimestep
//
//  Purpose:
//      Tells the reader to activate the current time step.  This means
//      rebuilding the nesting structure if it is not available.
//
//  Programmer: Hank Childs
//  Creation:   April 28, 2006
//
//  Modifications:
//  Paul D Stewart, May 9 2012
//  FLASH to BATL
// ****************************************************************************

void
avtBATLFileFormat::ActivateTimestep(void)
{
    debug5 << "ActivateTimestep Marker 1" << endl;
    BuildDomainNesting();
}

// ****************************************************************************
//  Method: avtBATLFileFormat::GetCycle
//
//  Purpose: Do as little work on file meta data as possible to get
//  current cycle
//
//  Programmer: Mark C. Miller
//  Creation:   March 5, 2007
//
//  Modifications:
//    Mark C. Miller, Tue Mar  6 23:41:33 PST 2007
//    Removed optimization to avoid re-reading simulation parameters between
//    calls to GetCycle and/or GetTime because we actually need to do only
//    a partial read of them.
//
//    Paul D. Stewart. May 9 2012
//    No need to bother with partial reads for the sake of a couple integers,
//    Also fixed H5F_CLOSE_SEMI warning.
// ****************************************************************************

int
avtBATLFileFormat::GetCycle()
{
    debug5 << "GetCycle Marker 1" << endl;
    hid_t aPlist = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(aPlist, H5F_CLOSE_SEMI);
    hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, aPlist);
    H5Pclose(aPlist);

    if (file_id < 0)
        return INVALID_CYCLE;
    ReadSimulationParameters(file_id);

    H5Fclose(file_id);
    return simParams.nsteps;
}

// ****************************************************************************
//  Method: avtBATLFileFormat::GetCycle
//
//  Purpose: Do as little work on file meta data as possible to get
//  current time
//
//  Programmer: Mark C. Miller
//  Creation:   March 5, 2007
//
//  Modifications:
//    Mark C. Miller, Tue Mar  6 23:41:33 PST 2007
//    Removed optimization to avoid re-reading simulation parameters between
//    calls to GetCycle and/or GetTime because we actually need to do only
//    a partial read of them.
//
//    Paul D. Stewart. May 9 2012
//    No need to bother with partial reads for the sake of a couple integers,
//    Also fixed H5F_CLOSE_SEMI warning.
// ****************************************************************************

double
avtBATLFileFormat::GetTime()
{
    debug5 << "GetTime Marker 1" << endl;
    hid_t aPlist = H5Pcreate(H5P_FILE_ACCESS );
    H5Pset_fclose_degree(aPlist, H5F_CLOSE_SEMI);
    hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, aPlist);
    H5Pclose(aPlist);


    if (file_id < 0)
        return INVALID_TIME;
    ReadSimulationParameters(file_id);
    H5Fclose(file_id);
    return simParams.time;
}


// ****************************************************************************
//  Method: avtBATLFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
//    Randy Hudson, June 23, 2007
//    Added code for vector of leaf blocks for morton curve.
//
//   Paul D. Stewart, May 9, 2012
//   Removed some things that are not necssary for BATL and of course changed
//   FLASH to BATL.
// ****************************************************************************

void
avtBATLFileFormat::FreeUpResources(void)
{
    debug5 << "FreeUpResources Marker 1" << endl;
    if (fileId >= 0)
    {
        H5Fclose(fileId);
        fileId = -1;
    }
    blocks.clear();
    varNames.clear();
    minVals.clear();
    maxVals.clear();
    leafBlocks.clear();
}


// ****************************************************************************
//  Function: QsortCurveSorter
//
//  Purpose:
//      Sorts the left-hand-side location for a curve, keeping the block ID
//      along with the sort.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2010
//
//  Modifications:
//  Paul D. Stewart, May 9 2012
//  Pretty much rewrote this becaus BATL block IDs can't be used for sorting.
//
// ****************************************************************************
typedef struct
{
    int f[3];
    int   b;
} IntInt;


static int
QsortiCoordSorter(const void *arg1, const void *arg2)
{
    IntInt *A = (IntInt *) arg1;
    IntInt *B = (IntInt *) arg2;

    if (A->f[2] < B->f[2])
        return -1;
    else if (B->f[2] < A->f[2])
        return 1;
    else if (B->f[2] == A->f[2])
    {
        if (A->f[1] < B->f[1])
            return -1;
        else if (B->f[1] < A->f[1])
            return 1;
        else if (B->f[1] == A->f[1])
        {
            if (A->f[0] < B->f[0])
                return -1;
            else if (B->f[0] < A->f[0])
                return 1;
        }
    }

    return 0;
}

// ****************************************************************************
//  Method: avtBATLFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
//    Jeremy Meredith, Tue Sep 27 14:23:17 PDT 2005
//    Added support for files containing only particles and no grids.
//
//    Jeremy Meredith, Thu Sep 29 11:12:29 PDT 2005
//    Added conversion of 1D grids to curves.
//
//    Hank Childs, Wed Jan 11 09:40:17 PST 2006
//    Change mesh type to AMR.
//
//    Randy Hudson, Apr 3, 2007
//    Added support for Morton curve.
//
//    Randy Hudson, June 12, 2007
//    Added support for subsets by processor number.
//
//    Randy Hudson, June 18, 2007
//    Added support for creating subsets of Morton curve.
//
//    Randy Hudson, July 19, 2007
//    Added support for concurrent block-level and block-processor subset pairs
//      for both domain and morton curve meshes
//
//    Hank Childs, Fri Dec 11 11:37:48 PST 2009
//    Add support for more efficient AMR data structure.
//
//    Hank Childs, Thu Dec 17 14:07:52 PST 2009
//    Added database options (for toggling between processors or levels).
//
//    Paul D. Stewart
//    I Started with the FLASH plugin code for this method, took some hints
//    from the SAMRAI plugin to get non-cartesian meshes to work and changed
//    the orginization of the if (showProcessors) statements to something
//    that seemed more logical to me.
// ****************************************************************************

void
avtBATLFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadAllMetaData();

    BuildDomainNesting();

    // grids
    //    for block and level SIL categories
    if (numBlocks > 0)
    {
        if (showProcessors)
        {
            avtMeshMetaData *bpmesh = new avtMeshMetaData;
            bpmesh->name = "mesh_blockandproc";
            ;
            bpmesh->originalName = "mesh_blockandproc";


            bpmesh->meshType = AVT_AMR_MESH;
            bpmesh->topologicalDimension = dimension;
            bpmesh->spatialDimension = dimension;
            bpmesh->blockOrigin = 1;
            bpmesh->groupOrigin = 1;

            bpmesh->hasSpatialExtents = true;
            bpmesh->minSpatialExtents[0] = minSpatialExtents[0];
            bpmesh->maxSpatialExtents[0] = maxSpatialExtents[0];
            bpmesh->minSpatialExtents[1] = minSpatialExtents[1];
            bpmesh->maxSpatialExtents[1] = maxSpatialExtents[1];
            bpmesh->minSpatialExtents[2] = minSpatialExtents[2];
            bpmesh->maxSpatialExtents[2] = maxSpatialExtents[2];

            bpmesh->numBlocks = numBlocks;
            bpmesh->blockTitle = "Blocks";
            bpmesh->blockPieceName = "block";
            // Processor number as group
            bpmesh->numGroups = numProcessors;
            bpmesh->groupTitle = "Processor";
            bpmesh->groupPieceName = "processor";
            bpmesh->numGroups = numProcessors;
            vector<int> groupIds(numBlocks);
            vector<string> pieceNames(numBlocks);
            debug5 << "axisLabels" << endl;
            for (size_t l = 0; l < axisLabels.size(); l++)
            {
                if (l == 0)
                    bpmesh->xLabel = axisLabels[l];
                else if (l == 1)
                    bpmesh->yLabel = axisLabels[l];
                else if (l == 2)
                    bpmesh->zLabel = axisLabels[l];
            }


            for (int i = 0; i < numBlocks; i++)
            {
                char tmpName[64];
                sprintf(tmpName,"processor%d,block%d",blocks[i].procnum, blocks[i].ID);
                groupIds[i] = blocks[i].procnum;
                pieceNames[i] = tmpName;
            }
            debug5 << "PopulateDatabaseMetaData Marker 37" << endl;
            bpmesh->blockNames = pieceNames;
            bpmesh->groupIds = groupIds;
            debug5 << "PopulateDatabaseMetaData Marker 38" << endl;


            md->Add(bpmesh);

            // grid variables
            for (int v = 0 ; v < nPlotVars; v++)
            {
                avtScalarMetaData *smd = new avtScalarMetaData;
                smd->name = varNames[v];
                smd->meshName = "amr_mesh";
                smd->centering = AVT_ZONECENT;
                smd->hasUnits = true;
                smd->units = varUnits[v];
                smd->hasDataExtents=true;
                smd->minDataExtents = minVals[v];
                smd->maxDataExtents = maxVals[v];
                md->Add(smd);
            }

            if (isCutFile == 0)
            {
                avtMeshMetaData *mcbpmesh = new avtMeshMetaData;
                mcbpmesh->name = "morton_blockandproc";
                mcbpmesh->originalName = "morton_blockandproc";

                mcbpmesh->meshType = AVT_UNSTRUCTURED_MESH;
                mcbpmesh->topologicalDimension = 1;    //    It's a curve
                mcbpmesh->spatialDimension = dimension;
                mcbpmesh->blockOrigin = 0;
                mcbpmesh->groupOrigin = 0;

                mcbpmesh->hasSpatialExtents = true;
                mcbpmesh->minSpatialExtents[0] = minSpatialExtents[0];
                mcbpmesh->maxSpatialExtents[0] = maxSpatialExtents[0];
                mcbpmesh->minSpatialExtents[1] = minSpatialExtents[1];
                mcbpmesh->maxSpatialExtents[1] = maxSpatialExtents[1];
                mcbpmesh->minSpatialExtents[2] = minSpatialExtents[2];
                mcbpmesh->maxSpatialExtents[2] = maxSpatialExtents[2];


                // DEFINING SUBSETS ON M. CURVE MESH (LIKE THAT ON BASE MESH "mesh")
                // spoof a group/domain mesh for the AMR hierarchy
                mcbpmesh->numBlocks = numBlocks;
                debug5 << "PopulateDatabaseMetaData Marker 5" << endl;
                mcbpmesh->blockTitle = "Blocks";
                mcbpmesh->blockPieceName = "block";
                // Processor number as group
                mcbpmesh->numGroups = numProcessors;
                mcbpmesh->groupTitle = "Processor";
                mcbpmesh->groupPieceName = "processor";
                mcbpmesh->numGroups = numProcessors;
                debug5 << "PopulateDatabaseMetaData Marker 36" << endl;
                vector<int> groupIds(numBlocks);
                vector<string> pieceNames(numBlocks);
                for (int i = 0; i < numBlocks; i++)
                {
                    char tmpName[64];
                    sprintf(tmpName,"processor%d,block%d",blocks[i].procnum, blocks[i].ID);
                    groupIds[i] = blocks[i].procnum;
                    pieceNames[i] = tmpName;
                }
                debug5 << "PopulateDatabaseMetaData Marker 37" << endl;
                mcbpmesh->blockNames = pieceNames;
                mcbpmesh->groupIds = groupIds;
                debug5 << "PopulateDatabaseMetaData Marker 38" << endl;

                md->Add(mcbpmesh);
            }
        }
        else
        {
            avtMeshMetaData *mesh = new avtMeshMetaData;
            mesh->name = "amr_mesh";
            mesh->originalName = "amr_mesh";

            mesh->meshType = AVT_AMR_MESH;
            mesh->topologicalDimension = dimension;
            mesh->spatialDimension = dimension;
            mesh->blockOrigin = 1;
            mesh->groupOrigin = 1;

            mesh->hasSpatialExtents = true;
            mesh->minSpatialExtents[0] = minSpatialExtents[0];
            mesh->maxSpatialExtents[0] = maxSpatialExtents[0];
            mesh->minSpatialExtents[1] = minSpatialExtents[1];
            mesh->maxSpatialExtents[1] = maxSpatialExtents[1];
            mesh->minSpatialExtents[2] = minSpatialExtents[2];
            mesh->maxSpatialExtents[2] = maxSpatialExtents[2];

            mesh->numBlocks = numBlocks;
            mesh->blockTitle = "Blocks";
            mesh->blockPieceName = "block";
            // Processor number as group
            mesh->numGroups = numProcessors;
            mesh->groupTitle = "Level";
            mesh->groupPieceName = "level";
            mesh->numGroups = numProcessors;
            vector<int> groupIds(numBlocks);
            vector<string> pieceNames(numBlocks);
            debug5 << "axisLabels" << endl;
            for (size_t l = 0; l < axisLabels.size(); l++)
            {
                if (l == 0)
                    mesh->xLabel = axisLabels[l];
                else if (l == 1)
                    mesh->yLabel = axisLabels[l];
                else if (l == 2)
                    mesh->zLabel = axisLabels[l];
            }


            for (int i = 0; i < numBlocks; i++)
            {
                char tmpName[64];
                sprintf(tmpName,"level%d,block%d",blocks[i].level, blocks[i].ID);
                groupIds[i] = blocks[i].level;
                pieceNames[i] = tmpName;
            }
            debug5 << "PopulateDatabaseMetaData Marker 37" << endl;
            mesh->blockNames = pieceNames;
            mesh->groupIds = groupIds;
//             mesh->nodesAreCritical = true;
            debug5 << "PopulateDatabaseMetaData Marker 38" << endl;


            md->Add(mesh);

            // grid variables
            for (int v = 0 ; v < nPlotVars; v++)
            {
                avtScalarMetaData *smd = new avtScalarMetaData;
                smd->name = varNames[v];
                smd->meshName = "amr_mesh";
                smd->centering = AVT_ZONECENT;
                smd->hasUnits = true;
                smd->units = varUnits[v];
                smd->hasDataExtents=true;
                smd->minDataExtents = minVals[v];
                smd->maxDataExtents = maxVals[v];
                debug5 << varUnits[v] << endl;
                debug5 << minVals[v] << endl;
                debug5 << maxVals[v] << endl;
                md->Add(smd);
            }

            if (isCutFile == 0)
            {
                avtMeshMetaData *mcblmesh = new avtMeshMetaData;
                mcblmesh->name = "morton_blockandlevel";
                mcblmesh->originalName = "morton_blockandlevel";

                mcblmesh->meshType = AVT_UNSTRUCTURED_MESH;
                mcblmesh->topologicalDimension = 1;    //    It's a curve
                mcblmesh->spatialDimension = dimension;
                mcblmesh->blockOrigin = 0;
                mcblmesh->groupOrigin = 0;

                mcblmesh->hasSpatialExtents = true;
                mcblmesh->minSpatialExtents[0] = minSpatialExtents[0];
                mcblmesh->maxSpatialExtents[0] = maxSpatialExtents[0];
                mcblmesh->minSpatialExtents[1] = minSpatialExtents[1];
                mcblmesh->maxSpatialExtents[1] = maxSpatialExtents[1];
                mcblmesh->minSpatialExtents[2] = minSpatialExtents[2];
                mcblmesh->maxSpatialExtents[2] = maxSpatialExtents[2];


                // DEFINING SUBSETS ON M. CURVE MESH (LIKE THAT ON BASE MESH "mesh")
                // spoof a group/domain mesh for the AMR hierarchy
                mcblmesh->numBlocks = numBlocks;
                debug5 << "PopulateDatabaseMetaData Marker 5" << endl;
                mcblmesh->blockTitle = "Blocks";
                mcblmesh->blockPieceName = "block";
                // Processor number as group
                mcblmesh->numGroups = numProcessors;
                mcblmesh->groupTitle = "Level";
                mcblmesh->groupPieceName = "level";
                mcblmesh->numGroups = numProcessors;
                debug5 << "PopulateDatabaseMetaData Marker 36" << endl;
                vector<int> groupIds(numBlocks);
                vector<string> pieceNames(numBlocks);
                for (int i = 0; i < numBlocks; i++)
                {
                    char tmpName[64];
                    sprintf(tmpName,"level%d,block%d",blocks[i].level, blocks[i].ID);
                    groupIds[i] = blocks[i].level;
                    pieceNames[i] = tmpName;
                }
                debug5 << "PopulateDatabaseMetaData Marker 37" << endl;
                mcblmesh->blockNames = pieceNames;
                mcblmesh->groupIds = groupIds;

                debug5 << "PopulateDatabaseMetaData Marker 38" << endl;

                md->Add(mcblmesh);
            }
        }
    }
    // curves
    if (numBlocks > 0 && dimension == 1)
    {
        // grid variables
        for (size_t v = 0 ; v < varNames.size(); v++)
        {
            avtCurveMetaData *curve = new avtCurveMetaData;
            curve->name = string("curves/") + varNames[v];
            md->Add(curve);
        }

    }

    // Populate cycle and time
    md->SetCycle(timestep, simParams.nsteps);
    md->SetTime(timestep, simParams.time);
    md->SetCycleIsAccurate(true, timestep);
    md->SetTimeIsAccurate(true, timestep);
}

// ****************************************************************************
//  Method: avtBATLFileFormat::GetMesh
//
//  Purpose:
///      Gets the mesh associated with this file.  The mesh is returned as a
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
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
//    Jeremy Meredith, Tue Sep 27 14:23:42 PDT 2005
//    Added "new" style particle support where the HDF5 variable name
//    containing particle data has changed.
//
//    Jeremy Meredith, Thu Sep 29 11:12:50 PDT 2005
//    Added support for converting 1D AMR grids to curves.
//
//    Kathleen Bonnell, Thu Jul 20 11:22:13 PDT 2006
//    Added support for BATL3 formats.
//
//    Randy Hudson, Apr 3, 2007
//    Added support for Morton curve.
//
//    Randy Hudson, June 18, 2007
//    Added support for creating subsets of Morton curve.
//
//    Randy Hudson, July 19, 2007
//    Added support for concurrent block-level and block-processor subset pairs
//      for both domain and morton curve meshes
//
//    Randy Hudson, June 10, 2008
//    Replaced the representation of curves as vtkPolyData with that as
//      vtkRectilinearGrid, to support expressions
//
//    Brad Whitlock, Wed Nov 18 11:10:09 PST 2009
//    Delete double precision particle data.
//
//    Hank Childs, Fri Dec 11 13:15:13 PST 2009
//    Add support for reordering of indices for SIL efficiencies.
//
//    Hank Childs, Fri Mar 19 11:06:49 PDT 2010
//    Do not flatten curves ... leave them as AMR.
//
//    Paul D. Stewart, May 9 2012
//    Combined needlessly repeated code into single if statements and wrote
//    the structured grid part to add support for non-cartesian meshes.
// ****************************************************************************

vtkDataSet *
avtBATLFileFormat::GetMesh(int domain, const char *meshname)
{
    debug5 << "GetMesh Marker 1" << endl;
    ReadAllMetaData();

    if ((string(meshname) == "amr_mesh" or string(meshname) == "mesh_blockandproc") and simParams.typeGeometry < 3)
    {
        // rectilinear mesh
        vtkFloatArray  *coords[3];
        int i;
        for (i = 0 ; i < 3 ; i++)
        {
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(block_ndims[i]);
            for (int j = 0 ; j < block_ndims[i] ; j++)
            {
                if (i+1 > dimension)
                {
                    coords[i]->SetComponent(j, 0, 0);
                }
                else
                {
                    double minExt = blocks[domain].minSpatialExtents[i];
                    double maxExt = blocks[domain].maxSpatialExtents[i];
                    double c = minExt + double(j) *
                               (maxExt-minExt) / double(block_ndims[i]-1);
                    coords[i]->SetComponent(j, 0, c);
                }
            }
        }

        vtkRectilinearGrid  *rGrid = vtkRectilinearGrid::New();
        rGrid->SetDimensions(block_ndims);
        rGrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rGrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rGrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        return rGrid;
    }
    else if ((string(meshname) == "amr_mesh" or string(meshname) == "mesh_blockandproc") and simParams.typeGeometry > 2 )
    {

        debug5 << "GetMesh Marker 3" << endl;
        hid_t varId = H5Dopen(fileId, "NodesX", H5P_DEFAULT);
        if (varId < 0)
        {
            EXCEPTION1(InvalidVariableException, "NodesX");
        }
        debug5 << "GetMesh Marker 4" << endl;
        hid_t spaceId = H5Dget_space(varId);
        hsize_t dims[5];
        hsize_t ndims = H5Sget_simple_extent_dims(spaceId, dims, NULL);

        if (ndims != 4)
        {
            EXCEPTION1(InvalidVariableException, "NodesX");
        }
        debug5 << "GetMesh Marker 5" << endl;
        int ntuples = dims[1]*dims[2]*dims[3];  //dims[0]==numBlocks

        hsize_t start[4];

        hsize_t stride[4], count[4];

        start[0]  = domain;
        start[1]  = 0;
        start[2]  = 0;
        start[3]  = 0;

        stride[0] = 1;
        stride[1] = 1;
        stride[2] = 1;
        stride[3] = 1;

        count[0]  = 1;
        count[1]  = dims[1];
        count[2]  = dims[2];
        count[3]  = dims[3];

        hid_t dataspace = H5Screate_simple(4, dims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, NULL);

        start[0]  = 0;

        hid_t memspace = H5Screate_simple(4, dims, NULL);
        H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

        hid_t raw_data_type = H5Dget_type(varId);
        hid_t data_type = H5Tget_native_type(raw_data_type, H5T_DIR_ASCEND);
        double *x_data = new double[ntuples];
        H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, x_data);
        H5Sclose(dataspace);

        // Done with the space
        H5Sclose(spaceId);

        // Done with the type
        H5Tclose(data_type);
        H5Tclose(raw_data_type);
        // Done with the variable; don't leak it
        H5Dclose(varId);
        //---------------------------------------------------------------------------------
        double *y_data = NULL;
        if (dimension > 1)
        {
            varId = H5Dopen(fileId, "NodesY", H5P_DEFAULT);
            if (varId < 0)
            {
                EXCEPTION1(InvalidVariableException, "NodesY");
            }
            spaceId = H5Dget_space(varId);

            if (ndims != 4)
            {
                EXCEPTION1(InvalidVariableException, "NodesY");
            }
            start[0]  = domain;

            dataspace = H5Screate_simple(4, dims, NULL);
            H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, NULL);

            start[0]  = 0;

            memspace = H5Screate_simple(4, dims, NULL);
            H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

            raw_data_type = H5Dget_type(varId);
            data_type = H5Tget_native_type(raw_data_type, H5T_DIR_ASCEND);

            y_data = new double[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, y_data);

            H5Sclose(dataspace);
            // Done with the space
            H5Sclose(spaceId);

            // Done with the type
            H5Tclose(data_type);
            H5Tclose(raw_data_type);

            // Done with the variable; don't leak it
            H5Dclose(varId);
        }
        //---------------------------------------------------------------------------------
        double *z_data = NULL;
        if (dimension == 3)
        {
            varId = H5Dopen(fileId, "NodesZ", H5P_DEFAULT);
            if (varId < 0)
            {
                EXCEPTION1(InvalidVariableException, "NodesZ");
            }

            spaceId = H5Dget_space(varId);

            if (ndims != 4)
            {
                EXCEPTION1(InvalidVariableException, "NodesZ");
            }
            start[0]  = domain;

            dataspace = H5Screate_simple(4, dims, NULL);
            H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, NULL);

            start[0]  = 0;

            memspace = H5Screate_simple(4, dims, NULL);
            H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

            raw_data_type = H5Dget_type(varId);
            data_type = H5Tget_native_type(raw_data_type, H5T_DIR_ASCEND);
            z_data = new double[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, z_data);
            H5Sclose(dataspace);
            // Done with the space
            H5Sclose(spaceId);
            // Done with the type
            H5Tclose(data_type);
            H5Tclose(raw_data_type);
            // Done with the variable; don't leak it
            H5Dclose(varId);
        }
        vtkFloatArray *pointData = vtkFloatArray::New();
        pointData->SetNumberOfComponents(3);
        pointData->SetNumberOfTuples(ntuples);

        if (dimension == 3)
        {
            for (int p = 0; p<ntuples; p++)
            {
                pointData->SetTuple3(p,x_data[p], y_data[p],z_data[p]);
            }
        }
        else if (dimension == 2)
        {
            for (int p = 0; p<ntuples; p++)
            {
                pointData->SetTuple3(p,x_data[p], y_data[p],0.0);
            }
        }
        else
        {
            for (int p = 0; p<ntuples; p++)
            {
                pointData->SetTuple3(p,x_data[p], 0.0, 0.0);
            }
        }


        //---------------------------------------------------------------------------------
        //
        // Create the vtkStructuredGrid and vtkPoints objects. //
        vtkStructuredGrid *sgrid = vtkStructuredGrid::New();

        vtkPoints *points = vtkPoints::New();
        sgrid->SetDimensions(block_ndims);
        points->SetData(pointData);
        sgrid->SetPoints(points);
        points->Delete();
        pointData->Delete();

        delete[] x_data;
        if (dimension >1)
            delete[] y_data;
        if (dimension == 3)
            delete[] z_data;
        return sgrid;

    }
    else if (strlen(meshname) > 7 && strncmp(meshname, "curves/", 7)==0)
    {
        int b,i;

        //
        // Read the values from the file
        //
        int nvals = block_zdims[0] * numBlocks;
        float *vals = new float[nvals];


        string varname = string(meshname).substr(7);
        hid_t varId = H5Dopen(fileId, varname.c_str(),H5P_DEFAULT);

        H5Dread(varId, H5T_NATIVE_FLOAT, H5S_ALL,H5S_ALL,H5P_DEFAULT, vals);
        H5Dclose(varId);

        if (newStyleCurves)
        {
            if (numBlocks <= 0)
            {
                delete [] vals;
                return NULL;
            }


            IntInt *di = new IntInt[numBlocks];
            for (int b=0; b<numBlocks; b++)
            {
                di[b].f[0] = blocks[b].minSpatialExtents[0];
                di[b].f[1] = blocks[b].minSpatialExtents[1];
                di[b].f[2] = blocks[b].minSpatialExtents[2];
                di[b].b = b;
            }

            vector<double> xvals;
            vector<double> yvals;

            // Sort them from left to right.
            qsort(di, numBlocks, sizeof(IntInt), QsortiCoordSorter);

            vector<int> blocksInProgress;
            blocksInProgress.push_back(di[0].b);
            double curLocation = blocks[di[0].b].minSpatialExtents[0];
            int    lastBlockIdx = 1;
            int    blocksInProgressIdx = 0;
            while (blocksInProgressIdx >= 0)
            {
                int curBlock  = blocksInProgress[blocksInProgressIdx];
                int nextBlock = di[lastBlockIdx].b;
                double goUntilLocation;
                if (lastBlockIdx >= numBlocks)
                {
                    // we are at the last block.
                    goUntilLocation = blocks[curBlock].maxSpatialExtents[0];
                }
                else
                    goUntilLocation = blocks[nextBlock].minSpatialExtents[0];

                bool curBlockWillEnd = false;
                if (goUntilLocation >= blocks[curBlock].maxSpatialExtents[0])
                {
                    curBlockWillEnd = true;
                    goUntilLocation = blocks[curBlock].maxSpatialExtents[0];
                }

                // We know that we want to add points for "curBlock" from "curLocation"
                // to "goUntilLocation".
                if (curLocation < goUntilLocation)
                {
                    double minExt = blocks[curBlock].minSpatialExtents[0];
                    double maxExt = blocks[curBlock].maxSpatialExtents[0];
                    double step = (maxExt-minExt) / block_zdims[0];
                    int start = (int) floor((curLocation-minExt)/step);
                    int stop  = (int) ceil((goUntilLocation-minExt)/step);
                    if (start < 0)
                        start = 0;
                    if (stop >= block_zdims[0])
                        stop = block_zdims[0]-1;
                    for (int j = start ; j <= stop ; j++)
                    {
                        double x = minExt + j*step + step/2.;
                        if (x < curLocation || x > goUntilLocation)
                            continue;
                        xvals.push_back(x);
                        yvals.push_back(vals[curBlock*block_zdims[0] + j]);
                    }
                }
                curLocation = goUntilLocation;

                if (curBlockWillEnd)
                {
                    // continue with the previous block.  If the new block
                    // starts, we will figure that out on the next iteration.
                    // This statement corresponds to a no-op.  However,
                    // if we have no blocks queued up, then the logic would end.
                    // So test for that.
                    if (lastBlockIdx < numBlocks && blocksInProgressIdx <= 0)
                    {
                        blocksInProgress[0] = nextBlock;
                        blocksInProgressIdx = 0;
                        lastBlockIdx++;
                    }
                    else
                        blocksInProgressIdx--;
                }
                else
                {
                    // queue up the old block and put the new block in.
                    blocksInProgressIdx++;
                    blocksInProgress.resize(blocksInProgressIdx+1);
                    blocksInProgress[blocksInProgressIdx] = nextBlock;
                    lastBlockIdx++;
                }
            }

            //
            // Add all of the points to an array.
            //
            int nPts = xvals.size();
            vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
            vtkFloatArray    *valarray = vtkFloatArray::New();
            valarray->SetNumberOfComponents(1);
            valarray->SetNumberOfTuples(nPts);
            valarray->SetName(meshname);
            rg->GetPointData()->SetScalars(valarray);
            vtkDataArray *xc = rg->GetXCoordinates();
            for (int j = 0 ; j < nPts ; j++)
            {
                //  NODE CENTERED:
                xc->SetComponent(j,  0, xvals[j]);

                valarray->SetValue(j, yvals[j]);
            }
            valarray->Delete();

            delete[] vals;

            return rg;
        }
        else
        {
            //
            // Find a sampling
            //
            double ds_size   = maxSpatialExtents[0] - minSpatialExtents[0];
            double step_size = ds_size;

            for (b=0; b<numBlocks; b++)
            {
                double minExt = blocks[b].minSpatialExtents[0];
                double maxExt = blocks[b].maxSpatialExtents[0];
                double step   = (maxExt - minExt) / double(block_zdims[0]);
                if (step < step_size)
                    step_size = step;
            }
            int nsteps = int(.5 + (ds_size / step_size));

            //
            // Create and initialize the sampling arrays
            //
            float *x = new float[nsteps];
            float *y = new float[nsteps];
            float *l = new float[nsteps];

            for (i=0; i<nsteps; i++)
            {
                x[i] = (step_size * (double(i)+.5)) + minSpatialExtents[0];
                y[i] = 0;
                l[i] = -1;
            }

            //
            // Sample each block
            //
            for (b=0; b<numBlocks; b++)
            {
                double minExt = blocks[b].minSpatialExtents[0];
                double maxExt = blocks[b].maxSpatialExtents[0];
                double size   = maxExt - minExt;
                double step   = size / double(block_zdims[0]);

                int first_index = int((minExt - (minSpatialExtents[0]+step_size/2.))/step_size+.99999);
                int last_index  = int((maxExt - (minSpatialExtents[0]+step_size/2.))/step_size);

                if (first_index < 0 || first_index >= nsteps)
                {
                    // Logic error!  Recover.
                    first_index = 0;
                }
                if (last_index < 0 || last_index >= nsteps)
                {
                    // Logic error!  Recover.
                    last_index = nsteps-1;
                }

                for (int j = first_index; j <= last_index; j++)
                {
                    if (blocks[b].level < l[j])
                        continue;

                    int block_index = int((x[j] - minExt) / step);
                    if (block_index < 0 || block_index >= block_zdims[0])
                    {
                        // Logic error! Could be propagated from earlier. Recover.
                        continue;
                    }

                    y[j] = vals[b * block_zdims[0] + block_index];
                    l[j] = blocks[b].level;
                }
            }

            //
            // Create the rectilinear grid
            //   (Lifted from avtCurve2DFileFormat::ReadFile())
            //

            //
            // Add all of the points to an array.
            //
            int nPts = nsteps;
            vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
            vtkFloatArray    *valarray = vtkFloatArray::New();
            valarray->SetNumberOfComponents(1);
            valarray->SetNumberOfTuples(nPts);
            valarray->SetName(meshname);
            rg->GetPointData()->SetScalars(valarray);
            vtkDataArray *xc = rg->GetXCoordinates();
            for (int j = 0 ; j < nPts ; j++)
            {
                //  NODE CENTERED:
                xc->SetComponent(j,  0, x[j]);

                valarray->SetValue(j, y[j]);
            }
            valarray->Delete();

            delete[] vals;
            delete[] x;
            delete[] y;
            delete[] l;

            return rg;
        }
    }
    else if (string(meshname) == "morton_blockandproc" or string(meshname) == "morton_blockandlevel")
    {
        return GetMortonCurveSubset(domain);
    }


    return NULL;
}



// ****************************************************************************
//  Method: avtBATLFileFormat::GetMortonCurveSubset
//          (Version of GetMortonCurve() for subsets)
//
//  Purpose:
//      Build and return the subset, for block "domain", of the Morton space-filling
//         curve as a vtkPolyData object.
//      The morton curve connects leaf blocks.
//      The two segments of the curve that enter and leave block "domain" are built.
//
//  Programmer: Randy Hudson
//  Creation:   June 15, 2007
//
//  Modifications:
//    Brad Whitlock, Wed Nov 18 11:11:37 PST 2009
//    I fixed a bug where points and lines were not deleted.
//
//   Paul D. Stewart, May 9 2012
//    I left this mostly untouched from the FLASH plugin, besides the necessary
//    name changes.
// ****************************************************************************

vtkPolyData *
avtBATLFileFormat::GetMortonCurveSubset(int domain)
{
    debug5 << "GetMortonCurveSubset" << endl;

    vtkPolyData *pdata = vtkPolyData::New();
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *lines = vtkCellArray::New();

    //
    // One cell per line segment, and unique end points (ones that coincide
    // are duplicated in vtkPoints)
    // Read centers of leaf blocks and insert into point object
    //

    //  LEAF-BLOCK MORTON CURVE
    vector<int>::iterator i = find(leafBlocks.begin(), leafBlocks.end(), domain);
    if (i != leafBlocks.end())  //  "domain" is in "leafBlocks"
    {
        if (i == leafBlocks.begin())
        {
            points->InsertPoint(0, blocks[domain].coords[0], blocks[domain].coords[1], blocks[domain].coords[2]);
            points->InsertPoint(1, blocks[*(i+1)].coords[0], blocks[*(i+1)].coords[1], blocks[*(i+1)].coords[2]);
            // Each cell is a (2-point) line segment
            lines->InsertNextCell(2);
            lines->InsertCellPoint(0);
            lines->InsertCellPoint(1);
        }
        else if (i == (leafBlocks.end()-1))
        {
            points->InsertPoint(0, blocks[*(i-1)].coords[0], blocks[*(i-1)].coords[1], blocks[*(i-1)].coords[2]);
            points->InsertPoint(1, blocks[domain].coords[0], blocks[domain].coords[1], blocks[domain].coords[2]);
            // Each cell is a (2-point) line segment
            lines->InsertNextCell(2);
            lines->InsertCellPoint(0);
            lines->InsertCellPoint(1);
        }
        else    //  "domain" is neither end block
        {
            // Interior points are duplicated so neighboring segments don't share them
            points->InsertPoint(0, blocks[*(i-1)].coords[0], blocks[*(i-1)].coords[1], blocks[*(i-1)].coords[2]);
            points->InsertPoint(1, blocks[domain].coords[0], blocks[domain].coords[1], blocks[domain].coords[2]);
            points->InsertPoint(2, blocks[domain].coords[0], blocks[domain].coords[1], blocks[domain].coords[2]);
            points->InsertPoint(3, blocks[*(i+1)].coords[0], blocks[*(i+1)].coords[1], blocks[*(i+1)].coords[2]);
            // Each cell is a (2-point) line segment
            lines->InsertNextCell(2);
            lines->InsertCellPoint(0);
            lines->InsertCellPoint(1);
            lines->InsertNextCell(2);
            lines->InsertCellPoint(2);
            lines->InsertCellPoint(3);
        }

        pdata->SetPoints(points);
        pdata->SetLines(lines);
    }

    points->Delete();
    lines->Delete();

    return pdata;
}

// ****************************************************************************
//  Method: avtBATLFileFormat::GetVar
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
//  Note: because the domains are small, we might get a speed boost by
//        leaving the varId and spaceId open for each variable, maybe even
//        caching things like the memspace
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
//    Jeremy Meredith, Tue Sep 27 14:24:32 PDT 2005
//    Added "new" style particle support where the HDF5 variable name
//    containing particle data has changed.
//
//    Mark C. Miller, Thu Apr  6 17:06:33 PDT 2006
//    Added conditional compilation for hssize_t type
//
//    Randy Hudson, July 19, 2007
//    Added support for concurrent block-level and block-processor subset pairs
//      for both domain and morton curve meshes
//
//    Brad Whitlock, Wed Nov 18 11:14:16 PST 2009
//    Delete particle data arrays!
//
//    Hank Childs, Fri Dec 11 13:15:13 PST 2009
//    Add support for reordering of indices for SIL efficiencies.
//
//    Paul D. Stewart, May 9 2012
//    Deleted the support for reordering of indicies because I couldn't get
//    it to work correctly for spherical meshes and it didn't seem to help
//    the Cartesian case anyway.
// ****************************************************************************

vtkDataArray *
avtBATLFileFormat::GetVar(int visitDomain, const char *vname)
{
    debug5 << "GetVar Marker 1" << endl;
    ReadAllMetaData();

    // Strip prefix (submenu name ("mesh_blockandproc/")) to leave actual var name
    string vn_str = vname;
    size_t pos = vn_str.find("/"); // position of "/" in str
    int domain = visitDomain;
    string vn_substr = vn_str;
    if (pos != string::npos)
        // We have a variable in a subdirectory.  Strip out the '/'
        vn_substr = vn_str.substr (pos+1); // get from just after "/" to the end
    else
    {
        //
        // It's a grid variable
        //

        hid_t varId = H5Dopen(fileId, vn_substr.c_str(), H5P_DEFAULT);
        if (varId < 0)
        {
            EXCEPTION1(InvalidVariableException, vn_substr.c_str());
        }

        hid_t spaceId = H5Dget_space(varId);

        hsize_t dims[4];
        hsize_t ndims = H5Sget_simple_extent_dims(spaceId, dims, NULL);

        if (ndims != 4)
        {
            EXCEPTION1(InvalidVariableException, vn_substr.c_str());
        }

        int ntuples = dims[1]*dims[2]*dims[3];  //dims[0]==numBlocks

        //
        // Set up a data space to read the right domain
        //
#if HDF5_VERSION_GE(1,6,4)
        hsize_t start[5];
#else
        hssize_t start[5];
#endif
        hsize_t stride[5], count[5];

        start[0]  = domain;
        start[1]  = 0;
        start[2]  = 0;
        start[3]  = 0;

        stride[0] = 1;
        stride[1] = 1;
        stride[2] = 1;
        stride[3] = 1;

        count[0]  = 1;
        count[1]  = dims[1];
        count[2]  = dims[2];
        count[3]  = dims[3];

        hid_t dataspace = H5Screate_simple(4, dims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, NULL);

        start[0]  = 0;
        start[1]  = 0;
        start[2]  = 0;
        start[3]  = 0;

        stride[0] = 1;
        stride[1] = 1;
        stride[2] = 1;
        stride[3] = 1;

        count[0]  = 1;
        count[1]  = dims[1];
        count[2]  = dims[2];
        count[3]  = dims[3];

        hid_t memspace = H5Screate_simple(4, dims, NULL);
        H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

        vtkFloatArray * fa = vtkFloatArray::New();
        fa->SetNumberOfTuples(ntuples);
        float *data = fa->GetPointer(0);

        double *d_data;
        int  *i_data;
        unsigned int *ui_data;
        int i;

        hid_t raw_data_type = H5Dget_type(varId);
        hid_t data_type = H5Tget_native_type(raw_data_type, H5T_DIR_ASCEND);
        if (H5Tequal(data_type, H5T_NATIVE_FLOAT)>0)
        {
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, data);
        }
        else if (H5Tequal(data_type, H5T_NATIVE_DOUBLE)>0)
        {
            d_data = new double[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, d_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = d_data[i];
            delete[] d_data;
        }
        else if (H5Tequal(data_type, H5T_NATIVE_INT))
        {
            i_data = new int[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, i_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = i_data[i];
            delete[] i_data;
        }
        else if (H5Tequal(data_type, H5T_NATIVE_UINT))
        {
            ui_data = new unsigned int[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, ui_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = ui_data[i];
            delete[] ui_data;
        }
        else
        {
            // ERROR: UKNOWN TYPE
            EXCEPTION1(InvalidVariableException, vn_substr.c_str());
        }

        // Done with hyperslab
        H5Sclose(dataspace);

        // Done with the space
        H5Sclose(spaceId);

        // Done with the type
        H5Tclose(data_type);
        H5Tclose(raw_data_type);

        // Done with the variable; don't leak it
        H5Dclose(varId);

        return fa;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtBATLFileFormat::GetVectorVar
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
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//  Paul D Stewart, May 9, 2012
//  FLASH to BATL
//
// ****************************************************************************

vtkDataArray *
avtBATLFileFormat::GetVectorVar(int visitDomain, const char *varname)
{
    // Is this necessary?  No.  But I don't want someone to get caught if
    // they ever decide to implement this method.
    return NULL;
}

// ****************************************************************************
//  Method:  avtBATLFileFormat::ReadAllMetaData
//
//  Purpose:
//    Read the metadata if we have not done so yet.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
//    Jeremy Meredith, Tue Sep 27 14:24:45 PDT 2005
//    Added support for files containing only particles, and no grids.
//
//    Randy Hudson, Apr 4, 2007
//    Added call to ReadNodeTypes.
//    Added call to ReadCoordinates.
//
//    Randy Hudson, June 12, 2007
//    Added call to ReadProcessorNumbers.
//
//    Hank Childs, Tue Dec 15 14:02:40 PST 2009
//    Calculate some data members here when we read the blocks.
//
//    Jeremy Meredith, Thu Jan  7 15:36:19 EST 2010
//    Close all open ids when returning an exception.
//
//    Andrew Szymkowiak, Wed Feb 23 22:32:06 PST 2011
//    Zero out patchesPerLevel vector so we don't miscount and end up with
//    SIL generation errors.
//
//    Paul D. Stewart, May 9 2012
//    I probably only kept a few lines of the FLASH code here.  This
//    version just reads a list of numbers.  Also fixed H5f_CLOSE_SEMI
//    error here
//
// ****************************************************************************

void
avtBATLFileFormat::ReadAllMetaData()
{
    debug5 << "ReadAllMetaData Marker 1" << endl;

    if (fileId >= 0)
    {
        return;
    }
    debug5 << "ReadAllMetaData Marker 2" << endl;
    hid_t aPlist = H5Pcreate(H5P_FILE_ACCESS );
    H5Pset_fclose_degree(aPlist, H5F_CLOSE_SEMI);
    fileId = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, aPlist);
    H5Pclose(aPlist);


    if (fileId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "ReadAllMetaData Marker 3" << endl;

    ReadSimulationParameters(fileId);
    debug5 << "ReadAllMetaData Marker 11" << endl;

    blocks.resize(numBlocks);

    debug5 << "ReadAllMetaData Marker 6" << endl;
    if (numBlocks == 0)
    {
        H5Fclose(fileId);
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "ReadAllMetaData Marker 7" << endl;
    if (numBlocks > 0)
    {
        debug5 << "ReadAllMetaData Marker 8" << endl;
        ReadBlockExtents();
        debug5 << "ReadAllMetaData Marker 9" << endl;
        ReadRefinementLevels();
        debug5 << "ReadAllMetaData Marker 10" << endl;
        ReadUnknownNames();
        showUnits = true;
        ReadUnknownUnits();
        ReadAxisLabels();
        debug5 << "ReadAllMetaData Marker 12" << endl;
        DetermineGlobalLogicalExtentsForAllBlocks();
        debug5 << "ReadAllMetaData Marker 14" << endl;
        ReadCoordinates();
        debug5 << "ReadAllMetaData Marker 14" << endl;
        ReadProcessorNumbers();
        debug5 << "ReadAllMetaData Marker 16" << endl;
        if (isCutFile == 0)
        {
            ReadMortonOrdering();
        }
//         ReadBATLToVisItID();
        patchesPerLevel.resize(numLevels);

        debug5 << "ReadAllMetaData Marker 17" << endl;
        for (int b=0; b<numBlocks; b++)
        {
            blocks[b].ID = b+1;  // 1-origin IDs
        }

        int  i;
        for(i = 0; i < numLevels; i++)
        {
            patchesPerLevel[i]=0;
        }
        debug5 << "numBlocks" << endl;
        debug5 << numBlocks << endl;
        for (i = 0; i < numBlocks; i++)
        {
            patchesPerLevel[blocks[i].level]++;
        }
        debug5 << "ReadAllMetaData Marker 17" << endl;

        debug5 << "ReadAllMetaData Marker 20" << endl;
    }
    debug5 << "ReadAllMetaData Marker 21" << endl;
}


// ****************************************************************************
//  Method: avtBATLFileFormat::ReadProcessorNumbers
//  Programmer: Paul D. Stewart, May 9 2012
//  I have a bunch of methods like this.  Just a generic hdf5 read.
// ****************************************************************************

void avtBATLFileFormat::ReadProcessorNumbers()
{
    debug5 << "ReadProcessorNumbers Marker 1" << endl;
    //
    // Read the bounding box description for the blocks
    //
    hid_t procnumId = H5Dopen(fileId, "Processor Number", H5P_DEFAULT);
    if (procnumId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "ReadProcessorNumbers Marker 2" << endl;
    hid_t procnumSpaceId = H5Dget_space(procnumId);

    hsize_t procnum_dims[1];
    hsize_t procnum_ndims = H5Sget_simple_extent_dims(procnumSpaceId,
                            procnum_dims,NULL);
    debug5 << "ReadProcessorNumbers Marker 3" << endl;
    if (procnum_ndims != 1 || procnum_dims[0] != numBlocks)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t procnum_raw_data_type = H5Dget_type(procnumId);
    hid_t procnum_data_type = H5Tget_native_type(procnum_raw_data_type,
                              H5T_DIR_ASCEND);
    debug5 << "ReadProcessorNumbers Marker 4" << endl;
    int *procnum_array = new int[numBlocks];
    H5Dread(procnumId, procnum_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            procnum_array);
    debug5 << "ReadProcessorNumbers Marker 5" << endl;
    blocksPerProc.resize(numProcessors);
    for (int b=0; b<numProcessors; b++)
        blocksPerProc[b] = 0;

    for (int b=0; b<numBlocks; b++)
    {
        int procnum = procnum_array[b];
        blocks[b].procnum = procnum;
        blocksPerProc[procnum]++;
    }
    debug5 << "ReadProcessorNumbers Marker 6" << endl;
    // Done with the type
    H5Tclose(procnum_data_type);
    H5Tclose(procnum_raw_data_type);
    debug5 << "ReadProcessorNumbers Marker 7" << endl;
    // Done with the space
    H5Sclose(procnumSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(procnumId);
    debug5 << "ReadProcessorNumbers Marker 8" << endl;
    // Delete the raw array
    delete[] procnum_array;
    debug5 << "ReadProcessorNumbers Marker 9" << endl;
}


// ****************************************************************************
//  Method: avtBATLFileFormat::ReadMortonOrdering
//
//  Programmer: Paul D. Stewart, May 9 2012
//  I have a bunch of methods like this.  Just a generic hdf5 read.
// ****************************************************************************

void avtBATLFileFormat::ReadMortonOrdering()
{
    debug5 << "ReadMortonOrdering Marker 1" << endl;
    //
    // Read the bounding box description for the blocks
    //
    hid_t mortonidxId = H5Dopen(fileId, "iMortonNode_A", H5P_DEFAULT);
    if (mortonidxId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "ReadMortonOrdering Marker 2" << endl;
    hid_t mortonidxSpaceId = H5Dget_space(mortonidxId);

    hsize_t mortonidx_dims[1];
    hsize_t mortonidx_ndims = H5Sget_simple_extent_dims(mortonidxSpaceId,
                              mortonidx_dims,NULL);
    debug5 << "ReadMortonOrdering Marker 3" << endl;
    if (mortonidx_ndims != 1 || mortonidx_dims[0] != numBlocks)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t mortonidx_raw_data_type = H5Dget_type(mortonidxId);
    hid_t mortonidx_data_type = H5Tget_native_type(mortonidx_raw_data_type,
                                H5T_DIR_ASCEND);
    debug5 << "ReadMortonOrdering Marker 4" << endl;
    int *mortonidx_array = new int[numBlocks];
    H5Dread(mortonidxId, mortonidx_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            mortonidx_array);


    leafBlocks.resize(numBlocks);
    mortonOrder.resize(numBlocks);
    debug5 << "ReadMortonOrdering Marker 5" << endl;
    for (int b=0; b<numBlocks; b++)
    {
        int mortonidx = mortonidx_array[b];
        leafBlocks[mortonidx-1] = b;
    }
    debug5 << "ReadMortonOrdering Marker 6" << endl;
    // Done with the type
    H5Tclose(mortonidx_data_type);
    H5Tclose(mortonidx_raw_data_type);
    debug5 << "ReadMortonOrdering Marker 7" << endl;
    // Done with the space
    H5Sclose(mortonidxSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(mortonidxId);
    debug5 << "ReadMortonOrdering Marker 8" << endl;
    // Delete the raw array
    delete[] mortonidx_array;
    debug5 << "ReadMortonOrdering Marker 9" << endl;
}


// ****************************************************************************
//  Method: avtBATLFileFormat::ReadCoordinates
//
//  Programmer: Paul D. Stewart, May 9 2012
//  I have a bunch of methods like this.  Just a generic hdf5 read.
//
// ****************************************************************************
void avtBATLFileFormat::ReadCoordinates()
{
    debug5 << "ReadCoordinates Marker 1" << endl;
    //
    // Read the coordinates description for the blocks
    //
    hid_t coordinatesId = H5Dopen(fileId, "coordinates",H5P_DEFAULT);
    if (coordinatesId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t coordinatesSpaceId = H5Dget_space(coordinatesId);

    hsize_t coordinates_dims[2];
    hsize_t coordinates_ndims = H5Sget_simple_extent_dims(coordinatesSpaceId, coordinates_dims, NULL);

    if (coordinates_ndims != 2 ||
            coordinates_dims[0] != numBlocks ||
            coordinates_dims[1] != dimension)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    double *coordinates_array = new double[numBlocks * dimension];
    H5Dread(coordinatesId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, coordinates_array);

    for (int b=0; b<numBlocks; b++)
    {
        double *coords = &coordinates_array[dimension*b];
        for (int d=0; d<3; d++)
        {
            if (d < dimension)
            {
                blocks[b].coords[d] = coords[d];
            }
            else
            {
                blocks[d].coords[d] = 0;
            }
        }

    }
    // Delete the raw array
    delete[] coordinates_array;

    // Done with the space
    H5Sclose(coordinatesSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(coordinatesId);
}


// ****************************************************************************
//  Method:  avtBATLFileFormat::ReadBlockExtents
//
//  Purpose:
//    Read the spatial extents for the blocks.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 27 14:26:41 PDT 2005
//    Force read data type to double; in other words, don't assume it was
//    saved as double precision values.
//
//    Randy Hudson, February 11, 2008
//    Changed code to distinguish between file format versions 8 and 9.
//
//    Paul D. Stewart, May 9, 2012
//    Took out what BATL had no need for.
// ****************************************************************************
void avtBATLFileFormat::ReadBlockExtents()
{
    debug5 << "ReadBlockExtents Marker 1" << endl;
    //
    // Read the bounding box description for the blocks
    //
    hid_t bboxId = H5Dopen(fileId, "bounding box",H5P_DEFAULT);
    if (bboxId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t bboxSpaceId = H5Dget_space(bboxId);

    hsize_t bbox_dims[3];
    hsize_t bbox_ndims = H5Sget_simple_extent_dims(bboxSpaceId, bbox_dims, NULL);
    if (bbox_ndims != 3 ||
            bbox_dims[0] != numBlocks ||
            bbox_dims[1] != dimension ||
            bbox_dims[2] != 2)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    double *bbox_array = new double[numBlocks * dimension * 2];
    H5Dread(bboxId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bbox_array);

    for (int b=0; b<numBlocks; b++)
    {
        double *bbox_line = &bbox_array[dimension*2*b];
        for (int d=0; d<3; d++)
        {
            if (d < dimension)
            {
                blocks[b].minSpatialExtents[d] = bbox_line[d*2 + 0];
                blocks[b].maxSpatialExtents[d] = bbox_line[d*2 + 1];
            }
            else
            {
                blocks[b].minSpatialExtents[d] = 0;
                blocks[b].maxSpatialExtents[d] = 0;
            }
        }
    }
    // Delete the raw array
    delete[] bbox_array;

    // Done with the space
    H5Sclose(bboxSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(bboxId);
}


// ****************************************************************************
//  Method:  avtBATLFileFormat::ReadRefinementLevels
//
//  Programmer: Paul D. Stewart, May 9 2012
//  I have a bunch of methods like this.  Just a generic hdf5 read.
//
// ****************************************************************************
void avtBATLFileFormat::ReadRefinementLevels()
{
    debug5 << "ReadRefinementLevels Marker 1" << endl;
    //
    // Read the bounding box description for the blocks
    //
    hid_t refinementId = H5Dopen(fileId, "refine level",H5P_DEFAULT);
    if (refinementId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "ReadRefinementLevels Marker 2" << endl;
    hid_t refinementSpaceId = H5Dget_space(refinementId);

    hsize_t refinement_dims[1];
    hsize_t refinement_ndims = H5Sget_simple_extent_dims(refinementSpaceId,
                               refinement_dims,NULL);
    debug5 << "ReadRefinementLevels Marker 3" << endl;
    if (refinement_ndims != 1 || refinement_dims[0] != numBlocks)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t refinement_raw_data_type = H5Dget_type(refinementId);
    hid_t refinement_data_type = H5Tget_native_type(refinement_raw_data_type,
                                 H5T_DIR_ASCEND);
    debug5 << "ReadRefinementLevels Marker 4" << endl;
    int *refinement_array = new int[numBlocks];
    H5Dread(refinementId, refinement_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            refinement_array);
    debug5 << "ReadRefinementLevels Marker 5" << endl;
    for (int b=0; b<numBlocks; b++)
    {
        int level = refinement_array[b];
        blocks[b].level = level;
    }
    debug5 << "ReadRefinementLevels Marker 6" << endl;
    // Done with the type
    H5Tclose(refinement_data_type);
    H5Tclose(refinement_raw_data_type);
    debug5 << "ReadRefinementLevels Marker 7" << endl;
    // Done with the space
    H5Sclose(refinementSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(refinementId);
    debug5 << "ReadRefinementLevels Marker 8" << endl;
    // Delete the raw array
    delete[] refinement_array;
    debug5 << "ReadRefinementLevels Marker 9" << endl;
}

// // ****************************************************************************
// //  Method:  avtBATLFileFormat::ReadSimulationParameters
// //
// //  Purpose:
// //    Read the simulation parameters from the file.
// //
// //  Arguments:
// //    none
// //
// //  Programmer: Paul Stewart
// //  Creation: Jan 23 2012
// //
// //
// // ****************************************************************************
void avtBATLFileFormat::ReadSimulationParameters(hid_t fileId)
{

    debug5 << "ReadSimulationParameters Marker 1" << endl;
    //
    // Read the bounding box description for the blocks
    //
    hid_t integerParamsId = H5Dopen(fileId, "Integer Plot Metadata",H5P_DEFAULT);
    if (integerParamsId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "ReadSimulationParameters Marker 2" << endl;
    hid_t integerParamsSpaceId = H5Dget_space(integerParamsId);

    hsize_t integerParams_dims[1];
    hsize_t integerParams_ndims = H5Sget_simple_extent_dims(integerParamsSpaceId,
                                  integerParams_dims,NULL);
    debug5 << "ReadSimulationParameters Marker 3" << endl;
    int numParams = 16;
    if (integerParams_ndims != 1 || integerParams_dims[0] != numParams)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t integerParams_raw_data_type = H5Dget_type(integerParamsId);
    hid_t integerParams_data_type = H5Tget_native_type(integerParams_raw_data_type,
                                    H5T_DIR_ASCEND);
    debug5 << "ReadSimulationParameters Marker 4" << endl;
    int *integerParams_array = new int[numParams];
    H5Dread(integerParamsId, integerParams_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            integerParams_array);
    debug5 << "ReadSimulationParameters Marker 5" << endl;

    fileFormatVersion = integerParams_array[0];
    debug5 << "Integer Meta FileFormatVersion" << endl;
    debug5 << fileFormatVersion << endl;

    simParams.timestep = integerParams_array[1];
    debug5 << "Integer Meta" << endl;
    debug5 << simParams.nsteps << endl;


    dimension= integerParams_array[2];
    debug5 << "Integer Meta dimension" << endl;
    debug5 <<  dimension << endl;

    amrDimension = integerParams_array[3];
    debug5 << "Integer Meta numBlocks" << endl;
    debug5 <<  amrDimension << endl;

    numBlocks = integerParams_array[4];
    debug5 << "Integer Meta numBlocks" << endl;
    debug5 <<  numBlocks << endl;

    numProcessors = integerParams_array[5];
    debug5 << "Integer Meta numBlocks" << endl;
    debug5 <<  numProcessors << endl;

    numLevels = integerParams_array[6]+1;
    debug5 << "Integer Meta numLevels" << endl;
    debug5 <<  numLevels << endl;

    simParams.nxb = integerParams_array[7];
    debug5 << "Integer Meta simParams.nxb" << endl;
    debug5 <<  simParams.nxb << endl;

    simParams.nyb = integerParams_array[8];
    debug5 << "Integer Meta simParams.nyb" << endl;
    debug5 <<  simParams.nyb << endl;

    simParams.nzb = integerParams_array[9];
    debug5 << "Integer Meta simParams.nzb" << endl;
    debug5 <<  simParams.nzb << endl;


    simParams.typeGeometry = integerParams_array[10];
    debug5 << "Integer Meta simParams.nzb" << endl;
    debug5 <<  simParams.nzb << endl;


    periodicBoundary[0] = integerParams_array[11];
    debug5 << "Integer Meta simParams.nzb" << endl;
    debug5 <<  simParams.nzb << endl;


    periodicBoundary[1] = integerParams_array[12];
    debug5 << "Integer Meta simParams.nzb" << endl;
    debug5 <<  simParams.nzb << endl;


    periodicBoundary[2] = integerParams_array[13];
    debug5 << "Integer Meta simParams.nzb" << endl;
    debug5 <<  simParams.nzb << endl;

    // used to turn off things that don't make sense to do with domain cuts
    isCutFile = integerParams_array[14];
    debug5 << "Integer Meta simParams.nzb" << endl;
    debug5 <<  isCutFile << endl;

    nPlotVars = integerParams_array[15];

    debug5 << "ReadSimulationParameters Marker 6" << endl;
    // Done with the type
    H5Tclose(integerParams_data_type);
    H5Tclose(integerParams_raw_data_type);
    debug5 << "ReadSimulationParameters Marker 7" << endl;
    // Done with the space
    H5Sclose(integerParamsSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(integerParamsId);

    numBlockCells = simParams.nxb*simParams.nyb*simParams.nzb;

    debug5 << "ReadSimulationParameters Marker 8" << endl;
    // Delete the raw array
    delete[] integerParams_array;
    debug5 << "ReadSimulationParameters Marker 9" << endl;
    // Sanity check: size of the gid array better match number of blocks
    //               reported in the simulation parameters
    simParams.total_blocks = numBlocks;

    if (simParams.nxb == 1)
    {
        block_ndims[0] = 1;
        block_zdims[0] = 1;
    }
    else
    {
        block_ndims[0] = simParams.nxb+1;
        block_zdims[0] = simParams.nxb;
    }

    if (simParams.nyb == 1)
    {
        block_ndims[1] = 1;
        block_zdims[1] = 1;
    }
    else
    {
        block_ndims[1] = simParams.nyb+1;
        block_zdims[1] = simParams.nyb;
    }

    if (simParams.nzb == 1)
    {
        block_ndims[2] = 1;
        block_zdims[2] = 1;
    }
    else
    {
        block_ndims[2] = simParams.nzb+1;
        block_zdims[2] = simParams.nzb;
    }


    debug5 << "ReadSimulationParameters Marker 10" << endl;

    hid_t realParamsId = H5Dopen(fileId, "Real Plot Metadata",H5P_DEFAULT);
    if (realParamsId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "ReadSimulationParameters Marker 11" << endl;
    hid_t realParamsSpaceId = H5Dget_space(realParamsId);

    hsize_t realParams_dims[1];
    hsize_t realParams_ndims = H5Sget_simple_extent_dims(realParamsSpaceId,
                               realParams_dims,NULL);
    debug5 << "ReadSimulationParameters Marker 12" << endl;
    numParams = 7;
    if (realParams_ndims != 1 || realParams_dims[0] != numParams)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t realParams_raw_data_type = H5Dget_type(realParamsId);
    hid_t realParams_data_type = H5Tget_native_type(realParams_raw_data_type,
                                 H5T_DIR_ASCEND);
    debug5 << "ReadSimulationParameters Marker 13" << endl;
    double *realParams_array = new double[numParams];
    H5Dread(realParamsId, realParams_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            realParams_array);
    debug5 << "ReadSimulationParameters Marker 14" << endl;

    simParams.time = realParams_array[0];
    debug5 << "Real Meta simParams.time" << endl;
    debug5 << simParams.time << endl;


    minSpatialExtents[0]  = realParams_array[1];
    debug5 << "Real Meta minSpatialExtents[0]" << endl;
    debug5 << minSpatialExtents[0] << endl;

    maxSpatialExtents[0]  = realParams_array[2];
    debug5 << "Real Meta maxSpatialExtents[0]" << endl;
    debug5 << maxSpatialExtents[0] << endl;

    minSpatialExtents[1]  = realParams_array[3];
    debug5 << "Real Meta minSpatialExtents[1]" << endl;
    debug5 << minSpatialExtents[1] << endl;

    maxSpatialExtents[1]  = realParams_array[4];
    debug5 << "Real Meta maxSpatialExtents[1]" << endl;
    debug5 << maxSpatialExtents[1] << endl;

    minSpatialExtents[2]  = realParams_array[5];
    debug5 << "Real Meta minSpatialExtents[2]" << endl;
    debug5 << minSpatialExtents[2] << endl;

    maxSpatialExtents[2]  = realParams_array[6];
    debug5 << "Real Meta maxSpatialExtents[2]" << endl;
    debug5 << maxSpatialExtents[2] << endl;

    debug5 << "ReadSimulationParameters Marker 15" << endl;
    // Done with the type
    H5Tclose(realParams_data_type);
    H5Tclose(realParams_raw_data_type);
    debug5 << "ReadSimulationParameters Marker 16" << endl;
    // Done with the space
    H5Sclose(realParamsSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(realParamsId);
    debug5 << "ReadSimulationParameters Marker 17" << endl;
    // Delete the raw array
    delete[] realParams_array;
    debug5 << "ReadSimulationParameters Marker 18" << endl;


}
// ****************************************************************************
//  Method:  avtBATLFileFormat::ReadUnknownUnits
//
//  Purpose:
//    Read the variable name list.  They are called "units" in
//    BATL terminology.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Brad Whitlock, Wed Nov 18 11:26:09 PST 2009
//    Delete some arrays.
//
//   Paul D. Stewart, May 9, 2012
//   VarNames changed to units and FLASH changed to BATL
//
// ****************************************************************************

void
avtBATLFileFormat::ReadUnknownUnits()
{
    debug5 << "ReadUnknownUnits Marker 1" << endl;
    //
    // Read the variable ("untnown") names
    //
    hid_t unitsId = H5Dopen(fileId, "plotVarUnits",H5P_DEFAULT);
    if (unitsId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t untSpaceId = H5Dget_space(unitsId);

    hsize_t unt_dims[1];
    hsize_t unt_ndims =  H5Sget_simple_extent_dims(untSpaceId, unt_dims, NULL);
    if (unt_ndims != 1)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t unt_raw_data_type = H5Dget_type(unitsId);
    int length = H5Tget_size(unt_raw_data_type);

    int nvars = unt_dims[0];
    char *unt_array = new char[nvars * length];

    H5Dread(unitsId, unt_raw_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, unt_array);

    varUnits.resize(nvars);
    char *tmpstring = new char[length+1];
    for (int v=0; v<nvars; v++)
    {
        for (int c=0; c<length; c++)
        {
            tmpstring[c] = unt_array[v*length + c];
        }
        tmpstring[length] = '\0';

        varUnits[v] = tmpstring;
    }

    delete [] unt_array;
    delete [] tmpstring;

    // Done with the type
    H5Tclose(unt_raw_data_type);

    // Done with the space
    H5Sclose(untSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(unitsId);
}


// ****************************************************************************
//  Method:  avtBATLFileFormat::ReadUnknownNames
//
//  Purpose:
//    Read the variable name list.  They are called "unknowns" in
//    BATL terminology.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Brad Whitlock, Wed Nov 18 11:26:09 PST 2009
//    Delete some arrays.
//
//  Paul D. Stewart. May, 9 2012
//  FLASH to BATL and this method now reads the minimum and maximum variable
//  attributes.
// ****************************************************************************

void
avtBATLFileFormat::ReadUnknownNames()
{
    debug5 << "ReadUnknownNames Marker 1" << endl;
    //
    // Read the variable ("unknown") names
    //
    hid_t unknownsId = H5Dopen(fileId, "plotVarNames",H5P_DEFAULT);
    if (unknownsId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t unkSpaceId = H5Dget_space(unknownsId);

    hsize_t unk_dims[1];
    hsize_t unk_ndims =  H5Sget_simple_extent_dims(unkSpaceId, unk_dims, NULL);
    if (unk_ndims != 1)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t unk_raw_data_type = H5Dget_type(unknownsId);
    int length = H5Tget_size(unk_raw_data_type);

    int nvars = unk_dims[0];
    char *unk_array = new char[nvars * length];

    H5Dread(unknownsId, unk_raw_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, unk_array);

    varNames.resize(nvars);
    char *tmpstring = new char[length+1];
    for (int v=0; v<nvars; v++)
    {
        for (int c=0; c<length; c++)
        {
            tmpstring[c] = unk_array[v*length + c];
        }
        tmpstring[length] = '\0';

        varNames[v] = tmpstring;

    }

    delete [] unk_array;
    delete [] tmpstring;

    minVals.resize(nvars);
    maxVals.resize(nvars);
    for (int v=0; v<nvars; v++)
    {
        string varNamesStr = varNames[v];

        hid_t dataSet = H5Dopen(fileId, varNamesStr.c_str(), H5P_DEFAULT);
        hid_t attribute = H5Aopen_name(dataSet, "minimum");
        H5Aread(attribute, H5T_NATIVE_DOUBLE, &minVals[v]);
        H5Dclose(dataSet);
        H5Aclose(attribute);


        dataSet = H5Dopen(fileId, varNamesStr.c_str(), H5P_DEFAULT);
        attribute  = H5Aopen_name(dataSet, "maximum");
        H5Aread(attribute, H5T_NATIVE_DOUBLE, &maxVals[v]);
        H5Dclose(dataSet);
        H5Aclose(attribute);
    }

    // Done with the type
    H5Tclose(unk_raw_data_type);

    // Done with the space
    H5Sclose(unkSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(unknownsId);
}

// ****************************************************************************
//  Method:  avtBATLFileFormat::ReadAxisLabels
//
//  Purpose:
//    Read the variable name list.  They are called "unknowns" in
//    BATL terminology.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Brad Whitlock, Wed Nov 18 11:26:09 PST 2009
//    Delete some arrays.
//
//   Paul D Stewart, May 9, 2012
//   Just like ReadUnknownNames, except, this one labels you axis.
// ****************************************************************************

void
avtBATLFileFormat::ReadAxisLabels()
{
    debug5 << "ReadUnknownNames Marker 1" << endl;
    //
    // Read the variable ("lblnown") names
    //
    hid_t lblnownsId = H5Dopen(fileId, "Axis Labels",H5P_DEFAULT);
    if (lblnownsId < 0)
    {
//        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t lblSpaceId = H5Dget_space(lblnownsId);

    hsize_t lbl_dims[1];
    hsize_t lbl_ndims =  H5Sget_simple_extent_dims(lblSpaceId, lbl_dims, NULL);
    if (lbl_ndims != 1)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t lbl_raw_data_type = H5Dget_type(lblnownsId);
    int length = H5Tget_size(lbl_raw_data_type);

    int nlabels = lbl_dims[0];
    char *lbl_array = new char[nlabels * length];

    H5Dread(lblnownsId, lbl_raw_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, lbl_array);

    axisLabels.resize(nlabels);
    char *tmpstring = new char[length+1];
    for (int v=0; v<nlabels; v++)
    {
        for (int c=0; c<length; c++)
        {
            tmpstring[c] = lbl_array[v*length + c];
        }
        tmpstring[length] = '\0';

        axisLabels[v] = tmpstring;
    }

    delete [] lbl_array;
    delete [] tmpstring;

    // Done with the type
    H5Tclose(lbl_raw_data_type);

    // Done with the space
    H5Sclose(lblSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(lblnownsId);
}

// ****************************************************************************
//  Method:  avtBATLFileFormat::DetermineGlobalLogicalExtentsForAllBlocks
//
//  Purpose:
//    Get the global logical extents for this block at this refinement level.
//    This is needed for domain nesting and relies on all the block
//    extents and simulation parameters having been read.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 27 14:30:59 PDT 2005
//    The old logic was failing when the origin of the data was not at
//    (0,0,0), and also when there was more than one top-level block.
//    Rewrote this routine to fix both these problems.
//
//    Paul D Stewart, May 9, 2012
//    Took out extents computation and replaced it with a read and an addition.
//
// ****************************************************************************
void avtBATLFileFormat::DetermineGlobalLogicalExtentsForAllBlocks()
{

    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 1" << endl;
    //
    // Read the minLogicalExtents description for the blocks
    //
    hid_t minLogicalExtentsId = H5Dopen(fileId, "MinLogicalExtents",H5P_DEFAULT);
    if (minLogicalExtentsId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 2" << endl;
    hid_t minLogicalExtentsSpaceId = H5Dget_space(minLogicalExtentsId);

    hsize_t minLogicalExtents_dims[2];
    hsize_t minLogicalExtents_ndims = H5Sget_simple_extent_dims(minLogicalExtentsSpaceId, minLogicalExtents_dims, NULL);
    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 3" << endl;
    if (minLogicalExtents_ndims != 2 ||
        minLogicalExtents_dims[0] != numBlocks ||
        minLogicalExtents_dims[1] != dimension)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 4" << endl;
    int *minLogicalExtents_array = new int[numBlocks * dimension];
    H5Dread(minLogicalExtentsId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, minLogicalExtents_array);
    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 5" << endl;
    for (int b=0; b<numBlocks; b++)
    {
        int *minGlobalLogicalExtents = &minLogicalExtents_array[dimension*b];
        for (int d=0; d<3; d++)
        {
            if (d < dimension)
            {
                blocks[b].minGlobalLogicalExtents[d]=minGlobalLogicalExtents[d];
                blocks[b].maxGlobalLogicalExtents[d]=minGlobalLogicalExtents[d] + block_zdims[d];
            }
            else
            {
                blocks[b].minGlobalLogicalExtents[d] = 0;
                blocks[b].maxGlobalLogicalExtents[d] = 0;

            }
        }

        debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 1" << endl;
        debug5 << b << endl;
    }
    // Delete the raw array
    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 6" << endl;
    delete[] minLogicalExtents_array;
    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 7" << endl;
    // Done with the space
    H5Sclose(minLogicalExtentsSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(minLogicalExtentsId);
    debug5 << "DetermineGlobalLogicalExtentsForAllBlocks Marker 8" << endl;

}

// ****************************************************************************
//  Method:  avtBATLFileFormat::BuildDomainNesting
//
//  Purpose:
//     Generates the domain nesting object needed to remove coarse zones in
//     an AMR mesh.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 27 14:31:56 PDT 2005
//    Made an indexing origin error for the domain boundaries.
//
//    Randy Hudson, Jul 29, 2008
//    Changed the hard-coded mesh name in two cache calls from "mesh" to
//    "mesh_blockandlevel" to match the new mesh name assigned in
//    "PopulateDatabaseMetaData()", then duplicated the contents of
//    "BuildDomainNesting()" to process mesh "mesh_blockandproc", also assigned
//    in "PopulateDatabaseMetaData()".
//
//    Hank Childs, Tue Dec 15 14:30:07 PST 2009
//    Reorder indices to accommodate reordering of patches for SIL efficiency.
//
//    Hank Childs, Thu Dec 17 14:07:52 PST 2009
//    Added database options (for toggling between processors or levels).
//
//    Hank Childs, Sat Mar 20 20:25:14 PDT 2010
//    Only do domain abutment based on database option.
//
//    Paul D Stewart, May 9, 2012,
//    You no longer have a choice about seting up domain abutment information.
//    Also added support for non-Cartesian grids.
// ****************************************************************************

void
avtBATLFileFormat::BuildDomainNesting()
{
    debug5 << "BuildDomainNesting Marker 1" << endl;
    if (numBlocks <= 1 || avtDatabase::OnlyServeUpMetaData())
        return;
    debug5 << "BuildDomainNesting Marker 2" << endl;

    //  ***********************************************************************
    //  PROCESS THE "amr_mesh" MESH
    //  ***********************************************************************

    // first, look to see if we don't already have it cached
    void_ref_ptr vrTmp = cache->GetVoidRef("amr_mesh",
                                           AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                           timestep, -1);
    debug5 << "BuildDomainNesting Marker 3" << endl;

    if ((*vrTmp == NULL))
    {
        int i;
        debug5 << "BuildDomainNesting Marker 4" << endl;

        int t1 = visitTimer->StartTimer();
        avtStructuredDomainBoundaries *rdb = 0;

        if (simParams.typeGeometry > 2)
            rdb = new avtCurvilinearDomainBoundaries(true);
        else
            rdb = new avtRectilinearDomainBoundaries(true);

        debug5 << "BuildDomainNesting Marker 5" << endl;

        rdb->SetNumDomains(numBlocks);
        for (i = 0; i < numBlocks; i++)
        {
            int logExts[6];
            logExts[0] = blocks[i].minGlobalLogicalExtents[0];
            logExts[1] = blocks[i].maxGlobalLogicalExtents[0];
            logExts[2] = blocks[i].minGlobalLogicalExtents[1];
            logExts[3] = blocks[i].maxGlobalLogicalExtents[1];
            logExts[4] = blocks[i].minGlobalLogicalExtents[2];
            logExts[5] = blocks[i].maxGlobalLogicalExtents[2];
            rdb->SetIndicesForAMRPatch(i, blocks[i].level - 1, logExts);

        }
        debug5 << "BuildDomainNesting Marker 6" << endl;

        rdb->CalculateBoundaries();
        debug5 << "BuildDomainNesting Marker 7" << endl;

        void_ref_ptr vrdb = void_ref_ptr(rdb,
                                         avtStructuredDomainBoundaries::Destruct);
        debug5 << "BuildDomainNesting Marker 8" << endl;

        cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                            timestep, -1, vrdb);
        debug5 << "BuildDomainNesting Marker 9" << endl;

        visitTimer->StopTimer(t1, "BATL setting up domain boundaries");
        debug5 << "BuildDomainNesting Marker 10" << endl;


        //
        // build the avtDomainNesting object
        //

        if (numLevels > 0)
        {
            debug5 << "BuildDomainNesting Marker 1" << endl;

            int t2 = visitTimer->StartTimer();
            avtStructuredDomainNesting *dn =
                new avtStructuredDomainNesting(numBlocks, numLevels);

            dn->SetNumDimensions(dimension);
            debug5 << "BuildDomainNesting Marker 1" << endl;

            //
            // Set refinement level ratio information
            //

            // NOTE: BATL files always have a 2:1 ratio
            vector<int> ratios(3);
            ratios[0] = 1;
            ratios[1] = 1;
            ratios[2] = 1;
            dn->SetLevelRefinementRatios(0, ratios);
            for (i = 1; i < numLevels; i++)
            {
                vector<int> ratios(3);
                ratios[0] = 2;
                ratios[1] = 2;
                ratios[2] = 2;
                dn->SetLevelRefinementRatios(i, ratios);
            }
            debug5 << "BuildDomainNesting Marker 1" << endl;

            //
            // set each domain's level and logical extents
            //
            for (i = 0; i < numBlocks; i++)
            {
                vector<int> childBlocks;

                vector<int> logExts(6);

                logExts[0] = blocks[i].minGlobalLogicalExtents[0];
                logExts[1] = blocks[i].minGlobalLogicalExtents[1];
                logExts[2] = blocks[i].minGlobalLogicalExtents[2];

                logExts[3] = blocks[i].maxGlobalLogicalExtents[0];
                if (dimension >= 2)
                    logExts[4] = blocks[i].maxGlobalLogicalExtents[1];
                else
                    logExts[4] = blocks[i].maxGlobalLogicalExtents[1];
                if (dimension >= 3)
                    logExts[5] = blocks[i].maxGlobalLogicalExtents[2];
                else
                    logExts[5] = blocks[i].maxGlobalLogicalExtents[2];

                dn->SetNestingForDomain(i, blocks[i].level,
                                        childBlocks, logExts);
            }
            debug5 << "BuildDomainNesting Marker 1" << endl;

            void_ref_ptr vr = void_ref_ptr(dn,
                                           avtStructuredDomainNesting::Destruct);

            cache->CacheVoidRef("amr_mesh",
                                AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                timestep, -1, vr);
            visitTimer->StopTimer(t2, "BATL setting up patch nesting");
        }
        debug5 << "BuildDomainNesting Marker 1" << endl;

    }

    //  ***********************************************************************
    //  PROCESS THE "mesh_blockandproc" MESH
    //  ***********************************************************************

    if (showProcessors)
    {
        // first, look to see if we don't already have it cached
        void_ref_ptr vrTmp2 = cache->GetVoidRef("mesh_blockandproc",
                                                AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                                timestep, -1);

        if ((*vrTmp2 == NULL))
        {
            int i;

            avtStructuredDomainBoundaries *rdb = 0;

            if (simParams.typeGeometry > 2)
                rdb = new avtCurvilinearDomainBoundaries(true);
            else
                rdb = new avtRectilinearDomainBoundaries(true);

            rdb->SetNumDomains(numBlocks);
            for (i = 0; i < numBlocks; i++)
            {
                int logExts[6];
                logExts[0] = blocks[i].minGlobalLogicalExtents[0];
                logExts[1] = blocks[i].maxGlobalLogicalExtents[0];
                logExts[2] = blocks[i].minGlobalLogicalExtents[1];
                logExts[3] = blocks[i].maxGlobalLogicalExtents[1];
                logExts[4] = blocks[i].minGlobalLogicalExtents[2];
                logExts[5] = blocks[i].maxGlobalLogicalExtents[2];
                rdb->SetIndicesForAMRPatch(i, blocks[i].level, logExts);

            }
            rdb->CalculateBoundaries();

            void_ref_ptr vrdb = void_ref_ptr(rdb,
                                             avtStructuredDomainBoundaries::Destruct);
            cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                timestep, -1, vrdb);

            //
            // build the avtDomainNesting object
            //

            if (numLevels > 0)
            {
                avtStructuredDomainNesting *dn =
                    new avtStructuredDomainNesting(numBlocks, numLevels);

                dn->SetNumDimensions(dimension);

                //
                // Set refinement level ratio information
                //

                // NOTE: BATL files always have a 2:1 ratio
                vector<int> ratios(3);
                ratios[0] = 1;
                ratios[1] = 1;
                ratios[2] = 1;
                dn->SetLevelRefinementRatios(0, ratios);
                for (i = 1; i < numLevels; i++)
                {
                    vector<int> ratios(3);
                    ratios[0] = 2;
                    ratios[1] = 2;
                    ratios[2] = 2;
                    dn->SetLevelRefinementRatios(i, ratios);
                }

                //
                // set each domain's level, children and logical extents
                //
                for (i = 0; i < numBlocks; i++)
                {
                    vector<int> childBlocks;

                    vector<int> logExts(6);

                    logExts[0] = blocks[i].minGlobalLogicalExtents[0];
                    logExts[1] = blocks[i].minGlobalLogicalExtents[1];
                    logExts[2] = blocks[i].minGlobalLogicalExtents[2];

                    logExts[3] = blocks[i].maxGlobalLogicalExtents[0];
                    if (dimension >= 2)
                        logExts[4] = blocks[i].maxGlobalLogicalExtents[1];
                    else
                        logExts[4] = blocks[i].maxGlobalLogicalExtents[1];
                    if (dimension >= 3)
                        logExts[5] = blocks[i].maxGlobalLogicalExtents[2];
                    else
                        logExts[5] = blocks[i].maxGlobalLogicalExtents[2];
                    dn->SetNestingForDomain(i, blocks[i].level,
                                            childBlocks, logExts);

//                     dn->SetNestingForDomain(BATLIdToVisitId[i], blocks[i].level,
//                                             childBlocks, logExts);
                }

                void_ref_ptr vr = void_ref_ptr(dn,
                                               avtStructuredDomainNesting::Destruct);

                cache->CacheVoidRef("mesh_blockandproc",
                                    AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                    timestep, -1, vr);
            }
        }
    }
}


// ****************************************************************************
//  Method:  avtBATLFileFormat::GetAuxiliaryData
//
//  Purpose:
//    Right now this only supports spatial interval trees.  There is no
//    other information like materials and species available.
//
//  Arguments:
//    type       the kind of auxiliary data to create
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 27 14:32:26 PDT 2005
//    Don't attempt if we don't have any grid data in this file (i.e. only
//    particles in the file).
//
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
//    Hank Childs, Tue Dec 15 14:30:07 PST 2009
//    Reorder indices to accommodate reordering of patches for SIL efficiency.
//
//    Paul D. Stewart, May 9, 2012,
//    Added DATA_DATA_EXTENTS
// ****************************************************************************
void *
avtBATLFileFormat::GetAuxiliaryData(const char *var, int dom,
                                    const char * type, void *,
                                    DestructorFunction &df)
{
    debug5 << "GetAuxiliaryData Marker 1" << endl;
    debug5 << type << endl;
    void *retval = 0;
    if (numBlocks == 0)
        return NULL;
    if (strcmp(type, "AUXILIARY_DATA_SPATIAL_EXTENTS") == 0)
    {
        debug5 << "DATA_SPATIAL_EXTENTS" << endl;
        avtIntervalTree *itree = new avtIntervalTree(numBlocks, 3);

        for (int b = 0 ; b < numBlocks ; b++)
        {
            double bounds[6];
            bounds[0] = blocks[b].minSpatialExtents[0];
            bounds[1] = blocks[b].maxSpatialExtents[0];
            bounds[2] = blocks[b].minSpatialExtents[1];
            bounds[3] = blocks[b].maxSpatialExtents[1];
            bounds[4] = blocks[b].minSpatialExtents[2];
            bounds[5] = blocks[b].maxSpatialExtents[2];
            itree->AddElement(b, bounds);
        }
        itree->Calculate(true);

        df = avtIntervalTree::Destruct;

        retval = (void *)itree;
    }
    else if (strcmp(type, "AUXILIARY_DATA_DATA_EXTENTS") == 0 )
    {
        debug5 << "DATA_DATA called" << endl;
        // Read the number of domains for the mesh.
        string varstr = var;
        varstr.append("_Ext");

        hid_t extrId = H5Dopen(fileId, varstr.c_str(),H5P_DEFAULT);
        if (extrId < 0)
        {
            return 0;
            debug5 << "file has no DATA_DATA extents" << endl;
        }

        hid_t extrSpaceId = H5Dget_space(extrId);

        hsize_t extr_dims[2];
        hsize_t extr_ndims = H5Sget_simple_extent_dims(extrSpaceId, extr_dims, NULL);
        if (extr_ndims != 2 ||
            extr_dims[0] != numBlocks ||
            extr_dims[1] != 2)
        {
            EXCEPTION1(InvalidFilesException, varstr.c_str());
        }

        double *extr_array = new double[numBlocks * 2];
        H5Dread(extrId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, extr_array);
        // Delete the raw array


        //Done with the space
        H5Sclose(extrSpaceId);

        // Done with the variable; don't leak it
        H5Dclose(extrId);

        // Create an interval tree
        avtIntervalTree *itree = new avtIntervalTree(numBlocks, 1);

        for (int b=0; b<numBlocks; b++)
        {
            double *extr_line = &extr_array[2*b];
            double range[2];
            range[0] = extr_line[0];
            range[1] = extr_line[1];
            debug5 << range[0] << endl;
            debug5 << range[1] << endl;
            if ((range[0] < 0) or (range[1] < 0))
            {
                debug5 << "NEGATIVE DATA DATA" << endl;
            }
            itree->AddElement(b, range);

        }
        itree->Calculate(true);
        // Delete temporary arrays.
        delete[] extr_array;
        // Set return values
        retval = (void *)itree;
        df = avtIntervalTree::Destruct;
    }
    return retval;
}

