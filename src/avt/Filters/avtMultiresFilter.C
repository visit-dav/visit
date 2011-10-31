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
//                            avtMultiresFilter.C                            //
// ************************************************************************* //

#include <avtMultiresFilter.h>

#include <avtDataAttributes.h>
#include <avtDomainNesting.h>
#include <avtExtents.h>
#include <avtMeshMetaData.h>
#include <avtMetaData.h>
#include <avtResolutionSelection.h>
#include <avtStructuredDomainNesting.h>
#include <DebugStream.h>

#include <float.h>
#include <limits.h>

#include <iterator>
#include <list>

// ****************************************************************************
//  Method: avtMultiresFilter constructor
//
//  Purpose:
//    Defines the constructor.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Oct 25 14:10:30 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

avtMultiresFilter::avtMultiresFilter(double *frust, double size)
{
    nDims = 3;
    for (int i = 0; i < 6; i++)
        frustum[i] = frust[i];
    smallestCellSize = size;
}

// ****************************************************************************
//  Method: avtMultiresFilter destructor
//
//  Purpose:
//    Defines the destructor.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Oct 25 14:10:30 PDT 2011
//
// ****************************************************************************

avtMultiresFilter::~avtMultiresFilter()
{
}

// ****************************************************************************
//  Method: avtMultiresFilter::Execute
//
//  Purpose:
//    Sends the specified input and output through the Multires filter. This
//    is basically a no-op, leaving the inputs unchanged and setting some
//    information in the data attributes.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Oct 25 14:10:30 PDT 2011
//
// ****************************************************************************

void
avtMultiresFilter::Execute(void)
{
    avtDataAttributes &dataAtts = GetTypedOutput()->GetInfo().GetAttributes();

    //
    // Set the multires extents.
    //
    avtExtents multiresExtents(nDims);
    multiresExtents.Set(frustum);
    dataAtts.SetMultiresExtents(&multiresExtents);

    //
    // Set the multires cell size.
    //
    dataAtts.SetMultiresCellSize(cellSize);

    //
    // Copy the input to the output.
    //
    avtDataTree_p newtree = GetInputDataTree();
    SetOutputDataTree(newtree);
}

// ****************************************************************************
//  Method: avtMultiresFilter::ModifyContract
//
//  Purpose:
//    Modify the contract to only serve up a subset of the chunks based on the
//    view frustum and the smallest cell size.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Oct 25 14:10:30 PDT 2011
//
// ****************************************************************************

avtContract_p avtMultiresFilter::ModifyContract(avtContract_p contract)
{
    //
    // Get the spatial extents for the mesh.
    //
    avtMetaData *md = GetMetaData();
    avtDataAttributes &dataAtts = GetInput()->GetInfo().GetAttributes();
    nDims = dataAtts.GetSpatialDimension();
    double extents[6];
    dataAtts.GetOriginalSpatialExtents()->CopyTo(extents);

    //
    // If the frustum is invalid, set it from the extents.
    //
    if (frustum[0] == DBL_MAX && frustum[1] == -DBL_MAX)
    {
        for (int i = 0; i < 6; i++)
            frustum[i] = extents[i];
    }

    //
    // Currently we are only implemented for 2d, so return if not 2d.
    //
    if (nDims != 2)
        return contract;

    //
    // Lookup the domain level information. If it gives this information,
    // we can turn off domains in the restriction and avoid reading and
    // processing all of them. If we can't get this information then return.
    //
    avtDomainNesting* dni = GetMetaData()->GetDomainNesting();
    if(dni == NULL)
        return contract;

    avtStructuredDomainNesting* sdn =
      dynamic_cast<avtStructuredDomainNesting*>(dni);
    if(sdn == NULL)
        return contract;

    //
    // Determine the number of levels and the logcial width of the
    // level 0 patch.
    //
    int maxLevel = -1;
    int minTopLogicalExtents[3] = {INT_MAX, INT_MAX, INT_MAX};
    int maxTopLogicalExtents[3] = {INT_MIN, INT_MIN, INT_MIN};

    size_t max_domain = sdn->GetNumberOfDomains();
    for(size_t dom=0; dom < max_domain; ++dom)
    {
        maxLevel = sdn->GetDomainLevel(dom) > maxLevel ?
            sdn->GetDomainLevel(dom) : maxLevel;
        if (sdn->GetDomainLevel(dom) == 0)
        {
            for (int i = 0; i < nDims; ++i)
            {
                minTopLogicalExtents[i] = sdn->GetDomainLogicalExtents(dom)[i] <
                    minTopLogicalExtents[i] ?
                    sdn->GetDomainLogicalExtents(dom)[i] :
                    minTopLogicalExtents[i];
                maxTopLogicalExtents[i] = sdn->GetDomainLogicalExtents(dom)[i+3] >
                    maxTopLogicalExtents[i] ?
                    sdn->GetDomainLogicalExtents(dom)[i+3] :
                    maxTopLogicalExtents[i];
            }
        }
    }

    int numLevels = maxLevel + 1;
    int topLogicalWidth[3];
    for (int i = 0; i < nDims; ++i)
        topLogicalWidth[i] = maxTopLogicalExtents[i] - minTopLogicalExtents[i] + 1;

    //
    // Determine the domains to display.
    //
    std::vector<int> domain_list;
    std::back_insert_iterator<std::vector<int> > doms(domain_list);

    double maxPatchDiag = 0.;
    int nVisible = 0;
    for(size_t dom=0; dom < max_domain; ++dom)
    {
        int level = sdn->GetDomainLevel(dom);
        std::vector<int> ratios = sdn->GetRatiosForLevel(0, dom);
        std::vector<int> logicalExtents = sdn->GetDomainLogicalExtents(dom);

        bool visible = true;
        double patchDx, frustumDx;
        double patchDiag = 0., frustumDiag = 0.;
        for (int i = 0; i < nDims; ++i)
        {
            patchDx = (extents[i*2+1] - extents[i*2]) / (topLogicalWidth[i] * ratios[i]);
            patchDiag += patchDx * patchDx;
            frustumDx = (frustum[i*2+1] - frustum[i*2]);
            frustumDiag += frustumDx * frustumDx;
            double min, max;
            min = double(logicalExtents[i]) * patchDx;
            max = (double(logicalExtents[i+3]) + 1.) * patchDx;
            if (max < frustum[i*2] || min > frustum[i*2+1])
                visible = false;
        }
        patchDiag = sqrt(patchDiag);
        frustumDiag = sqrt(frustumDiag);
        double ratio = patchDiag / frustumDiag;
        if (visible && ratio < smallestCellSize)
        {
            maxPatchDiag = patchDiag > maxPatchDiag ? patchDiag : maxPatchDiag;
        }
        if (ratio < smallestCellSize)
            visible = false;

        //
        // Always include the top level patches. This eliminates the problem
        // where all the patches are outside of the view frustum resulting
        // in no patches being sent resulting in a no data exception.
        //
        if (visible || sdn->GetDomainLevel(dom) == 0)
        {
            *doms = dom;
            nVisible++;
        }
    }
    cellSize = maxPatchDiag;

    contract->GetDataRequest()->GetRestriction()->RestrictDomains(domain_list);

    return contract;
}
