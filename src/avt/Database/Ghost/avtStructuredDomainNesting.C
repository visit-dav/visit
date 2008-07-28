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
//                       avtStructuredDomainNesting.C                        //
// ************************************************************************* //

#include <avtStructuredDomainNesting.h>

#include <avtGhostData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <UnexpectedValueException.h>
#include <VisItException.h>

#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#define MAX_GHOST_LAYERS 2

// ****************************************************************************
//  Destructor:  avtStructuredDomainNesting::Destruct
//
//  Programmer:  Mark C. Miller 
//  Creation:    October 13, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 14:45:46 PDT 2002
//    Cleaned up stupid mistake where void pointer was being deleted.
//
// ****************************************************************************
void
avtStructuredDomainNesting::Destruct(void *p)
{
    avtStructuredDomainNesting *sdn = (avtStructuredDomainNesting *) p;
    delete sdn;
}

// ****************************************************************************
//  Function:  DetectBoundaryGhostLayers 
//
//  Purpose: Detects number of ghost layers in each dimension given
//     knowledge of the dimension, non-ghosted size of the patch in each
//     dimension and the pre-existing avtGhostZones data
//
//     To perform the detection, basically we try all reasonable combinations
//     of ghost numbers of layers and probe the ghostData array around the
//     extreme high and low ijk corners of the patch to see if we get something
//     thats consistent. The loop looks daunting but it completes very quickly.
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 8, 2004
//
// ****************************************************************************
void
DetectBoundaryGhostLayers(int numDims, unsigned char *ghostData, int numCells,
    vector<int> extents, int *ghostLayers)
{
    int Ni = 0, Nj = 0, Nk = 0;

    // compute size of NON-ghosted patch
    switch (numDims) // note: exploits fall-through
    {
        case 3: Nk = extents[5] - extents[2] + 1;
        case 2: Nj = extents[4] - extents[1] + 1;
        case 1: Ni = extents[3] - extents[0] + 1;
    }

    // vector to populate with valid ghost cases
    vector<int> ghostCases;

    // loop over all possible combinations of ghost layer widths
    int gi, gj, gk;
    for (gi = 0; gi <= (Ni==0 ? 0 : MAX_GHOST_LAYERS); gi++)
    {
        for (gj = 0; gj <= (Nj==0 ? 0 : MAX_GHOST_LAYERS); gj++)
        {
            for (gk = 0; gk <= (Nk==0 ? 0 : MAX_GHOST_LAYERS); gk++)
            {
                // we always skip the all 0 case
                if ((gi == 0) && (gj == 0) && (gk == 0))
                    continue;

                // compute size of ghosted patch given this layering 
                int Mk = Nk + 2 * gk;
                int Mj = Nj + 2 * gj;
                int Mi = Ni + 2 * gi;

                // can ignore this case immediately if the number of
                // cells it presumes doesn't equate to the number of
                // cells we know we have 
                int ncells = 0;
                switch (numDims)
                {
                    case 3: ncells = Mk * Mj * Mi; break;
                    case 2: ncells =      Mj * Mi; break;
                    case 1: ncells =           Mi; break;
                }
                if (ncells != numCells)
                    continue;

                // probe ghost data array round 2 extreme corners
                int mult;
                bool impossibleGhostCase = false;
                for (mult = -1; (mult <= 1) && !impossibleGhostCase; mult += 2)
                {
                    int i0, j0, k0;

                    // set which corner to examine ghost values around 
                    // -1 --> Lower, Left, Front
                    // +1 --> Upper, Right, Back
                    if (mult == -1)
                    {
                        // the extreme lowest corner 
                        // in the 
                        i0 = 0 + gi;
                        j0 = 0 + gj;
                        k0 = 0 + gk;
                    }
                    else
                    {
                        // the extreme highest corner 
                        i0 = Ni - 1 + gi;
                        j0 = Nj - 1 + gj;
                        k0 = Nk - 1 + gk;
                    }

                    // examine all cells around the corner for ghost values
                    int i,j,k;
                    for (i = 0; (i <= gi) && !impossibleGhostCase; i++)
                    {
                        for (j = 0; (j <= gj) && !impossibleGhostCase; j++)
                        {
                            for (k = 0; (k <= gk) && !impossibleGhostCase; k++)
                            {
                                // we always skip the all 0 case
                                if ((i == 0) && (j == 0) && (k == 0))
                                    continue;

                                // compute index within ghosted patch
                                int a = i0 + i * mult;
                                int b = j0 + j * mult;
                                int c = k0 + k * mult;
                                int idx;

                                // compute offset into ghost array assuming
                                // a zone really exists at the logical index [a,b,c]
                                switch (numDims)
                                {
                                    case 3: idx = c*Mi*Mj + b*Mi + a; break;
                                    case 2: idx =           b*Mi + a; break;
                                    case 1: idx =                  a; break;
                                }

                                // if the computed index is either out of
                                // range or the ghost value indicates it
                                // is NOT really a ghost, its invalid
                                if ((idx < 0) || (idx >= numCells) ||
                                    (ghostData[idx] == 0))
                                {
                                    impossibleGhostCase = true;
                                }
                            }
                        }
                    }
                }

                if (!impossibleGhostCase)
                {
                    ghostCases.push_back(gi);
                    ghostCases.push_back(gj);
                    ghostCases.push_back(gk);
                }

            }
        }
    }

    if (ghostCases.size() != 3)
    {
        EXCEPTION2(UnexpectedValueException, 3, ghostCases.size());
    }

    ghostLayers[0] = ghostCases[0];
    ghostLayers[1] = ghostCases[1];
    ghostLayers[2] = ghostCases[2];

}

