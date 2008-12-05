/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                            avtITAPS_CWriter.C                             //
// ************************************************************************* //

#include <avtITAPS_CWriter.h>
#include <avtITAPS_CUtility.h>

#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

#include <iMesh.h>

#include <vtkCell.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkType.h>

#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;
using namespace avtITAPS_CUtility;

// ****************************************************************************
//  Method: avtITAPS_CWriter constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

avtITAPS_CWriter::avtITAPS_CWriter(DBOptionsAttributes *dbopts)
{
    formatType = "MOAB";
    formatExtension = "h5m";

    for (int i = 0; dbopts != 0 && i < dbopts->GetNumberOfOptions(); ++i)
    {
        if (dbopts->GetName(i) == "Format")
        {
            switch (dbopts->GetEnum("Format"))
            {
                case 0: break; // default case 
                case 1: formatType = "EXODUS"; formatExtension = "exoII"; break;
                case 2: formatType = "VTK";    formatExtension = "vtk"; break;
                case 3: formatType = "SLAC";   formatExtension = "slac"; break;
                case 4: formatType = "GMV";    formatExtension = "gmv"; break;
                case 5: formatType = "ANSYS";  formatExtension = "ans"; break;
                case 6: formatType = "GMSH";   formatExtension = "gmsh"; break;
                case 7: formatType = "STL";    formatExtension = "stl"; break;
            }
        }
    }
}

// ****************************************************************************
//  Method: avtITAPS_CWriter destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

avtITAPS_CWriter::~avtITAPS_CWriter()
{
}


// ****************************************************************************
//  Method: avtITAPS_CWriter::OpenFile
//
//  Purpose:
//
//  Programmer: November 20, 2008 
//  Creation:   September 11, 2003
//
// ****************************************************************************

void
avtITAPS_CWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
    dir ="";
    // find dir if provided
    int idx = stem.rfind("/");
    if ( idx != string::npos )
    {
        int stem_len = stem.size() - (idx+1) ;
        dir  = stem.substr(0,idx+1);
        stem = stem.substr(idx+1,stem_len);
    }
}

// ****************************************************************************
//  Method: avtITAPS_CWriter::WriteHeaders
//
//  Purpose:
//      This will write out the multi-vars for the ITAPS_C constructs.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::WriteHeaders(const avtDatabaseMetaData *md,
                            vector<string> &scalars, vector<string> &vectors,
                            vector<string> &materials)
{
    const avtMeshMetaData *mmd = md->GetMesh(0);

}

// ****************************************************************************
//  Method: avtITAPS_CWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    int i;

    char dummyStr[32];
    iMesh_Instance itapsMesh;
    iMesh_newMesh(dummyStr, &itapsMesh, &itapsError, 0);
    CheckITAPSError(itapsMesh, iMesh_newMesh, NoL);

    iBase_EntitySetHandle rootSet;
    iMesh_getRootSet(itapsMesh, &rootSet, &itapsError);
    CheckITAPSError(itapsMesh, iMesh_getRootSet, NoL);

    try {

        // Create the entity set representing this chunk 
        iBase_EntitySetHandle chunkSet;
        iMesh_createEntSet(itapsMesh, 0, &chunkSet, &itapsError);
        CheckITAPSError(itapsMesh, iMesh_createEntSet, NoL);
    
        // Add the nodes of this mesh as vertices of the iMesh instance.
        // Note that vertices can only ever be created or live in the instance
        // itself and not in entity sets of the instance. Though, I think we
        // can create 'links' to vertices from a given entity set using 'add'
        // method(s). We should really use the 'Arr' versions of this method
        // but initial coding is simplest to use individual entity methods.
        //
        // In fact, it would appear as though everything is, by fiat, created
        // in the iMesh instance itself (root set), and then can be 'moved' by
        // adding to another set and removing from root?
        int npts = ds->GetNumberOfPoints(); 
        iBase_EntityHandle *ptHdls = new iBase_EntityHandle[npts];
        for (i = 0; i < npts; i++)
        {
            double pt[3];
            ds->GetPoint(i, pt);
    
            // create initial Vtx entity
            iMesh_createVtx(itapsMesh, pt[0], pt[1], pt[2], &ptHdls[i], &itapsError);
            if (i<5) CheckITAPSError(itapsMesh, iMesh_createVtx, NoL);
    
            // add Vtx entity to chunkSet
            //iMesh_addEntToSet(itapsMesh, ptHdls[i], &chunkSet, &itapsError);
            //if (i<5) CheckITAPSError(itapsMesh, iMesh_addEntToSet, NoL);
    
            // remove Vtx entity from root set
            //iMesh_rmvEntFromSet(itapsMesh, ptHdls[i], &rootSet, &itapsError); 
            //if (i<5) CheckITAPSError(itapsMesh, iMesh_rmvEntFromSet, NoL);
        }
    
        // add just created Vtx entites to chunkSet
        iMesh_addEntArrToSet(itapsMesh, ptHdls, npts, &chunkSet, &itapsError);
        CheckITAPSError(itapsMesh, iMesh_addEntArrToSet, NoL);
    
        // remove just created Vtx entities from rootSet, ok?
        //iMesh_rmvEntArrFromSet(itapsMesh, ptHdls, npts, &rootSet, &itapsError);
        //CheckITAPSError(itapsMesh, iMesh_rmvEntArrFromSet, NoL);
    
        int ncells = ds->GetNumberOfCells();
        iBase_EntityHandle *znHdls = new iBase_EntityHandle[ncells];
        for (i = 0; i < ncells; i++)
        {
            vtkCell *theCell = ds->GetCell(i);
    
            int status;
            int topo = VTKZoneTypeToITAPSEntityTopology(theCell->GetCellType());
            int ncellPts = theCell->GetNumberOfPoints();
            iBase_EntityHandle *cellPtEnts = new iBase_EntityHandle[ncellPts];
            for (int j = 0; j < ncellPts; j++)
                cellPtEnts[j] = ptHdls[theCell->GetPointId(j)];
    
            iMesh_createEnt(itapsMesh, topo, cellPtEnts, ncellPts, &znHdls[i], &status, &itapsError);
            if (i<5) CheckITAPSError(itapsMesh, iMesh_createEnt, NoL);
        }
    
        // add just created cell entites to chunkSet
        iMesh_addEntArrToSet(itapsMesh, znHdls, ncells, &chunkSet, &itapsError);
        CheckITAPSError(itapsMesh, iMesh_addEntArrToSet, NoL);
    
        // remove just created cell entities from rootSet, ok?
        //iMesh_rmvEntArrFromSet(itapsMesh, znHdls, ncells, &rootSet, &itapsError);
        //CheckITAPSError(itapsMesh, iMesh_rmvEntArrFromSet, NoL);
    
        // save the file
        string fname = dir + stem;
        char filename[1024];
        sprintf(filename, "%s.%d.%s", fname.c_str(), chunk, formatExtension.c_str());
        iMesh_save(itapsMesh, rootSet, filename, formatType.c_str(), &itapsError,
            strlen(filename), formatType.size());
        CheckITAPSError(itapsMesh, iMesh_save, NoL);
    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#ifdef ITAPS_MOAB
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#elif ITAPS_GRUMMP
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc);
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }
funcEnd: ;
}


// ****************************************************************************
//  Method: avtITAPS_CWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::CloseFile(void)
{
}
