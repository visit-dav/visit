/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                            avtMapperBase.C                                //
// ************************************************************************* //

#include <avtMapperBase.h>

#include <vector>
#include <string>

#include <vtkDataSet.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtGeometryDrawable.h>
#include <avtTransparencyActor.h>

#include <ColorAttribute.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtMapperBase constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Mar 15 19:15:10 PST 2001
//    Added initialization of new members nRenderingModes, modeVisibility,
//    modeRepresentation.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001
//    Removed functionality related to having multiple rendering modes.
//    No longer needed.
//
//    Hank Childs, Sun Jul  7 12:31:10 PDT 2002
//    Initialize transparency actor as NULL.
//
//    Brad Whitlock, Mon Sep 23 16:56:08 PST 2002
//    I initialized the new immediateMode flag.
//
//    Kathleen Bonnell, Sat Oct 19 15:08:41 PDT 2002
//    Initialize globalAmbient.
//
//    Mark C. Miller, Thu Jan  9 13:31:32 PST 2003
//    Added initializiation for transparencyIndex data member to -1
//    Added initializations for externally rendered images actor
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Removed extRenderedImagesActor data member
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004
//    Initialize specularIsInappropriate.
//
//    Hank Childs, Sat Dec  3 20:32:37 PST 2005
//    Change test for whether or not we are doing software rendering.
//
//    Kathleen Biagas, Tue Jul 12 13:29:26 MST 2016
//    Remove drawSurfaces/Wireframe/Points and similar.
//
// ****************************************************************************

avtMapperBase::avtMapperBase()
{
    drawable = NULL;
    nMappers = 0;
    transparencyActor = NULL;
    transparencyIndex = -1;
    globalAmbient = 0.;
    specularIsInappropriate = false;
}


// ****************************************************************************
//  Method: avtMapperBase destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//    Kathleen Bonnell, Thu Mar 15 19:15:10 PST 2001
//    Added destruction of new members modeVisibility, modeRepresentation.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001
//    Removed functionality related to having multiple rendering modes.
//    No longer needed.
//
// ****************************************************************************

avtMapperBase::~avtMapperBase()
{
    ClearSelf();
}


// ****************************************************************************
//  Method: avtMapperBase::ChangedInput
//
//  Purpose:
//      A hook from avtDatasetSink telling us the input has changed.  We will
//      invalidate the drawable we previously had.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

void
avtMapperBase::ChangedInput(void)
{
    MapperChangedInput();
    ClearSelf();
}


// ****************************************************************************
//  Method: avtMapperBase::InputIsReady
//
//  Purpose:
//      Since we now know that the input is ready, this sets up the mappers.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Kathleen Biagas, Tue Jul 12 13:30:00 MST 2016
//    Moved setting of transparency actor, drawable out of SetUpMappers to
//    here, to ease Override of SetUpMappers.
//
//    Kathleen Biagas, Thu Jul 21 09:15:34 PDT 2016
//    Create GeometryDrawable, even if we have no mappers. Helps the pipeline
//    behave in a more user-friendly manner.
//
// ****************************************************************************

void
avtMapperBase::InputIsReady(void)
{
    avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
    if (inatts.GetRectilinearGridHasTransform())
    {
        // The renderer is smart enough to apply any extra transform
        // needed for rectilinear grids, but it will not have access
        // to the avtDataAttributes.  Instead, we insert the transform
        // as a field variable in any vtkRectilinearGrids.
        avtDataTree_p tree = GetInputDataTree();
        bool dummy;
        tree->Traverse(CInsertRectilinearTransformInfoIntoDataset,
                       (void*)inatts.GetRectilinearGridTransform(), dummy);
    }
    SetUpMappers();
    PrepareExtents();
    CustomizeMappers();
    SetUpTransparencyActor();

    SetUpDrawable();
}


// ****************************************************************************
//  Method: avtMapperBase::MapperChangedInput
//
//  Purpose:
//      This is a hook to allow derived types of avtMapperBase to reset their
//      state.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

void
avtMapperBase::MapperChangedInput(void)
{
    ;
}


// ****************************************************************************
//  Method: avtMapperBase::ClearSelf
//
//  Purpose:
//      Cleans up memory so a new drawable can be created.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//    Dave Bremer, Fri Mar  7 10:43:49 PST 2008
//    When this object clears itself, it should clear the drawable's
//    reference to 'this' as well.
// ****************************************************************************