// ****************************************************************************
//  Method avtStructuredDomainNesting::GetSelectedDescendents
//
//  Purpose: Given the list of all domains in the current selection and a
//  a given domain, find all of the given domain's descendents that are also
//  in the current selection. In fact, it finds only the *greatest* descendents
//  in the sense that once it finds a descendent that is in the selection, it
//  does not continue to process that descendent's descendents. However, if
//  a descendent is not in the current selection, it nonetheless, checks 
//  that descendent's descendents for the possibility that thay might be.
//
//  Programmer:  Mark C. Miller 
//  Creation:    August 18, 2004
//
//  Modifications:
//
//    Hank Childs, Mon May 12 08:09:45 PDT 2008
//    Added the lookup table as an argument, since it was being recalculated 
//    for each domain and that was adding up (i.e. O(n^2)).
//
// ****************************************************************************

void
avtStructuredDomainNesting::GetSelectedDescendents(
    const vector<int>& allDomainList, int dom,
    vector<int>& selectedDescendents, const vector<bool> &lookup) const
{
    int maxDom = lookup.size()-1;
    vector<int> domQueue;
    domQueue.push_back(dom);

    while (domQueue.size())
    {
       int currentDom = domQueue.back();
       domQueue.pop_back();
       const vector<int>& childDoms = domainNesting[currentDom].childDomains;

       for (size_t i = 0; i < childDoms.size(); i++)
       {
           if ((childDoms[i] <= maxDom) && lookup[childDoms[i]])
               selectedDescendents.push_back(childDoms[i]);
           else
               domQueue.push_back(childDoms[i]);
       }
    }
}

// ****************************************************************************
//  Method: avtStructuredDomainNesting::ApplyGhost
//
//  Purpose:
//    Applies "avtGhostZones" array to the meshes passed in 
//
//  Programmer:  Mark C. Miller 
//  Creation:    October 13, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Nov 18 22:56:02 PST 2003
//    Clean up memory leak.
//
//    Mark C. Miller, Thu Jan  8 10:08:18 PST 2004
//    Modified to account for fact that incoming meshes may already have ghost
//    zones from the file AND to accomodate possible avariation in number of
//    ghost layers for different variables (SAMRAI does this). Also, I modifed
//    it so that it will or the NESTING_GHOST_MASK value into the ghost data
//    array. That way, when we go to using a bitfield for vtkGhostLevels, it can
//    be easily modified to set the right bit. I also replaced cerr statements
//    with bonified exceptions.
//
//    Mark C. Miller, Thu Jan 29 10:49:35 PST 2004
//    I added code to set ghost values whether a child is present or not. If
//    a child is NOT present, ghost values are set to 0 indicating that
//    the parent should not be ghosted. I also removed tests for index out
//    of range from inner loops. Finally, I changed to the code to use straight
//    assignment rather than bit operators to avoid possible generation of
//    non-0/1 values.
//
//    Mark C. Miller, Wed Aug 18 18:20:27 PDT 2004
//    Made it support the case where a domain's children may not be selected
//    but its grandchildren (or any deeper descendent) might be. I also
//    re-organized the key loops a bit for better cache efficiency and less
//    multiplication.
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Renamed ghost data array.  Also properly mark each zone's ghost type.
//
//    Mark C. Miller, Wed Feb 16 14:44:17 PST 2005
//    Added check to skip over patches with no mesh
//
//    Hank Childs, Tue Oct  9 07:43:43 PDT 2007
//    Add support for avtRealDims.
//
//    Hank Childs, Wed Oct 24 13:20:07 PDT 2007
//    If the input already has ghost data, then make a copy of its
//    ghost data.  Otherwise, we can run into the following bug:
//    (1) Make PC plot with all patches, (2) Make second PC plot with
//    only one patch, (3) Pick on first PC plot.  If we don't replace the
//    ghost data with a new array, then step (2) will overwrite the ghost
//    data from step (1) and the pick from (3) will get the wrong ghost data.
//
//    Hank Childs, Mon May 12 08:13:45 PDT 2008
//    Cache the lookup table to prevent an O(n^2) type algorithm.
//
// ****************************************************************************

