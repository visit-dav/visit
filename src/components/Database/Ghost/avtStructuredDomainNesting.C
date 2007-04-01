#include <avtStructuredDomainNesting.h>
#include <BadIndexException.h>
#include <UnexpectedValueException.h>
#include <VisItException.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkStructuredGrid.h>

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
//     dimension and the pre-existing vtkGhostLevels data
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
//  Method: avtStructuredDomainNesting::ApplyGhost
//
//  Purpose:
//    Applies "vtkGhostLevels" array to the meshes passed in 
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
//    Modified to account for fact that incomming meshes may already have ghost
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
//    assignment rather than bit operators for to avoid possible generation of
//    non-0/1 values.
//
// ****************************************************************************
bool
avtStructuredDomainNesting::ApplyGhost(vector<int> domainList,
                                       vector<int> allDomainList,
                                       vector<vtkDataSet*> meshes)
{
    bool didGhost = false;

    for (int i = 0; i < domainList.size(); i++)
    {

        unsigned char *ghostData = 0;

        vtkUnsignedCharArray *ghostArray = vtkUnsignedCharArray::SafeDownCast(
            meshes[i]->GetCellData()->GetArray("vtkGhostLevels"));

        int parentDom = domainList[i];
        int numCells = meshes[i]->GetNumberOfCells();
        bool parentAlreadyHasGhosts = (ghostArray != 0);

        // assume there are no boundary ghost layers
        int ghostLayers[3] = {0, 0, 0};
        if (parentAlreadyHasGhosts)
        {
            ghostData = (unsigned char *) ghostArray->GetVoidPointer(0);

            //
            // because number of ghost layers may vary from one variable to the next,
            // we have a strange problem here in that we know we have ghost layers but
            // don't know precisely how many in each dimension. So, we detect it from
            // knowledge of the non-ghosted size (in the nesting info's logical extents
            // of the parent) of the patch and the actual ghost data array
            //
            DetectBoundaryGhostLayers(numDimensions, ghostData, numCells,
                domainNesting[parentDom].logicalExtents, ghostLayers);

        }
        else
        {
            ghostArray = vtkUnsignedCharArray::New();
            ghostArray->SetNumberOfTuples(numCells);
            ghostData = (unsigned char *) ghostArray->GetVoidPointer(0);
            ghostArray->SetName("vtkGhostLevels");
            meshes[i]->GetCellData()->AddArray(ghostArray);
            ghostArray->Delete();
            memset(ghostData, 0x0, numCells);
        }

        //
        // Look at each of the children of this domain.
        // If in current selection, ghost the appropriate elements here 
        //
        for (int j = 0; j < domainNesting[parentDom].childDomains.size(); j++)
        {

            bool thisChildIsOn = false;
            for (int k = 0; k < allDomainList.size(); k++)
            {
                if (allDomainList[k] == domainNesting[parentDom].childDomains[j])
                {
                   thisChildIsOn = true;
                   break;
                }
            }

            //
            // whether the child is on or not, we need to go through and set the ghosts of
            // the parent accordingly. If the child is on, we need to set the ghost array
            // in the parent to 1 where appropriate. Otherwise, we need to set it to 0
            // We use the exclusive or operator below to achieve this
            //
            unsigned char ghostVal = 0x0;
            if (thisChildIsOn)
                ghostVal = 0x1;

            {
                int ratio[3];
                int childDom   = domainNesting[parentDom].childDomains[j];
                int childLevel = domainNesting[childDom].level;
                int Ni         = domainNesting[parentDom].logicalExtents[3] -
                                 domainNesting[parentDom].logicalExtents[0] +
                                 1 + 2 * ghostLayers[0];
                int Nj         = domainNesting[parentDom].logicalExtents[4] -
                                 domainNesting[parentDom].logicalExtents[1] +
                                 1 + 2 * ghostLayers[1];
                int Nk         = domainNesting[parentDom].logicalExtents[5] -
                                 domainNesting[parentDom].logicalExtents[2] +
                                 1 + 2 * ghostLayers[2];
                int I0         = domainNesting[parentDom].logicalExtents[0];
                int I1         = domainNesting[parentDom].logicalExtents[3] + 1;
                int J0         = domainNesting[parentDom].logicalExtents[1];
                int J1         = domainNesting[parentDom].logicalExtents[4] + 1;
                int K0         = domainNesting[parentDom].logicalExtents[2];
                int K1         = domainNesting[parentDom].logicalExtents[5] + 1;
                ratio[0]       = levelRatios[childLevel][0];
                ratio[1]       = numDimensions >= 2 ? levelRatios[childLevel][1] : 1;
                ratio[2]       = numDimensions >= 3 ? levelRatios[childLevel][2] : 1;
                int i0         = domainNesting[childDom].logicalExtents[0] / ratio[0];
                int i1         = (domainNesting[childDom].logicalExtents[3] + 1) / ratio[0];
                int j0         = domainNesting[childDom].logicalExtents[1] / ratio[1];
                int j1         = (domainNesting[childDom].logicalExtents[4] + 1) / ratio[1];
                int k0         = domainNesting[childDom].logicalExtents[2] / ratio[2];
                int k1         = (domainNesting[childDom].logicalExtents[5] + 1) / ratio[2];

                //
                // Often a child domain spans multiple parents. So, we need to
                // clip the bounds computed above to THIS parent's upper bounds
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
                    for (int ii = i0; ii < i1; ii++)
                        for (int jj = j0; jj < j1; jj++)
                            for (int kk = k0; kk < k1; kk++)
                            {
                                int a = ii - I0 + ghostLayers[0];
                                int b = jj - J0 + ghostLayers[1];
                                int c = kk - K0 + ghostLayers[2];
                                int idx = c*Ni*Nj + b*Ni + a;
                                ghostData[idx] = ghostVal;
                            }
                }
                else if (numDimensions == 2)
                {
                    for (int ii = i0; ii < i1; ii++)
                        for (int jj = j0; jj < j1; jj++)
                        {
                            int a = ii - I0 + ghostLayers[0];
                            int b = jj - J0 + ghostLayers[1];
                            int idx = b*Ni + a;
                            ghostData[idx] = ghostVal; 
                        }
                }
                else
                {
                    for (int ii = i0; ii < i1; ii++)
                    {
                        int a = ii - I0 + ghostLayers[0];
                        ghostData[a] = ghostVal;
                    }
                }
            }
        }
    }

    return didGhost;

}