void
avtMapperBase::ClearSelf(void)
{
    if (*drawable != NULL)
    {
        ((avtGeometryDrawable *)*drawable)->SetMapper(NULL);

        //
        // This probably doesn't need to be done, but it will guarantee that
        // we are never in an inconsistent state (where we have a valid
        // drawable, but no mappers).
        //
        drawable = NULL;
    }
}

void
avtMapperBase::ReleaseData(void)
{
   ClearSelf();
}


// ****************************************************************************
//  Method: avtMapperBase::GetDrawable
//
//  Purpose:
//      Gets a drawable from the input dataset.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

avtDrawable_p
avtMapperBase::GetDrawable(void)
{
    avtDataObject_p input = GetInput();
    if (*drawable == NULL || *input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return drawable;
}


// ****************************************************************************
//  Method: avtMapperBase::SetUpTransparencyActor
//
//  Purpose:
//      Sets up the transparancy actor with the current inputs.
//
//  Notes:  Pulled from old SetUpMappers method.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtMapperBase::SetUpTransparencyActor()
{
    ;
}


// ****************************************************************************
//  Method: avtMapperBase::SetUpMappers
//
//  Purpose:
//      Sets up the mappers and creates a drawable based on the input.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  9 15:44:57 PST 2001
//    Modified to reflect that GetInputDomain returns and avtDomainTree.
//
//    Kathleen Bonnell, Thu Feb 15 10:06:43 PST 2001
//    Add check for NULL tree on return from call to GetInputDomain.
//
//    Kathleen Bonnell, Thu Mar 15 19:15:10 PST 2001
//    Added logic to handle multiple rendering modes (creating extra
//    mappers/actors to handle this), and for setting actor visibility
//    and representation.
//
//    Hank Childs, Fri Mar 23 14:44:49 PST 2001
//    Added hook to allow derived classes to insert filters into the pipeline.
//
//    Hank Childs, Mon Mar 26 19:44:21 PST 2001
//    Added hook to allow derived classes to use their own mappers.
//
//    Kathleen Bonnell, Thu Apr 19 12:22:15 PDT 2001
//    Reflect that input now stored as single avtDatTree instead of multiple
//    avtDomainTrees.  Changed parameter in call to InsertFilters from
//    j (child num) to i (domain num).
//
//    Hank Childs, Sun Jun 24 19:52:13 PDT 2001
//    Added explicit initializations that only come up during error cases.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001
//    Removed functionality related to having multiple rendering modes.
//    No longer needed.
//
//    Kathleen Bonnell, Fri Sep 21 10:54:37 PDT 2001
//    Reflect that data trees are now compacted before being sent to viewer.
//
//    Kathleen Bonnell, Mon Sep 24 15:24:18 PDT 2001
//    Retrieve labels from input tree and from info attributes.  Call
//    SetLabels method so derived types may make us of the labels.
//
//    Hank Childs, Fri May 24 15:41:49 PDT 2002
//    Tell the drawable which mapper it came from.
//
//    Hank Childs, Sun Jul  7 12:31:10 PDT 2002
//    Add support for transparency.
//
//    Hank Childs, Sun Aug 18 21:06:24 PDT 2002
//    Do not use display lists if we have more than 250,000 cells.
//
//    Hank Childs, Wed Sep 18 11:04:10 PDT 2002
//    Do not use display lists if we are in no-win mode.
//
//    Brad Whitlock, Mon Sep 23 16:56:50 PST 2002
//    Changed the immediate mode rendering test so it uses a new member.
//
//    Hank Childs, Thu Mar 18 16:02:27 PST 2004
//    Do not create actors and mappers for empty datasets.
//
//    Hank Childs, Tue May 25 13:30:16 PDT 2004
//    Allow display lists of 250,000 polygons or more again, since the display
//    list generation was made much faster by breaking it into chunks.
//
//    Hank Childs, Tue Nov 18 05:46:45 PST 2008
//    Do not assume the tree is going to be non-NULL.
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PDT 2013
//    Changed signature of InsertFilters to return vtkAlgorithmOutput, so
//    connections are set up properly with vtk-6.
//
//    Kathleen Biagas, Tue Jul 12 13:31:09 MST 2016
//    Removed wireframe/surface/points specific settings.
//    Moved setting of transparency actor, drawable to InputIsReader
//    to make it easier for base classes to Override this method.
//
// ****************************************************************************

void
avtMapperBase::SetUpMappers(void)
{
    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    avtDataTree_p tree = GetInputDataTree();
    if (*tree == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    vector<string> labels;
    tree->GetAllLabels(labels);
    if (!labels.empty() )
    {
        SetLabels(labels, true);
        labels.clear();
    }

    input->GetInfo().GetAttributes().GetLabels(labels);
    if (!labels.empty())
    {
        SetLabels(labels, false);
        labels.clear();
    }

    vtkDataSet **children = NULL;
    children = tree->GetAllLeaves(nMappers);

    CreateActorMapperPairs(children);

    // this was allocated in GetAllLeaves, need to free it now
    if (children != NULL)
        delete [] children;
}


// ****************************************************************************
//  Method: avtMapperBase::GetRange
//
//  Purpose:
//      Gets the range of the input.  This will looks at the extents in the
//      information if they are set and look at the dataset otherwise.
//
//  Arguments:
//      rmin    Will hold the minimum value.
//      rmax    Will hold the maximum value.
//
//  Returns:    True if the extents were calculated successfully,
//              False otherwise.
//
//  Programmer: Hank Childs
//  Creation:   April 20, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep  7 18:30:33 PDT 2001
//    Sent a double into GetDataExtents for new interface.
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Account for dataset examiner.
//
//    Sean Ahern, Thu May 16 12:13:40 PDT 2002
//    Added some debugging.
//
//    Hank Childs, Thu Oct 10 08:22:31 PDT 2002
//    Added a return value.
//
//    Brad Whitlock, Wed Dec 4 11:39:48 PDT 2002
//    I changed the code so it supports calculating extents of vector
//    magnitudes.
//
//    Hank Childs, Tue Sep 23 23:08:19 PDT 200
//    Support tensor magnitudes as the eigenvalues extents.
//
//    Kathleen Bonnell, Thu Mar 11 10:07:35 PST 2004
//    Tensor, vectors and scalars all handled by GetDataExtents.
//
//    Jeremy Meredith, Fri Jun 18 14:18:23 PDT 2004
//    Allow variable dimension of 4.  This will be used for colors.
//
// ****************************************************************************

bool
avtMapperBase::GetRange(double &rmin, double &rmax)
{
    if(*GetInput() == 0)
    {
        debug1 << "avtMapperBase::GetRange: The mapper has no input!" << endl;
        EXCEPTION0(ImproperUseException);
    }

    bool gotExtents = false;
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    if ((data.GetVariableDimension() == 1) ||
        (data.GetVariableDimension() <= 3) ||
        (data.GetVariableDimension() == 4) ||
        (data.GetVariableDimension() == 9))
    {
        double extents[2];
        gotExtents = data.GetDataExtents(extents);

        if (gotExtents)
        {
            rmin = extents[0];
            rmax = extents[1];
        }
        else
        {
            double de[2];
            avtDataset_p input = GetTypedInput();
            gotExtents = avtDatasetExaminer::GetDataExtents(input, de);

            rmin = de[0];
            rmax = de[1];
        }
    }
    else
    {
        debug1 << "avtMapperBase::GetRange: Invalid variable dimension: dim=" <<
            data.GetVariableDimension() << endl;
        EXCEPTION0(ImproperUseException);
    }

    return gotExtents;
}


// ****************************************************************************
//  Method: avtMapperBase::PrepareExtents
//
//  Purpose:
//      Prepares the extents of a dataset.  This means trying to figure out
//      what the current extents are by looking at the original extents and
//      finally the data itself.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Sep  5 16:56:35 PDT 2001
//    Removed unnecessary temporary variable to fix compiler warnings.
//
//    Hank Childs, Fri Sep  7 18:30:33 PDT 2001
//    Sent doubles into avtDataset::Get...Extent routines.
//
//    Kathleen Bonnell, Wed Oct 10 13:32:31 PDT 2001
//    Retrieve/set current data extents.
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Account for dataset examiner.
//
//    Hank Childs, Fri Feb 27 08:29:54 PST 2004
//    Account for multiple variables.
//
//    Mark C. Miller, Sun Feb 29 18:35:00 PST 2004
//    Added calls to GetAnySpatialExtents before arbitrarily setting to [0,1]
//
//    Kathleen Bonnell, Thu Mar 11 10:07:35 PST 2004
//    DataExtents now always have only 2 components.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
// ****************************************************************************

void
avtMapperBase::PrepareExtents(void)
{
    avtDataset_p input = GetTypedInput();

    avtDataAttributes &atts = input->GetInfo().GetAttributes();

    int nvars = atts.GetNumberOfVariables();
    double exts[2];
    for (int var = 0 ; var < nvars ; var++)
    {
        const char *vname = atts.GetVariableName(var).c_str();
        bool gotDataExtents = atts.GetDataExtents(exts, vname);
        if (!gotDataExtents)
        {
            if (!(avtDatasetExaminer::GetDataExtents(input, exts, vname)))
            {
                exts[0] = 0.;
                exts[1] = 1.;
            }
        }
        atts.GetOriginalDataExtents(vname)->Set(exts);

        bool gotActualDataExtents = atts.GetActualDataExtents(exts, vname);
        if (!gotActualDataExtents)
        {
            if (!(avtDatasetExaminer::GetDataExtents(input, exts, vname)))
            {
                exts[0] = 0.;
                exts[1] = 1.;
            }
        }
        atts.GetActualDataExtents(vname)->Set(exts);
    }

    double bounds[6];
    bool gotBounds = atts.GetSpatialExtents(bounds);
    if (!gotBounds)
    {
        if (!(avtDatasetExaminer::GetSpatialExtents(input, bounds)))
        {
            if (!atts.GetAnySpatialExtents(bounds))
            {
                for (int i = 0 ; i < 3 ; i++)
                {
                    bounds[2*i] = 0.;
                    bounds[2*i+1] = 1.;
                }
            }
        }
    }
    atts.GetOriginalSpatialExtents()->Set(bounds);
}


// ****************************************************************************
//  Method: avtMapperBase::SetLabels
//
//  Purpose:
//      A hook to allow derived types to make use of labels.
//
//  Arguments:
//    <unnamed>  The labels.
//    <unnamed>  True if labels were retrieved from the input data tree,
//               false if labels were retrieved from input info.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 24, 2001
//
// ****************************************************************************

void
avtMapperBase::SetLabels(vector<string> &, bool)
{
   ;
}


// ****************************************************************************
//  Method: avtMapperBase::GetCurrentRange
//
//  Purpose:
//      Gets the current range of the input.  This will look at the extents
//      in the information if they are set and look at the dataset otherwise.
//
//  Arguments:
//      rmin     Will hold the minimum value.
//      rmax     Will hold the maximum value.
//
//  Returns:    True, if it successfully calculated the extents,
//              False otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 3, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Oct 10 08:25:18 PDT 2002
//    Added a return value.
//
//    Kathleen Bonnell, Wed Dec 22 16:36:29 PST 2004
//    Removed exception for variable dimension != 1, as all var extents now
//    contain two elements, regardless of dimension.
//
// ****************************************************************************

bool
avtMapperBase::GetCurrentRange(double &rmin, double &rmax)
{
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    double extents[2];
    bool gotExtents = data.GetActualDataExtents(extents);

    if (!gotExtents)
    {
        //
        //  avtDataset retrives extents from the tree, so this will get us
        //  the proper current range.
        //
        avtDataset_p input = GetTypedInput();
        gotExtents = avtDatasetExaminer::GetDataExtents(input, extents);
    }

    rmin = extents[0];
    rmax = extents[1];

    return gotExtents;
}


// ****************************************************************************
//  Method: avtMapperBase::GetLighting
//
//  Purpose:
//      Returns the lighting state for a particular mapper.
//      Derived classes may override as necessary.
//
//  Returns:
//      True if lighting for this mapper is ON, false otherwise.
//      Default is ON.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 13, 2002
//
// ****************************************************************************

bool
avtMapperBase::GetLighting()
{
   return true;
}


// ****************************************************************************
//  Method: avtMapperBase::InvalidateTransparencyCache
//
//  Purpose:
//      Invalidates transparency cache.
//
//  Notes:
//      Can be used by plots when transparency cache won't be invalidated
//      through normal methods.  Eg during a SetColorTable call from viewer.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtMapperBase::InvalidateTransparencyCache()
{
    if (transparencyActor != NULL)
        transparencyActor->InvalidateTransparencyCache();
}