bool
avtStructuredDomainNesting::ApplyGhost(vector<int> domainList,
                                       vector<int> allDomainList,
                                       vector<vtkDataSet*> meshes)
{
    bool didGhost = false;

    //
    // Build a lookup table for the all domain list.  This will be sent
    // into GetSelectedDescendants.  And we are doing it here to prevent
    // a lot of recalculation.
    //
    int maxDom = 0;
    for (size_t i = 0; i < allDomainList.size(); i++)
    {
        if (allDomainList[i] > maxDom)
            maxDom = allDomainList[i];
    }
    vector<bool> lookup(maxDom + 1, false);
    for (size_t i = 0; i < allDomainList.size(); i++)
        lookup[allDomainList[i]] = true;

    for (size_t i = 0; i < domainList.size(); i++)
    {
        unsigned char *ghostData = 0;

        if (meshes[i] == NULL)
            continue;

        int parentDom = domainList[i];
        int numCells = meshes[i]->GetNumberOfCells();

        // Allocate the new ghost data array.  Allocate it now in case we
        // need to copy into it.
        vtkUnsignedCharArray *ghostArray = vtkUnsignedCharArray::New();
        ghostArray->SetNumberOfTuples(numCells);
        ghostData = (unsigned char *) ghostArray->GetVoidPointer(0);
        ghostArray->SetName("avtGhostZones");

        // assume there are no boundary ghost layers
        int ghostLayers[3] = {0, 0, 0};

        // See if we had an existing ghost data array.
        vtkUnsignedCharArray *oldArray = vtkUnsignedCharArray::SafeDownCast(
            meshes[i]->GetCellData()->GetArray("avtGhostZones"));
        if (oldArray != NULL)
        {
            // Copy the info from the old ghost data array to 
            // the new ghost data array.
            unsigned char *oldGhostData = 
                               (unsigned char *) oldArray->GetVoidPointer(0);
            memcpy(ghostData, oldGhostData, numCells);

            if (meshes[i]->GetFieldData()->GetArray("avtRealDims") != NULL)
            {
                vtkIntArray *realDims = (vtkIntArray *) 
                            meshes[i]->GetFieldData()->GetArray("avtRealDims");
                ghostLayers[0] = realDims->GetValue(0);
                ghostLayers[1] = realDims->GetValue(2);
                ghostLayers[2] = realDims->GetValue(4);
            }
            else
            {
                //
                // because number of ghost layers may vary from one variable to
                // the next, we have a strange problem here in that we know we 
                // have ghost layers but don't know precisely how many in each 
                // dimension.  So, we detect it from knowledge of the non-
                // ghosted size (in the nesting info's logical extents of the 
                // parent) of the patch and the actual ghost data array
                //
                DetectBoundaryGhostLayers(numDimensions, oldGhostData, numCells,
                    domainNesting[parentDom].logicalExtents, ghostLayers);
            }

            // Get rid of the old ghost data.
            meshes[i]->GetCellData()->RemoveArray("avtGhostZones");
        }

        // Add the new ghost data array.
        meshes[i]->GetCellData()->AddArray(ghostArray);
        ghostArray->Delete();

        //
        // Compute (memory) size of this domain including ghostLayers
        //
        int Ni         = domainNesting[parentDom].logicalExtents[3] -
                         domainNesting[parentDom].logicalExtents[0] +
                         1 + 2 * ghostLayers[0];
        int Nj         = domainNesting[parentDom].logicalExtents[4] -
                         domainNesting[parentDom].logicalExtents[1] +
                         1 + 2 * ghostLayers[1];
        int Nk         = domainNesting[parentDom].logicalExtents[5] -
                         domainNesting[parentDom].logicalExtents[2] +
                         1 + 2 * ghostLayers[2];

        //
        // Clear out internal ghost values (e.g. those that are not part
        // of ghostLayers which came from the database)
        //
        for (int kk = ghostLayers[2]; kk < Nk - ghostLayers[2]; kk++)
        {
            int c = kk*Ni*Nj;
            for (int jj = ghostLayers[1]; jj < Nj - ghostLayers[1]; jj++)
            {
                int b = c + jj*Ni;
                for (int ii = ghostLayers[0]; ii < Ni - ghostLayers[0]; ii++)
                {
                    int a = b + ii;
                    ghostData[a] = 0x0;
                }
            }
        }

        //
        // Obtain the list of descendents of the current domain that are
        // also in the current selection.
        //
        vector<int> selectedDescendents;
        GetSelectedDescendents(allDomainList, parentDom, selectedDescendents, lookup);

        //
        // For each descendent, ghost the current domain appropriately
        //
        for (size_t j = 0; j < selectedDescendents.size(); j++)
        {
            int ratio[3]   = {1, 1, 1};
            int descDom    = selectedDescendents[j];
            int parentLevel= domainNesting[parentDom].level;
            int descLevel  = domainNesting[descDom].level;
            int I0         = domainNesting[parentDom].logicalExtents[0];
            int I1         = domainNesting[parentDom].logicalExtents[3] + 1;
            int J0         = domainNesting[parentDom].logicalExtents[1];
            int J1         = domainNesting[parentDom].logicalExtents[4] + 1;
            int K0         = domainNesting[parentDom].logicalExtents[2];
            int K1         = domainNesting[parentDom].logicalExtents[5] + 1;

            //
            // compute ratio to descendent from parent
            //
            for (int l = parentLevel+1; l <= descLevel; l++)
            {
                ratio[0] *= levelRatios[l][0];
                ratio[1] *= (numDimensions >= 2 ? levelRatios[l][1] : 1);
                ratio[2] *= (numDimensions >= 3 ? levelRatios[l][2] : 1);
            }

            //
            // Compute min/max extents in the current patch's level by mapping
            // the selected descendent's indexing scheme onto this level
            //
            int i0 = domainNesting[descDom].logicalExtents[0] / ratio[0];
            int i1 = (domainNesting[descDom].logicalExtents[3] + 1) / ratio[0];
            if ((domainNesting[descDom].logicalExtents[3] + 1) % ratio[0]) 
                i1++;
            int j0 = domainNesting[descDom].logicalExtents[1] / ratio[1];
            int j1 = (domainNesting[descDom].logicalExtents[4] + 1) / ratio[1];
            if ((domainNesting[descDom].logicalExtents[4] + 1) % ratio[1]) 
                j1++;
            int k0 = domainNesting[descDom].logicalExtents[2] / ratio[2];
            int k1 = (domainNesting[descDom].logicalExtents[5] + 1) / ratio[2];
            if ((domainNesting[descDom].logicalExtents[5] + 1) % ratio[2]) 
                k1++;

            //
            // Often a descendent domain spans multiple parents. So, we need to
            // clip the bounds computed above to THIS parent's upper/lower 
            // bounds
            //
            if (i1 > I1) i1 = I1;
            if (j1 > J1) j1 = J1;
            if (k1 > K1) k1 = K1;
            if (i0 < I0) i0 = I0;
            if (j0 < J0) j0 = J0;
            if (k0 < K0) k0 = K0;

            didGhost = true;

            if (numDimensions == 3)
            {
                for (int kk = k0; kk < k1; kk++)
                {
                    int cc = kk - K0 + ghostLayers[2];
                    int c = cc*Ni*Nj;
                    for (int jj = j0; jj < j1; jj++)
                    {
                        int bb = jj - J0 + ghostLayers[1];
                        int b = c + bb*Ni;
                        for (int ii = i0; ii < i1; ii++)
                        {
                            int aa = ii - I0 + ghostLayers[0];
                            int a = b + aa;
                            avtGhostData::AddGhostZoneType(ghostData[a],
                                                     REFINED_ZONE_IN_AMR_GRID);
                        }
                    }
                }
            }
            else if (numDimensions == 2)
            {
                for (int jj = j0; jj < j1; jj++)
                {
                    int bb = jj - J0 + ghostLayers[1];
                    int b = bb*Ni;
                    for (int ii = i0; ii < i1; ii++)
                    {
                        int aa = ii - I0 + ghostLayers[0];
                        int a = b + aa;
                        avtGhostData::AddGhostZoneType(ghostData[a],
                                                     REFINED_ZONE_IN_AMR_GRID);
                    }
                }
            }
            else
            {
                for (int ii = i0; ii < i1; ii++)
                {
                    int a = ii - I0 + ghostLayers[0];
                    avtGhostData::AddGhostZoneType(ghostData[a],
                                                     REFINED_ZONE_IN_AMR_GRID);
                }
            }
        }
    }

    return didGhost;

}


