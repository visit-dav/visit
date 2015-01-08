/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <avtCallback.h>
#include <avtDataAttributes.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtDomainNesting.h>
#include <avtExtents.h>
#include <avtMeshMetaData.h>
#include <avtMetaData.h>
#include <avtMultiresSelection.h>
#include <avtStructuredDomainNesting.h>
#include <avtView2D.h>
#include <avtView3D.h>
#include <DebugStream.h>

#include <InvalidFilesException.h>

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
//    Eric Brugger, Fri Dec 20 11:52:45 PST 2013
//    Add support for doing multi resolution data selections.
//
//    Eric Brugger, Thu Jan  2 15:16:28 PST 2014
//    Add support for 3d multi resolution data selections.
//
//    Eric Brugger, Wed Jan  8 17:00:23 PST 2014
//    I added a ViewArea to the multi resolution data selection since the
//    view frustum was insufficient in 3d.
//
// ****************************************************************************

avtMultiresFilter::avtMultiresFilter(double *trans2D, double *trans3D,
    double *vp2D, double *vp3D, int *size, double area2D, double area3D,
    double *extents2D, double *extents3D, double area)
{
    nDims = 3;
    for (int i = 0; i < 16; i++)
        transform2D[i] = trans2D[i];
    for (int i = 0; i < 16; i++)
        transform3D[i] = trans3D[i];
    for (int i = 0; i < 6; i++)
        viewport2D[i] = vp2D[i];
    for (int i = 0; i < 6; i++)
        viewport3D[i] = vp3D[i];
    windowSize[0] = size[0];
    windowSize[1] = size[1];
    viewArea2D = area2D;
    viewArea3D = area3D;
    for (int i = 0; i < 6; i++)
        desiredExtents2D[i] = extents2D[i];
    for (int i = 0; i < 6; i++)
        desiredExtents3D[i] = extents3D[i];
    desiredCellArea = area;

    selID = -1;
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
//  Modifications:
//    Eric Brugger, Fri Dec 20 11:52:45 PST 2013
//    Add support for doing multi resolution data selections.
//
// ****************************************************************************

void
avtMultiresFilter::Execute(void)
{
    avtDataAttributes &dataAtts = GetTypedOutput()->GetInfo().GetAttributes();

    if (selID != -1)
    {
        //
        // The database can do multiresolution so just set the multires
        // extents and cell size so that Visit doesn't complain.
        //
        avtExtents *oldExtents = dataAtts.GetMultiresExtents();
        double oldFrustum[6];
        oldExtents->CopyTo(oldFrustum);

        avtExtents multiresExtents(nDims);
        multiresExtents.Set(oldFrustum);
        dataAtts.SetMultiresExtents(&multiresExtents);

        double oldCellSize;
        oldCellSize = dataAtts.GetMultiresCellSize();
        dataAtts.SetMultiresCellSize(oldCellSize);
    }
    else
    {
        //
        // The database can't do multiresolution so set the multires
        // extents and cell size using the information from this class.
        //
        avtExtents multiresExtents(nDims);
        multiresExtents.Set(desiredExtents);
        dataAtts.SetMultiresExtents(&multiresExtents);

        dataAtts.SetMultiresCellSize(actualCellArea);
    }

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
//    view frustum and the desired cell size.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Oct 25 14:10:30 PDT 2011
//
//  Modifications:
//    Eric Brugger, Fri Dec 20 11:52:45 PST 2013
//    Add support for doing multi resolution data selections.
//
//    Eric Brugger, Thu Jan  2 15:16:28 PST 2014
//    Add support for 3d multi resolution data selections.
//
//    Eric Brugger, Wed Jan  8 17:00:23 PST 2014
//    I added a ViewArea to the multi resolution data selection since the
//    view frustum was insufficient in 3d.
//
// ****************************************************************************

avtContract_p avtMultiresFilter::ModifyContract(avtContract_p contract)
{
    //
    // Get the desired frustum and the spatial extents for the mesh.
    //
    avtDataAttributes &dataAtts = GetInput()->GetInfo().GetAttributes();
    nDims = dataAtts.GetSpatialDimension();
    if (nDims == 2)
    {
        for (int i = 0; i < 16; i++)
            transform[i] = transform2D[i];
        for (int i = 0; i < 6; i++)
            viewport[i] = viewport2D[i];
        viewArea = viewArea2D;
        for (int i = 0; i < 6; i++)
            desiredExtents[i] = desiredExtents2D[i];
    }
    else
    {
        for (int i = 0; i < 16; i++)
            transform[i] = transform3D[i];
        for (int i = 0; i < 6; i++)
            viewport[i] = viewport3D[i];
        viewArea = viewArea3D;
        for (int i = 0; i < 6; i++)
            desiredExtents[i] = desiredExtents3D[i];
    }
    double extents[6];
    dataAtts.GetOriginalSpatialExtents()->CopyTo(extents);

    //
    // Calculate the desired extents and view area.
    //
    if (viewport[0] != DBL_MAX && viewport[1] != DBL_MAX)
    {
        //
        // The transform is valid, calculate the desired extents and
        // view area from them.
        //
        if (nDims == 2)
        {
            avtView2D::CalculateExtentsAndArea(desiredExtents, viewArea,
                                            transform);
        }
        else
        {
            avtView3D::CalculateExtentsAndArea(desiredExtents, viewArea,
                                            transform);
        }
    }
    else
    {
        //
        // The transform is invalid, set the desired extents and view area
        // from the extents.  Note that the extents may also be invalid so
        // we need to check for validity before calculating the viewArea to
        // avoid an arithmetic overflow.
        //
        for (int i = 0; i < 6; i++)
            desiredExtents[i] = extents[i];
        viewArea = DBL_MAX;
        if (nDims == 2)
        {
            if (extents[0] != DBL_MAX && extents[1] != -DBL_MAX)
            {
                viewArea = (extents[1] - extents[0]) *
                           (extents[3] - extents[2]);
            }
        }
        else
        {
            if (extents[0] != DBL_MAX && extents[1] != -DBL_MAX)
            {
                //
                // We don't know the aspect ratio of the window and the
                // zoom factor so assume they are both 1.
                //
                double ratio = 1.;
                double imageZoom = 1.;
                double parallelScale = 0.5 * sqrt((extents[1]-extents[0]) *
                                                  (extents[1]-extents[0]) +
                                                  (extents[3]-extents[2]) *
                                                  (extents[3]-extents[2]) +
                                                  (extents[5]-extents[4]) *
                                                  (extents[5]-extents[4]));
                viewArea = (parallelScale * parallelScale * ratio) /
                           (imageZoom * imageZoom);
            }
        }
    }

    //
    // If the format can do multires then add a multi resolution data
    // selection to the contract and return.
    //
    std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
    ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
    if (*dbp == NULL)
        EXCEPTION1(InvalidFilesException, db.c_str());
    avtDatabaseMetaData *dbmd = dbp->GetMetaData(0,true,true,false);

    if (dbmd->GetFormatCanDoMultires())
    {
        avtMultiresSelection *selection = new avtMultiresSelection;
        selection->SetCompositeProjectionTransformMatrix(transform);
        selection->SetViewport(viewport);
        selection->SetSize(windowSize);
        selection->SetViewArea(viewArea);
        selection->SetDesiredExtents(desiredExtents);
        selection->SetDesiredCellArea(desiredCellArea);
        selID = contract->GetDataRequest()->AddDataSelection(selection);

        return contract;
    }

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

    int max_domain = (int)sdn->GetNumberOfDomains();
    for(int dom=0; dom < max_domain; ++dom)
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

    int topLogicalWidth[3];
    for (int i = 0; i < nDims; ++i)
        topLogicalWidth[i] = maxTopLogicalExtents[i] - minTopLogicalExtents[i] + 1;

    //
    // Determine the domains to display.
    //
    std::vector<int> domain_list;
    std::back_insert_iterator<std::vector<int> > doms(domain_list);

    double viewSize = sqrt(viewArea);
    double maxPatchSize = 0.;
    int nVisible = 0;
    for(int dom=0; dom < max_domain; ++dom)
    {
        std::vector<int> ratios = sdn->GetRatiosForLevel(0, dom);
        std::vector<int> logicalExtents = sdn->GetDomainLogicalExtents(dom);

        bool visible = true;
        double patchDx;
        double patchVolume = 1.;
        for (int i = 0; i < nDims; ++i)
        {
            patchDx = (extents[i*2+1] - extents[i*2]) / (topLogicalWidth[i] * ratios[i]);
            patchVolume *= patchDx;
            double min, max;
            min = double(logicalExtents[i]) * patchDx;
            max = (double(logicalExtents[i+3]) + 1.) * patchDx;
            if (max < desiredExtents[i*2] || min > desiredExtents[i*2+1])
                visible = false;
        }
        double patchSize = pow(patchVolume, 1. / double(nDims));
        double ratio = patchSize / viewSize;
        if (visible && ratio < desiredCellArea)
        {
            maxPatchSize = patchSize > maxPatchSize ? patchSize : maxPatchSize;
        }
        if (ratio < desiredCellArea)
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
    actualCellArea = maxPatchSize;

    contract->GetDataRequest()->GetRestriction()->RestrictDomains(domain_list);

    return contract;
}