// ****************************************************************************
//  Method: avtStructuredDomainNesting::ConfirmMesh
//
//  Purpose:
//      Confirms that the dataset is the one that this object is intended to
//      operate on.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
//  Modifications:
//
//    Hank Childs, Tue Jan 18 11:25:15 PST 2005
//    Fix typo that came up when dims[1] != dims[2].
//
//    Mark C. Miller, Wed Feb 16 14:32:55 PST 2005
//    Account for possibility that some meshes may not exist
//
//    Hank Childs, Fri Jan 20 17:11:59 PST 2006
//    Add warning messages for failure.
//
//    Hank Childs, Wed Aug  1 13:01:40 PDT 2007
//    Add warning for case where unstructured grid is sent in.
//
//    Hank Childs, Tue Oct  9 07:43:43 PDT 2007
//    Add support for avtRealDims.
//
//    Hank Childs, Sun Oct 28 14:30:31 PST 2007
//    Fix off-by-one error.  Real off-by-one error was in Chombo.  When that
//    bug got corrected, this got discovered.
//
// ****************************************************************************

bool
avtStructuredDomainNesting::ConfirmMesh(vector<int> &domains,
                                        vector<vtkDataSet *> &meshes)
{
    for (size_t i = 0 ; i < domains.size() ; i++)
    {
        int dims[3] = { -1, -1, -1 };

        if (meshes[i] == NULL)
            continue;

        int do_type = meshes[i]->GetDataObjectType();
        if (do_type == VTK_STRUCTURED_GRID)
        {
            vtkStructuredGrid *sgrid = (vtkStructuredGrid *) meshes[i];
            sgrid->GetDimensions(dims);
        }
        else if (do_type == VTK_RECTILINEAR_GRID)
        {
            vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) meshes[i];
            rgrid->GetDimensions(dims);
        }
        else
        {
            debug1 << "Got a non-structured mesh sent into "
                   << "avtStructuredDomainNesting.  This should not "
                   << "happen." << endl;
            return false;
        }
         
        if (meshes[i]->GetFieldData()->GetArray("avtRealDims") != NULL)
        {
            vtkIntArray *realDims = (vtkIntArray *) 
                            meshes[i]->GetFieldData()->GetArray("avtRealDims");
            dims[0] = realDims->GetValue(1)-realDims->GetValue(0)+1;
            dims[1] = realDims->GetValue(3)-realDims->GetValue(2)+1;
            dims[2] = realDims->GetValue(5)-realDims->GetValue(4)+1;
        }

        if (domains[i] >= domainNesting.size())
        {
            debug1 << "Warning: avtStructuredDomainNesting failing ConfirmMesh"
                   << " because domain number " << domains[i] << " was bigger "
                   << "than largest domain " << domainNesting.size() << endl;
            return false;
        }

        vector<int> &extents = domainNesting[domains[i]].logicalExtents;
        if ((extents[3]-extents[0]+2) != dims[0])
        {
            debug1 << "Warning: avtStructuredDomainNesting failing "
                   << " because declared extents in I " 
                   << extents[3]-extents[0]+2 
                   << " are bigger than data set size " << dims[0]
                   << "." << endl;
            return false;
        }
        if ((extents[4]-extents[1]+2) != dims[1])
        {
            debug1 << "Warning: avtStructuredDomainNesting failing "
                   << " because declared extents in J " 
                   << extents[4]-extents[1]+2 
                   << " are bigger than data set size " << dims[1]
                   << "." << endl;
            return false;
        }
        if (dims[2] > 1 && (extents[5]-extents[2]+2) != dims[2])
        {
            debug1 << "Warning: avtStructuredDomainNesting failing "
                   << " because declared extents in K " 
                   << extents[5]-extents[2]+2 
                   << " are bigger than data set size " << dims[2]
                   << "." << endl;
            return false;
        }
    }

    return true;
}


// ****************************************************************************
//  Method: avtStructuredDomainNesting::GetRatiosForLevel
//
//  Purpose:
//    Creates an array of indices for a dataset which allows translation from
//    its indexing scheme to another levels indexing scheme.  Array is of the 
//    form RI RJ RK OP where RI, RJ, & RK are the conversion factors for each 
//    dimension between the dataset's level and the requested AMR level.  OP is
//    the operation type needed to perform the conversion (multiply or divide). 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 22, 2007 
//
//  Modifications:
//
// ****************************************************************************

vector<int>
avtStructuredDomainNesting::GetRatiosForLevel(int level, int dom)
{
    if (level < 0 || level >= levelRatios.size())
        EXCEPTION2(BadIndexException, level, levelRatios.size());
    if (dom < 0 || dom >= domainNesting.size())
        EXCEPTION2(BadIndexException, dom, domainNesting.size());

    int myLevel        = domainNesting[dom].level;
    vector<int> rv;
    int i;

    // 0 for mult 1 for divide
    int multOrDivide = 0;
    int ratios[3] = {1, 1, 1}; 

    // how to translate the requested level to my level.
    if (myLevel > level)
    {
        multOrDivide = 0; // multiply
        // whats the multiplier for each dimension? 
        for (i = myLevel; i > level; i--)
        {
            ratios[0] *= levelRatios[i][0];
            ratios[1] *= (numDimensions >= 2 ? levelRatios[i][1] : 1);
            ratios[2] *= (numDimensions >= 3 ? levelRatios[i][2] : 1);
        }
    }
    else if (myLevel < level)
    {
        multOrDivide = 1; // divide
        for (i = level; i > myLevel; i--)
        {
            ratios[0] *= levelRatios[i][0];
            ratios[1] *= (numDimensions >= 2 ? levelRatios[i][1] : 1);
            ratios[2] *= (numDimensions >= 3 ? levelRatios[i][2] : 1);
        }
    }
    for (i = 0; i < 3; i++)
    {
        rv.push_back(ratios[i]);
    }
    rv.push_back(multOrDivide);
    
    return rv;
}


// ****************************************************************************
//  Method: avtStructuredDomainNesting::GetNestingForDomain
//
//  Purpose:
//      Returns the domains that nest inside this domain, as well as their
//      indices of overlap.  The indices are relative to the domain of
//      interest.  That is, if we are asking about domain 5, and domain 10
//      nests inside domain 5, then we convert domain 10's extents to be
//      relative to domain 5's indexing that is what is returned.
//
//  Arguments:
//      domain      The domain we should get nesting information for.
//      exts        The extents of domain (6 values).
//      children    The domain IDs of its children.
//      childExts   The [min|max][IJK] extents of each child domain (6 per)
//
//  Programmer: Hank Childs
//  Creation:   July 27, 2007
//
// ****************************************************************************

void
avtStructuredDomainNesting::GetNestingForDomain(int domain,
              vector<int> &exts, vector<int> &children, vector<int> &childExts)
{
    if (domain < 0 || domain >= domainNesting.size())
    {
        EXCEPTION2(BadIndexException, domain, domainNesting.size());
    }

    avtNestedDomainInfo_t &info = domainNesting[domain];
    exts     = info.logicalExtents;
    children = info.childDomains;
    childExts.clear();
    childExts.resize(6*children.size());
    for (size_t i = 0 ; i < children.size() ; i++)
    {
        vector<int> ratios = GetRatiosForLevel(info.level, children[i]);
        vector<int> rawExts = domainNesting[children[i]].logicalExtents;
        childExts[6*i+0] = rawExts[0] / ratios[0];
        childExts[6*i+3] = rawExts[3] / ratios[0];
        childExts[6*i+1] = rawExts[1] / ratios[1];
        childExts[6*i+4] = rawExts[4] / ratios[1];
        childExts[6*i+2] = rawExts[2] / ratios[2];
        childExts[6*i+5] = rawExts[5] / ratios[2];
    }
}
