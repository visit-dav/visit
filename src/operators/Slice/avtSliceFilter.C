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
//                             avtSliceFilter.C                              //
// ************************************************************************* //

#include <avtSliceFilter.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSlicer.h>
#include <vtkTransformFilter.h>

#include <avtCallback.h>
#include <avtDataset.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtPlaneSelection.h>
#include <avtSpatialBoxSelection.h>
#include <avtOriginatingSource.h>

#include <BadVectorException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

#include <snprintf.h>

using     std::vector;


static bool      PlaneIntersectsCube(float plane[4], float bounds[6]);
static void      FindCells(float *x, float *y, float *z, int nx, int ny, 
                           int nz, int *clist, int &ncells, float *plane, 
                           int dim, int ax, int ay, int az, int onx, int ony,
                           int onz);
static void      ProjectExtentsCallback(const double *in, double *out,
                                        void *args);

// ****************************************************************************
// Class: vtkVisItMatrixToLinearTransform
//
// Purpose:
//   Linear transform subclass that has the option of not transforming vectors.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 15:22:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class vtkVisItMatrixToLinearTransform : public vtkMatrixToLinearTransform
{
public:
  static vtkVisItMatrixToLinearTransform *New() { return new vtkVisItMatrixToLinearTransform; }

  void PrintSelf (ostream& os, vtkIndent indent)
  {
      vtkMatrixToLinearTransform::PrintSelf(os, indent);
  }

  void SetDoTransformVectors(bool val)
  {
      doTransformVectors = val;
      this->Modified();
  }

  virtual void TransformVectors (vtkDataArray *inVrs, vtkDataArray *outVrs)
  {
      if(doTransformVectors)
          vtkMatrixToLinearTransform::TransformVectors(inVrs, outVrs);
      else
      {
          for(vtkIdType id = 0; id < inVrs->GetNumberOfTuples(); ++id)
              outVrs->SetTuple(id, inVrs->GetTuple(id));
      }
  }
protected:
  bool doTransformVectors;

  vtkVisItMatrixToLinearTransform()  { doTransformVectors = true; }
  ~vtkVisItMatrixToLinearTransform() { }
};

// ****************************************************************************
//  Method: avtSliceFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   July 25, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  8 16:56:24 PDT 2000
//    Changed argument to be a plane from a cutter to ensure that our cutter
//    stems from a plane.
//
//    Jeremy Meredith, Tue Sep 19 22:27:16 PDT 2000
//    Made the avtSliceFilter initialize from the raw origin and normal
//    and create the plane itself.
//
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Folded functionality of avtProjectFilter into this class.
//    Made attributes be stored as an SliceAttributes class.
//
//    Hank Childs, Fri Mar 15 19:33:24 PST 2002
//    Initialize point.
//
//    Hank Childs, Mon Sep 16 17:34:06 PDT 2002
//    Initialize transform.
//
//    Kathleen Bonnell, Thu Apr 10 11:25:01 PDT 2003  
//    Initialize inverse transformation matrix. 
//
//    Jeremy Meredith, Mon May  5 14:31:45 PDT 2003
//    Removed "point" for now.  The slice window has changed, and dynamically
//    resolved attributes will work differently soon.
//
//    Hank Childs, Mon Jun  9 09:20:43 PDT 2003
//    Use the new vtkSlicer class.
//
//    Kathleen Bonnell, Wed Jun  2 09:11:01 PDT 2004 
//    Added origTrans. 
//
//    Hank Childs, Fri Aug 19 08:57:27 PDT 2005
//    Use vtkTransformFilter instead of vtkTransformPolyDataFilter ['6471].
//
// ****************************************************************************

avtSliceFilter::avtSliceFilter()
{
    slicer = vtkSlicer::New();
    transform = vtkTransformFilter::New();
    celllist = NULL;
    invTrans = vtkMatrix4x4::New();
    origTrans = vtkMatrix4x4::New();
    cachedOrigin[0] = 0.;
    cachedOrigin[1] = 0.;
    cachedOrigin[2] = 0.;
    doTransformVectors = true;
}


// ****************************************************************************
//  Method: avtSliceFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   July 25, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  8 16:56:24 PDT 2000
//    Added Delete for plane.
//
//    Hank Childs, Fri Mar 15 19:33:24 PST 2002
//    Destruct point.
//
//    Hank Childs, Mon Sep 16 17:34:06 PDT 2002
//    Destruct transform.
//
//    Kathleen Bonnell, Thu Apr 10 11:25:01 PDT 2003  
//    Delete inverse transformation matrix. 
//
//    Jeremy Meredith, Mon May  5 14:31:45 PDT 2003
//    Removed "point" for now.  The slice window has changed, and dynamically
//    resolved attributes will work differently soon.
//
//    Hank Childs, Mon Jun  9 09:20:43 PDT 2003
//    Use the new vtkSlicer class.
//
//    Kathleen Bonnell, Wed Jun  2 09:11:01 PDT 2004 
//    Added origTrans. 
//
// ****************************************************************************

avtSliceFilter::~avtSliceFilter()
{
    if (slicer != NULL)
    {
        slicer->Delete();
        slicer = NULL;
    }
    if (celllist != NULL)
    {
        delete [] celllist;
        celllist = NULL;
    }
    if (transform != NULL)
    {
        transform->Delete();
        transform = NULL;
    }
    if (invTrans!= NULL)
    {
        invTrans->Delete();
        invTrans= NULL;
    }
    if (origTrans!= NULL)
    {
        origTrans->Delete();
        origTrans= NULL;
    }
}


// ****************************************************************************
//  Method:  avtSliceFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 13 10:38:03 PDT 2001
//    Change type of return value.
//
// ****************************************************************************

avtFilter *
avtSliceFilter::Create()
{
    return new avtSliceFilter();
}


// ****************************************************************************
//  Method:  avtSliceFilter::SetAtts
//
//  Purpose:
//    Set the attributes of the filter
//
//  Arguments:
//    a          the atts
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Fri Mar 15 17:15:07 PST 2002
//    Made it use the origin as a Point.
//
//    Hank Childs, Fri Mar 15 19:33:24 PST 2002
//    Initialize point attribute.
//
//    Hank Childs, Mon Sep 16 17:39:49 PDT 2002
//    Clean up memory leak.
//
//    Kathleen Bonnell, Thu Apr 10 11:25:01 PDT 2003  
//    Save inverse transformation matrix for project-2d. 
//
//    Jeremy Meredith, Mon May  5 14:31:45 PDT 2003
//    Changed the way "origin" works.
//
//    Kathleen Bonnell, Tue May 20 16:02:52 PDT 2003  
//    Added tests for valid Normal, upAxis.
//
//    Hank Childs, Mon Jun  9 09:20:43 PDT 2003
//    Use the new vtkSlicer class.
//    
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Added theta-phi method of editing the plane normal.    
//
// ****************************************************************************

void
avtSliceFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const SliceAttributes*)a;

    double nx = 0, ny = 0, nz = 0;
    GetNormal( nx, ny, nz );
    cachedNormal[0] = nx;
    cachedNormal[1] = ny;
    cachedNormal[2] = nz;    

    //
    // Make sure the Normal is valid. 
    //
    if (nx == 0. && ny == 0. && nz == 0.)
    {
        EXCEPTION1(BadVectorException, "Normal");
        return;
    }
}


// ****************************************************************************
//  Method: avtSliceFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtSliceFilter with the given
//      parameters would result in an equivalent avtSliceFilter.
//
//  Arguments:
//      ox,oy,oz   The origin of the slice plane
//      nx,ny,nz   The normal of the slice plane
//
//  Programmer: Jeremy Meredith
//  Creation:   September 19, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Made attributes be stored as an SliceAttributes class.
//
// ****************************************************************************

bool
avtSliceFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(SliceAttributes*)a);
}


// ****************************************************************************
//  Method: avtSliceFilter::ModifyContract
//
//  Purpose:
//      Modifies the contract for the slice filter.
//
//  Arguments:
//      spec    The current contract.
//
//  Returns:    The new contract
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 26 10:43:23 PST 2002 
//    Added code to turn on zone-numbers as needed (2d only).
//
//    Kathleen Bonnell, Wed Jun 19 12:10:34 PDT 2002 
//    Don't turn off zone numbers if they have been turned on elsewhere in
//    the pipeline. 
//
//    Kathleen Bonnell, Wed Jun 19 13:42:37 PDT 2002  
//    Completely removed the code turning off zone numbers.  Why set a flag
//    to false if it is already false?  False is the default setting.
//    
//    Hank Childs, Wed Jun 18 11:06:30 PDT 2003
//    Request arrays if necessary for different slice types.
//
//    Hank Childs, Mon Jul 14 19:50:30 PDT 2003
//    Make sure that the info about the origin is up-to-date before eliminating
//    domains from potential calculation.
//
//    Kathleen Bonnell, Wed Jun  2 09:11:01 PDT 2004 
//    Turn on node numbers when appropriate. 
// 
//    Hank Childs, Tue Jun 29 07:24:11 PDT 2004
//    Use interval trees when slicing by percent.
//
//    Kathleen Bonnell, Tue Aug 10 09:20:32 PDT 2004 
//    Always turn on Node/Zone numbers, because Pick will not work correctly
//    without them. 
//
//    Mark C. Miller, Tue Sep 28 19:32:50 PDT 2004
//    Added code to populate a data selection for this operator
//
//    Hank Childs, Thu Mar  3 16:05:20 PST 2005
//    Don't allow dynamic load balancing if we will need to communicate the
//    point location.
//
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
//    Hank Childs, Wed Aug  1 11:54:51 PDT 2007
//    Add special logic for simplified nesting representations (i.e AMR
//    subset plots).  This is a very special case in that it is easy
//    to get very speedy results, but VisIt's infrastructure makes it hard.
//    Slice is turning on zone numbers by default.  If we are doing a wireframe
//    subset plot, then I'm stopping these zone numbers.  I am having the
//    output marked so that it is non-pickable ... so this should all still
//    work fine.
//
//    Hank Childs, Wed Aug 15 10:39:51 PDT 2007
//    Whoops ... I added bad logic for the change above.  Fixed now.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
//    Hank Childs, Fri Nov  2 16:40:46 PDT 2007
//    If the spatial meta data is invalidated, then don't use it.
//
//    Brad Whitlock, Wed Jan 23 15:49:00 PST 2008
//    Set the doTransformVectors flag.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Thu Nov 20 14:07:56 PST 2008
//    Making streaming be always disabled, because of the collective communication
//    in PreExecute.
//
//    Hank Childs, Mon Jan  5 15:41:16 CST 2009
//    Add a data selection.
//
// ****************************************************************************

avtContract_p
avtSliceFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = new avtContract(contract);

    //
    // Pick returns wrong results (even with transform) when slice lies
    // along boundary between zones.  So always turn on zone, node
    // numbers.  WE MAY WANT TO REVERT BACK IN THE FUTURE IF A
    // BETTER WAY CAN BE FOUND FOR PICK TO RETURN CORRECT RESULTS.  
    bool needToTurnOnIds = true;

    // 
    // Add the zone and node numbers for subset plots of rectilinear/AMR
    // grids blows up memory.  In this case, do not request IDs, since 
    // the database can mark their output as non-pickable, which means
    // that we don't need the IDs after all...
    //
    if (contract->GetDataRequest()->GetSimplifiedNestingRepresentation())
        needToTurnOnIds = false;

    if (needToTurnOnIds)
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }

    // Get the flag indicating whether vectors must be projected during 
    // project to 2D and save for later.
    doTransformVectors = rv->GetDataRequest()->TransformVectorsDuringProject();
         
#if 0
    if (atts.GetProject2d() && rv->GetDataRequest()->MayRequireZones())
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (atts.GetProject2d() && rv->GetDataRequest()->MayRequireNodes())
    {
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }

    if (atts.GetOriginType() == SliceAttributes::Zone)
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (atts.GetOriginType() == SliceAttributes::Node)
    {
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }
#endif

    //
    // Setup data selection characteristics of this operator 
    // Currently, we only handle orthogonal slice here
    //
    if ((atts.GetAxisType() == SliceAttributes::XAxis) ||
        (atts.GetAxisType() == SliceAttributes::YAxis) ||
        (atts.GetAxisType() == SliceAttributes::ZAxis))
    {
        avtSpatialBoxSelection *sel = new avtSpatialBoxSelection;
        sel->SetInclusionMode(avtSpatialBoxSelection::Partial);
        double origin[3];
        double mins[3], maxs[3];
        GetOrigin(origin[0], origin[1], origin[2]);
        if (atts.GetAxisType() == SliceAttributes::XAxis)
        {
            mins[0] = origin[0]; mins[1] = -FLT_MAX; mins[2] = -FLT_MAX;
            maxs[0] = origin[0]; maxs[1] = +FLT_MAX; maxs[2] = +FLT_MAX;
        }
        else if (atts.GetAxisType() == SliceAttributes::YAxis)
        {
            mins[0] = -FLT_MAX; mins[1] = origin[1]; mins[2] = -FLT_MAX;
            maxs[0] = +FLT_MAX; maxs[1] = origin[1]; maxs[2] = +FLT_MAX;
        }
        else if (atts.GetAxisType() == SliceAttributes::ZAxis)
        {
            mins[0] = -FLT_MAX; mins[1] = -FLT_MAX; mins[2] = origin[2];
            maxs[0] = +FLT_MAX; maxs[1] = +FLT_MAX; maxs[2] = origin[2];
        }
        sel->SetMins(mins);
        sel->SetMaxs(maxs);
        rv->GetDataRequest()->AddDataSelection(sel);
    }

    //
    // We can't do streaming because:
    //  (A) we do collective communication to get the bounding box
    //      of the whole data set in PreExecute.
    //  (B) if we need zone or node ids, we can't do streaming.
    //
    rv->NoStreaming();

    //
    // Get the interval tree.  If we can't use the interval tree, then exit
    // early.
    //
    if (! GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
        return rv;

    avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
    if (it == NULL)
    {
        if (atts.GetOriginType() == SliceAttributes::Percent)
            rv->NoStreaming();
        return rv;
    }

    //
    // Give the interval tree the linear equation of the plane and have it
    // return a domain list.
    //
    if (atts.GetOriginType() == SliceAttributes::Point ||
        atts.GetOriginType() == SliceAttributes::Intercept || 
        atts.GetOriginType() == SliceAttributes::Percent)
    {
        double normal[3]
             = {cachedNormal[0], cachedNormal[1], cachedNormal[2]};
        double origin[3];
        if (atts.GetOriginType() == SliceAttributes::Percent)
        {
            double percent = atts.GetOriginPercent() / 100.;
            double bounds[6];
            it->GetExtents(bounds);
            origin[0] = (bounds[1] - bounds[0])*percent + bounds[0];
            origin[1] = (bounds[3] - bounds[2])*percent + bounds[2];
            origin[2] = (bounds[5] - bounds[4])*percent + bounds[4];
        }
        else
        {
            GetOrigin(origin[0], origin[1], origin[2]);
        }
        double tmpD = normal[0]*origin[0] + normal[1]*origin[1] +
                     normal[2]*origin[2];
        vector<int> domains;
        it->GetElementsList(normal, tmpD, domains);
        rv->GetDataRequest()->GetRestriction()->RestrictDomains(domains);

        //
        // Tell the file format reader that we will be slicing,
        // in case it can limit its reads to only the domains/elements that
        // intersect the plane.
        //
        avtPlaneSelection *sel = new avtPlaneSelection;
        sel->SetNormal(normal);
        sel->SetOrigin(origin);
        rv->GetDataRequest()->AddDataSelection(sel);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSliceFilter::PreExecute
//
//  Purpose:
//      Gets the point from the dynamically resolved point attribute and sets
//      it with the plane.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon May  5 14:37:08 PDT 2003
//    Changed the way "origin" works.
//
//    Hank Childs, Tue Jun 17 08:54:31 PDT 2003
//    Made origin be determined in this routine since it may require parallel
//    execution.
//
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
//    Hank Childs, Thu Nov 20 13:58:50 PST 2008
//    Get the spatial extents of the whole mesh and use that to set the
//    plane orientation.  Previously, we were changing the plane orientation
//    on a per domain basis, which could lead to inappropriate swaps of
//    orientation (in some cases), which meant that if a plane was on a domain
//    boundary, it would get contributions from both domains.
//
// ****************************************************************************

void
avtSliceFilter::PreExecute(void)
{
    avtPluginDataTreeIterator::PreExecute();

    double ox = 0;
    double oy = 0;
    double oz = 0;
    GetOrigin(ox, oy, oz);

    cachedOrigin[0] = ox;
    cachedOrigin[1] = oy;
    cachedOrigin[2] = oz;

    slicer->SetOrigin(ox, oy, oz);
    slicer->SetNormal(cachedNormal[0], cachedNormal[1], cachedNormal[2] );

    // figure out D in the plane equation
    D = cachedNormal[0]*ox + cachedNormal[1]*oy + cachedNormal[2]*oz;

    if (atts.GetProject2d())
        SetUpProjection();

    double bounds[6];
    GetSpatialExtents(bounds);
    SetPlaneOrientation(bounds);
}


// ****************************************************************************
//  Method: avtSliceFilter::SetUpProjection
//
//  Purpose:
//      Sets up the projection matrix.
//
//  Programmer: Hank Childs (taken from old PreExecute code)
//  Creation:   June 17, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul  8 20:27:53 PDT 2003 
//    Changed cross-products to conform to right-handed rule.
//    Removed origin from ftcf to prevent unintentional translation of the
//    slice.  Modified the calculation of invTransform to conform to other
//    changes (with help from Jeremy).
// 
//    Kathleen Bonnell, Wed Jun  2 09:11:01 PDT 2004 
//    Added origTrans. 
//
//    Hank Childs, Tue Jul 20 14:21:06 PDT 2004
//    Make sure that basis vectors are unit vectors -- crossing two
//    non-orthogonal vectors does not yield a unit vector.
//
//    Hank Childs, Fri Aug 13 09:07:31 PDT 2004
//    Only use 4-tuples with VTK's multiply point method to prevent UMRs.
//
//    Jeremy Meredith, Thu Aug 16 12:17:28 EDT 2007
//    Rewrote all the matrix stuff.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
//    Hank Childs, Wed Nov 14 16:10:07 PST 2007
//    Make sure that projection really projects to Z=0.  Without the
//    final projection to Z=0 added in this change, floating point 
//    precision problems in a matrix inversion can lead to projections 
//    that didn't actually go to 0.
//
//    Brad Whitlock, Wed Jan 23 15:49:33 PST 2008
//    Use a different, derived transform type and set whether it will 
//    transform its vectors based on doTransformVectors.
//
// ****************************************************************************

void
avtSliceFilter::SetUpProjection(void)
{
    double nx = cachedNormal[0];
    double ny = cachedNormal[1];
    double nz = cachedNormal[2];

    double ox = cachedOrigin[0];
    double oy = cachedOrigin[1];
    double oz = cachedOrigin[2];

    double ux = atts.GetUpAxis()[0];
    double uy = atts.GetUpAxis()[1];
    double uz = atts.GetUpAxis()[2];

    //
    // Make sure the up axis is valid. 
    //
    if (ux == 0. && uy == 0. && uz == 0.)
    {
        EXCEPTION1(BadVectorException, "Up Axis");
        return;
    }

    //
    // Make sure the up axis and normal are not equal
    //
    if (nx==ux && ny==uy && nz==uz)
    {
        // We could throw an exception here....
        // ...but for now I'll just correct the error instead.
        if (ux==0 && uy==0 && uz==1)
        {
            ux=1;  uy=0;  uz=0;
        }
        else
        {
            ux=0;  uy=0;  uz=1;
        }
    }

    double origin[4] = {ox,oy,oz,1};
    double normal[3] = {nx,ny,nz};
    double upaxis[3] = {ux,uy,uz};

    vtkMath::Normalize(normal);
    vtkMath::Normalize(upaxis);

    //
    // The normal and up vectors form two thirds of a basis, take their
    // cross product to find the third element of the basis.
    //
    double  third[3];
    vtkMath::Cross(upaxis, normal, third); // right-handed
    vtkMath::Normalize(third);  // if normal is not "orthogonal" to "third".

    // Make sure the up axis is orthogonal to third and normal
    vtkMath::Cross(normal, third, upaxis); // right-handed
    vtkMath::Normalize(upaxis); // probably not necessary since "normal" and
                                // "third" are orthogonal

    //
    // It's easier to create the matrix going from the projected system
    // to the original, then inverting it to find the matrix that
    // actually performs the projection.  Note VTK's matrix convention
    // is transposed from what we're used to.
    //
    vtkMatrix4x4 *xformToOriginalSpace = vtkMatrix4x4::New();
    xformToOriginalSpace->SetElement(0, 0, third[0]);
    xformToOriginalSpace->SetElement(1, 0, third[1]);
    xformToOriginalSpace->SetElement(2, 0, third[2]);
    xformToOriginalSpace->SetElement(3, 0, 0.);
    xformToOriginalSpace->SetElement(0, 1, upaxis[0]);
    xformToOriginalSpace->SetElement(1, 1, upaxis[1]);
    xformToOriginalSpace->SetElement(2, 1, upaxis[2]);
    xformToOriginalSpace->SetElement(3, 1, 0.);
    xformToOriginalSpace->SetElement(0, 2, normal[0]);
    xformToOriginalSpace->SetElement(1, 2, normal[1]);
    xformToOriginalSpace->SetElement(2, 2, normal[2]);
    xformToOriginalSpace->SetElement(3, 2, 0.);
    xformToOriginalSpace->SetElement(0, 3, origin[0]);
    xformToOriginalSpace->SetElement(1, 3, origin[1]);
    xformToOriginalSpace->SetElement(2, 3, origin[2]);
    xformToOriginalSpace->SetElement(3, 3, 1.);

    // And now invert to get the needed projection matrix.
    vtkMatrix4x4 *xformToXYPlane = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(xformToOriginalSpace, xformToXYPlane);

    // Floating point precision can cause the projection of really big data 
    // sets (in terms of spatial extents) to have Z-values that are not 0, and
    // even be outside the near and far planes.  So add an additional matrix to
    // project it down to Z=0.
    vtkMatrix4x4 *reallyProject = vtkMatrix4x4::New();
    reallyProject->Identity();
    reallyProject->SetElement(2, 2, 0.);
    vtkMatrix4x4 *realXformToXYPlane = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(reallyProject,xformToXYPlane,realXformToXYPlane);

    // Set the projection matrix for the transform.
    vtkVisItMatrixToLinearTransform *mtlt = vtkVisItMatrixToLinearTransform::New();
    mtlt->SetInput(realXformToXYPlane);
    mtlt->SetDoTransformVectors(doTransformVectors);
    transform->SetTransform(mtlt);
    mtlt->Delete();

    // Save the original and inverse matrix.  We probably could have
    // created these in-place and used them instead of the temporary
    // ones, but this makes the memory management a little clearer.
    invTrans->DeepCopy(xformToOriginalSpace);
    origTrans->DeepCopy(realXformToXYPlane);

    // Free the temporary matrices
    xformToOriginalSpace->Delete();
    xformToXYPlane->Delete();
    realXformToXYPlane->Delete();
    reallyProject->Delete();
}

// ****************************************************************************
//  Method:  avtSliceFilter::GetNormal
//
//  Purpose:
//    Extract the normal from the attributes as a 3-tuple.
//
//  Arguments:
//    nx,ny,nz   the normal        (n)
//
//  Programmer:  Dave Pugmire
//  Creation:    Oct 22, 2007
//
// ****************************************************************************

void
avtSliceFilter::GetNormal(double &nx, double &ny, double &nz)
{
    // Compute the normal if this is a Theta/Phi axis type.
    if ( atts.GetAxisType() == SliceAttributes::ThetaPhi )
    {
        double theta = (atts.GetTheta() - 90) * 0.017453292519943295;
        double phi = (atts.GetPhi() - 90) * 0.017453292519943295;
        nx = cos(theta)*sin(phi);
        ny = sin(theta)*sin(phi);
        nz = cos(phi);
    }
    else
    {
        nx = atts.GetNormal()[0];
        ny = atts.GetNormal()[1];
        nz = atts.GetNormal()[2];
    }
}


// ****************************************************************************
//  Method:  avtSliceFilter::GetOrigin
//
//  Purpose:
//    Extract the origin from the attributes as a 3-tuple.
//
//  Arguments:
//    ox,oy,oz   the origin        (o)
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  5, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Jun 17 09:07:14 PDT 2003 
//    Made the routine be a method with avtSliceFilter.  Added cases for
//    slicing by zone, node, etc.
//
//    Kathleen Bonnell, Wed Jun 16 14:01:07 PDT 2004 
//    Made SliceByZone use a db query to get the zone center. 
//    
//    Hank Childs, Thu Jun 17 15:08:24 PDT 2004
//    Improve error message when zone or node cannot be located.
//
//    Eric Brugger, Tue Jan  4 09:11:51 PST 2005
//    Made SliceByNode use avtOriginatingSource::QueryCoord instead of
//    avtDatasetExaminer::FindNode to match the SliceByZone code so that
//    this routine succeeds more often.
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added meshName specified in atts to QueryCoords call.  Added more to
//    warning message.
//
//    Mark C. Miller, Tue Mar 27 08:39:07 PDT 2007
//    Added support for nodeOrigin offsets in node numbers
//
//    Hank Childs, Wed Sep 12 09:13:20 PDT 2007
//    Choose origin more carefully since the project code now depends more
//    heavily on it.
//    
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead. Also consider axis type
//    of 'ThetaPhi'.
//
// ****************************************************************************

void
avtSliceFilter::GetOrigin(double &ox, double &oy, double &oz)
{
    double nx = cachedNormal[0];
    double ny = cachedNormal[1];
    double nz = cachedNormal[2];
    double nl = sqrt(nx*nx + ny*ny + nz*nz);

    // We want to make sure for orthogonal slices that "intercept" is still
    // meaningful even when the normal is pointing in the negative direction
    if (nx+ny+nz < 0 && (atts.GetAxisType() != SliceAttributes::Arbitrary ||
                         atts.GetAxisType() != SliceAttributes::ThetaPhi) )
    {
        nl *= -1;
    }

    ox = 0;
    oy = 0;
    oz = 0;

    switch (atts.GetOriginType())
    {
      case SliceAttributes::Point:
      {
          double *p = atts.GetOriginPoint();
          ox = p[0];
          oy = p[1];
          oz = p[2];
          break;
      }
      case SliceAttributes::Intercept:
      {
          if (nl != 0)
          {
              double d = atts.GetOriginIntercept();
              ox = nx * d / nl;
              oy = ny * d / nl;
              oz = nz * d / nl;
          }
          break;
      }
      case SliceAttributes::Percent:
      {
          double bounds[6];
          double percent = atts.GetOriginPercent() / 100.;
          GetSpatialExtents(bounds);
          ox = (bounds[1] - bounds[0])*percent + bounds[0];
          oy = (bounds[3] - bounds[2])*percent + bounds[2];
          oz = (bounds[5] - bounds[4])*percent + bounds[4];
          break;
      }
      case SliceAttributes::Zone:
      {
          avtOriginatingSource *src = GetInput()->GetOriginatingSource();
          int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
          int cellOrigin  = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
          int domain = atts.GetOriginZoneDomain();
          domain -= blockOrigin;
          int zone = atts.GetOriginZone();
          zone -= cellOrigin;
          double point[3];
          point[0] = FLT_MAX;
          point[1] = FLT_MAX;
          point[2] = FLT_MAX;
          string var = src->GetFullDataRequest()->GetVariable();
          int    ts  = src->GetFullDataRequest()->GetTimestep();
          bool success = src->QueryCoords(var, domain, zone, ts, point, true,
                         false, atts.GetMeshName().c_str());

          //
          //  All processors are participating in the same query, so should
          //  return the same results.  In case they don't (e.g. one proc
          //  failed for some reason), Unify the results.
          //
          double buff[6];
          if (success)
          {
              buff[0] = point[0];
              buff[2] = point[1];
              buff[4] = point[2];
          }
          else
          {
              buff[0] = DBL_MAX;
              buff[2] = DBL_MAX;
              buff[4] = DBL_MAX;
          }

          // This buffer is pretty much set up as a hack that uses existing
          // functions to unify minimum and maximum values -- we will only use
          // the minimum part as a way to broadcast a message.
          UnifyMinMax(buff, 6);

          if (buff[0] == DBL_MAX)
          {
              point[0] = 0.;
              point[1] = 0.;
              point[2] = 0.;
              char warning[1024];
              sprintf(warning, "Was not able to locate domain %d, zone %d. "
                               " They may not be defined on this mesh. "
                               " Using point (0., 0., 0.) instead.", 
                                          domain+blockOrigin, zone+cellOrigin);
              avtCallback::IssueWarning(warning);
          }
          else
          {
              point[0] = buff[0];
              point[1] = buff[2];
              point[2] = buff[4];
          }

          ox = point[0];
          oy = point[1];
          oz = point[2];
          break;
      }        
      case SliceAttributes::Node:
      {
          avtOriginatingSource *src = GetInput()->GetOriginatingSource();
          int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
          int nodeOrigin  = GetInput()->GetInfo().GetAttributes().GetNodeOrigin();
          int domain = atts.GetOriginNodeDomain();
          domain -= blockOrigin;
          int node = atts.GetOriginNode();
          node -= nodeOrigin;
          double point[3];
          point[0] = DBL_MAX;
          point[1] = DBL_MAX;
          point[2] = DBL_MAX;
          string var = src->GetFullDataRequest()->GetVariable();
          int    ts  = src->GetFullDataRequest()->GetTimestep();
          bool success = src->QueryCoords(var, domain, node, ts, point, false,
                         false, atts.GetMeshName().c_str());

          double buff[6];
          if (success)
          {
              buff[0] = point[0];
              buff[2] = point[1];
              buff[4] = point[2];
          }
          else
          {
              buff[0] = DBL_MAX;
              buff[2] = DBL_MAX;
              buff[4] = DBL_MAX;
          }

          // This buffer is pretty much set up as a hack that uses existing
          // functions to unify minimum and maximum values -- we will only use
          // the minimum part as a way to broadcast a message.
          UnifyMinMax(buff, 6);

          if (buff[0] == DBL_MAX)
          {
              point[0] = 0.;
              point[1] = 0.;
              point[2] = 0.;
              char warning[1024];
              sprintf(warning, "Was not able to locate domain %d, node %d. "
                               " They may not be defined on this mesh. " 
                               " Using point (0., 0., 0.) instead.",
                                                    domain+blockOrigin,
                                                    node+nodeOrigin);
              avtCallback::IssueWarning(warning);
          }
          else
          {
              point[0] = buff[0];
              point[1] = buff[2];
              point[2] = buff[4];
          }

          ox = point[0];
          oy = point[1];
          oz = point[2];
          break;
      }
    }

    // If we are doing a projection to 2D, then we want the units to make
    // sense.  The project code will project the origin of the plane to
    // be at (0,0).  If the origin of the plane is (X0,Y0,Z0), and we are 
    // slicing by Y=Y0, then the project code would place X0 and X=0 and
    // Z0 at Y=0.  This is confusing.  Instead, we would like X0 to be at
    // X=X0 and Z0 to be at Y=Z0.  We can do this by adjusting the origin.
    // If we have an axis-aligned slice, then we can set some components of
    // the origin to 0 and it will project like the user expects.
    if (atts.GetOriginType() != SliceAttributes::Point)
    {
        if (nx == 0.)
            ox = 0.;
        if (ny == 0.)
            oy = 0.;
        if (nz == 0.)
            oz = 0.;
    }
}


// ****************************************************************************
//  Method: avtSliceFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the slicer.
//
//  Arguments:
//      in_ds      The input dataset.
//      domain     The domain number.
//      <unused>   The label.
//
//  Notes:
//     In the context of selection by the database it may seem like we should
//     bypass the Slice operator.  We do *NOT* do this.  The reason is 
//     that the database still serves up a slab of zones (a 3D mesh) 
//     from which a 2D slice needs to be computed.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//    Jeremy Meredith, Thu Sep 28 12:45:16 PDT 2000
//    Made this create a new vtk dataset.
//
//    Hank Childs, Fri Oct 27 10:23:52 PDT 2000
//    Added argument for domain number to match inherited interface.
//
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Folded functionality of avtProjectFilter into this class.
//    Made attributes be stored as an SliceAttributes class.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Renamed method from ExecuteDomain to ExecuteData 
//
//    Hank Childs, Tue Oct 16 08:00:36 PDT 2001
//    Do not slice if it cannot intersect the dataset.
//
//    Hank Childs, Fri Oct 19 10:53:00 PDT 2001
//    Return NULL if there is no intersection.
//
//    Hank Childs, Wed Dec  5 17:21:53 PST 2001 
//    Do not let our interval tree do parallel communication.
//
//    Hank Childs, Mon Sep 16 17:34:06 PDT 2002
//    Clean up memory leak.
//
//    Hank Childs, Mon Dec  9 14:52:50 PST 2002
//    If a slice intersects the domain on the boundary, we may get no data
//    under some circumstances.  Reverse the direction of the normal in this
//    case.
//
//    Hank Childs, Mon Jun  9 09:20:43 PDT 2003
//    Use the new vtkSlicer class.
//
//    Mark C. Miller, Tue Sep 28 19:32:50 PDT 2004
//    Added comment regarding not by-passing this operator even if
//    data selection is applied
//
//    Kathleen Bonnell, Wed May 17 10:28:40 PDT 2006 
//    VTK filters no longer have a SetOutput method, Use SetOuputData from
//    the filter's executive instead.
//
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
//    Hank Childs, Fri Dec 29 15:02:34 PST 2006
//    Added support for rectilinear to rectilinear slicing.  Also moved
//    some logic into its own subroutine.
//
//    Hank Childs, Thu Nov 20 14:02:34 PST 2008
//    No longer use the per-domain bounds to set the plane orientation.
//
// ****************************************************************************

vtkDataSet *
avtSliceFilter::ExecuteData(vtkDataSet *in_ds, int domain, std::string)
{
    if (!CanIntersectPlane(in_ds))
    {
        debug5 << "Not slicing domain " << domain
               << ", it does not intersect the plane." << endl;
        return NULL;
    }

    bool haveExecuted = false;

    vtkDataSet *out_ds = NULL;
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rg = (vtkRectilinearGrid *) in_ds;

        if (OutputCanBeRectilinear(rg))
        {
            // This method returns a data set that must be freed.  That will
            // be done in the logic at the bottom of the routine.
            out_ds = RectilinearToRectilinearSlice(rg);
            haveExecuted = true;
        }
    }
    if (!haveExecuted)
    {
        vtkPolyData *pd = vtkPolyData::New();
    
        if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
            CalculateRectilinearCells((vtkRectilinearGrid *) in_ds);
        }
        else
        {
            slicer->SetCellList(NULL, 0);
        }
        slicer->SetInput(in_ds);
        if (atts.GetProject2d())
        {
            transform->SetInput(slicer->GetOutput());
            transform->GetExecutive()->SetOutputData(0, pd);
    
            //
            // Update will check the modifed time and call Execute.  We have no
            // direct hooks into the Execute method.
            //
            transform->Update();
        }
        else
        {
            slicer->SetOutput(pd);
    
            //
            // Update will check the modifed time and call Execute.  We have no
            // direct hooks into the Execute method.
            //
            slicer->Update();
        }
        out_ds = pd;
    }

    vtkDataSet *rv = out_ds;
    if (out_ds != NULL && out_ds->GetNumberOfCells() == 0)
    {
        rv = NULL;
    }
    
    ManageMemory(rv);
    if (out_ds != NULL)
        out_ds->Delete();
    
    return rv;
}


// ****************************************************************************
//  Method: avtSliceFilter::CanIntersectPlane
//
//  Purpose:
//     See if the slice plane even intersects the data set. 
//     
//  Programmer: Hank Childs
//  Creation:   December 29, 2006
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
// ****************************************************************************

bool
avtSliceFilter::CanIntersectPlane(vtkDataSet *in_ds)
{
    double bounds[6];
    in_ds->GetBounds(bounds);
    double normal[3];
    normal[0] = cachedNormal[0];
    normal[1] = cachedNormal[1];
    normal[2] = cachedNormal[2];
    avtIntervalTree tree(1, 3);
    tree.AddElement(0, bounds);
    tree.Calculate(true);
    vector<int> domains;
    tree.GetElementsList(normal, D, domains);
    if (domains.size() <= 0)
        return false;

    return true;
}


// ****************************************************************************
//  Method: avtSliceFilter::OutputCanBeRectilinear
//
//  Purpose:
//      Determines if we are taking an orthogonal slice, allowing the output
//      of the slice to still be rectilinear.
//      Determine if a rectilinear grid is valid to slice.  (It is not
//      if it is 2D.)  By "valid", we mean acceptable to the optimized
//      routine "RectilinearToRectilinearSlice"
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2006
//
// Modifications:
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
// ****************************************************************************

bool
avtSliceFilter::OutputCanBeRectilinear(vtkRectilinearGrid *rg)
{
    const double *normal = cachedNormal;
    if (! ((normal[0] != 0. && normal[1] == 0. && normal[2] == 0.) || 
           (normal[0] == 0. && normal[1] != 0. && normal[2] == 0.) || 
           (normal[0] == 0. && normal[1] == 0. && normal[2] != 0.)) )
        return false;
        
    if (atts.GetProject2d())
    {
        const double *up     = atts.GetUpAxis();
        if (normal[0] == +1.)
            if (up[0] != 0. || up[1] != 0. || up[2] != 1.)
                return false;
        if (normal[0] == -1.)
            if (up[0] != 0. || up[1] != 1. || up[2] != 0.)
                return false;
        if (normal[1] == +1.)
            if (up[0] != 1. || up[1] != 0. || up[2] != 0.)
                return false;
        if (normal[1] == -1.)
            if (up[0] != 0. || up[1] != 0. || up[2] != 1.)
                return false;
        if (normal[2] == +1.)
            if (up[0] != 0. || up[1] != 1. || up[2] != 0.)
                return false;
        if (normal[2] == -1.)
            if (up[0] != 1. || up[1] != 0. || up[2] != 0.)
                return false;
    }

    int pt_dims[3];
    rg->GetDimensions(pt_dims);
    if (pt_dims[0] <= 1 || pt_dims[1] <= 1 || pt_dims[2] <= 1)
        return false;

    return true;
}


// ****************************************************************************
//  Method: avtSliceFilter::RectilinearToRectilinearSlice
//
//  Purpose:
//      Perform an orthogonal slice of a rectilinear grid, to produce a 
//      rectilinear grid.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2006
//
//  Modifications:
//    Gunther H. Weber, Wed May 23 17:41:28 PDT 2007
//    Added Hank's bug fix for copying field data.
//
//    Hank Childs, Wed Sep 12 09:33:50 PDT 2007
//    Change project behavior to be consistent with Jeremy's changes.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
//    Jeremy Meredith & Hank Childs, Sun Nov  1 15:15:40 PST 2009
//    Transform the vector data when projecting to 2D.
//
// ****************************************************************************

vtkRectilinearGrid *
avtSliceFilter::RectilinearToRectilinearSlice(vtkRectilinearGrid *rg)
{
    int  i, j;
    
    const double *up     = atts.GetUpAxis();
    const double *normal = cachedNormal;
    bool  project        = atts.GetProject2d();

    int pt_dims[3];
    rg->GetDimensions(pt_dims);

    // Make the coordinates easily accessible.
    int           nCells = rg->GetNumberOfCells();
    double  *X = new double[pt_dims[0]];
    for (i = 0 ; i < pt_dims[0] ; i++)
        X[i] = rg->GetXCoordinates()->GetTuple1(i);
    double  *Y = new double[pt_dims[1]];
    for (i = 0 ; i < pt_dims[1] ; i++)
        Y[i] = rg->GetYCoordinates()->GetTuple1(i);
    double  *Z = new double[pt_dims[2]];
    for (i = 0 ; i < pt_dims[2] ; i++)
        Z[i] = rg->GetZCoordinates()->GetTuple1(i);

    vtkDataArray *flat_dim = vtkDataArray::CreateDataArray(
                                         rg->GetXCoordinates()->GetDataType());
    flat_dim->SetNumberOfTuples(1);
    if (project)
        flat_dim->SetTuple1(0, 0.);
    else
    {
        for (i = 0 ; i < 3 ; i++)
            if (normal[i] != 0.)
                flat_dim->SetTuple1(0, cachedOrigin[i]);
    }

    vtkRectilinearGrid *output = vtkRectilinearGrid::New();
    vtkCellData *newCD = output->GetCellData();
    vtkCellData *oldCD = rg->GetCellData();
    vtkPointData *newPD = output->GetPointData();
    vtkPointData *oldPD = rg->GetPointData();

    vtkDataArray *oldX = rg->GetXCoordinates();
    vtkDataArray *oldY = rg->GetYCoordinates();
    vtkDataArray *oldZ = rg->GetZCoordinates();
    vtkDataArray *transX = oldX;
    vtkDataArray *transY = oldY;
    vtkDataArray *transZ = oldZ;
    if (atts.GetOriginType() == SliceAttributes::Point)
    {
        int ntups = 0;

        transX = vtkDataArray::CreateDataArray(oldX->GetDataType());
        ntups = oldX->GetNumberOfTuples();
        transX->SetNumberOfTuples(ntups);
        for (j = 0 ; j < ntups ; j++)
            transX->SetTuple1(j, oldX->GetTuple1(j)-cachedOrigin[0]);

        transY = vtkDataArray::CreateDataArray(oldY->GetDataType());
        ntups = oldY->GetNumberOfTuples();
        transY->SetNumberOfTuples(ntups);
        for (j = 0 ; j < ntups ; j++)
            transY->SetTuple1(j, oldY->GetTuple1(j)-cachedOrigin[1]);

        transZ = vtkDataArray::CreateDataArray(oldZ->GetDataType());
        ntups = oldZ->GetNumberOfTuples();
        transZ->SetNumberOfTuples(ntups);
        for (j = 0 ; j < ntups ; j++)
            transZ->SetTuple1(j, oldZ->GetTuple1(j)-cachedOrigin[2]);
    }

    if (normal[0] != 0.)
    {
        //
        // Determine where the origin intersects our slice.
        //
        int iOffset = -1;
        float percent = 0.;
        for (i = 0 ; i < pt_dims[0]-1 ; i++)
        {
            if (X[i] <= cachedOrigin[0] && cachedOrigin[0] <= X[i+1])
            {
                iOffset = i;
                if (X[i] != X[i+1])
                    percent = ((cachedOrigin[0] - X[i]) / (X[i+1] - X[i]));
                break;
            }
        }
        if (iOffset == -1)
        {
            // return with grid not populated / empty grid.
            output->Delete();
            flat_dim->Delete();
            delete [] X; delete [] Y; delete [] Z;
            return NULL;
        }

        //
        // Now create a new rectilinear grid along the plane.
        //
        int new_dims[3];
        bool yVariesFastest = true;
        if (project)
        {
            output->SetZCoordinates(flat_dim);
            new_dims[2] = 1;
            if (up[0] == 0. && up[1] == 1. && up[2] == 0.)
            {
                yVariesFastest = false;
                output->SetXCoordinates(transZ);
                output->SetYCoordinates(transY);
                new_dims[0] = pt_dims[2];
                new_dims[1] = pt_dims[1];
            }
            else if (up[0] == 0. && up[1] == 0. && up[2] == 1.)
            {
                yVariesFastest = true;
                output->SetXCoordinates(transY);
                output->SetYCoordinates(transZ);
                new_dims[0] = pt_dims[1];
                new_dims[1] = pt_dims[2];
            }
            else
                EXCEPTION0(ImproperUseException);
        }
        else
        {
            yVariesFastest = true;
            output->SetXCoordinates(flat_dim);
            output->SetYCoordinates(oldY);
            output->SetZCoordinates(oldZ);
            new_dims[0] = 1;
            new_dims[1] = pt_dims[1];
            new_dims[2] = pt_dims[2];
        }

        output->SetDimensions(new_dims);
        int nnewpts = new_dims[0]*new_dims[1]*new_dims[2];
        int nnewcells = 1;
        nnewcells *= (new_dims[0] > 1 ? new_dims[0]-1 : 1);
        nnewcells *= (new_dims[1] > 1 ? new_dims[1]-1 : 1);
        nnewcells *= (new_dims[2] > 1 ? new_dims[2]-1 : 1);

        newCD->CopyAllocate(oldCD, nnewcells);
        for (i = 0 ; i < pt_dims[1]-1 ; i++)
            for (j = 0 ; j < pt_dims[2]-1 ; j++)
            {
                int new_index;
                if (yVariesFastest)
                    new_index = j*(pt_dims[1]-1) + i;
                else
                    new_index = i*(pt_dims[2]-1) + j;
                int old_index = j*(pt_dims[1]-1)*(pt_dims[0]-1)
                              + i*(pt_dims[0]-1) + iOffset;
                newCD->CopyData(oldCD, old_index, new_index);
            }
            
        newPD->CopyAllocate(oldPD, nnewcells);
        for (i = 0 ; i < pt_dims[1] ; i++)
            for (j = 0 ; j < pt_dims[2] ; j++)
            {
                int new_index;
                if (yVariesFastest)
                    new_index = j*(pt_dims[1]) + i;
                else
                    new_index = i*(pt_dims[2]) + j;
                int old_index1 = j*(pt_dims[1])*(pt_dims[0]) 
                               + i*(pt_dims[0]) + iOffset;
                int old_index2 = j*(pt_dims[1])*(pt_dims[0]) 
                               + i*(pt_dims[0]) + iOffset+1;
                newPD->InterpolateEdge(oldPD, new_index, 
                                       old_index1, old_index2, percent);
            }
    }
    else if (normal[1] != 0.)
    {
        //
        // Determine where the origin intersects our slice.
        //
        int jOffset = -1;
        float percent = 0.;
        for (i = 0 ; i < pt_dims[1]-1 ; i++)
        {
            if (Y[i] <= cachedOrigin[1] && cachedOrigin[1] <= Y[i+1])
            {
                jOffset = i;
                if (Y[i] != Y[i+1])
                    percent = ((cachedOrigin[1] - Y[i]) / (Y[i+1] - Y[i]));
                break;
            }
        }
        if (jOffset == -1)
        {
            // return with grid not populated / empty grid.
            output->Delete();
            flat_dim->Delete();
            delete [] X; delete [] Y; delete [] Z;
            return NULL;
        }

        //
        // Now create a new rectilinear grid along the plane.
        //
        int new_dims[3];
        bool xVariesFastest = true;
        if (project)
        {
            output->SetZCoordinates(flat_dim);
            new_dims[2] = 1;
            if (up[0] == 1. && up[1] == 0. && up[2] == 0.)
            {
                xVariesFastest = false;
                output->SetXCoordinates(transZ);
                output->SetYCoordinates(transX);
                new_dims[0] = pt_dims[2];
                new_dims[1] = pt_dims[0];
            }
            else if (up[0] == 0. && up[1] == 0. && up[2] == 1.)
            {
                xVariesFastest = true;
                output->SetXCoordinates(transX);
                output->SetYCoordinates(transZ);
                new_dims[0] = pt_dims[0];
                new_dims[1] = pt_dims[2];
            }
            else
                EXCEPTION0(ImproperUseException);
        }
        else
        {
            xVariesFastest = true;
            output->SetXCoordinates(oldX);
            output->SetYCoordinates(flat_dim);
            output->SetZCoordinates(oldZ);
            new_dims[0] = pt_dims[0];
            new_dims[1] = 1;
            new_dims[2] = pt_dims[2];
        }

        output->SetDimensions(new_dims);
        int nnewpts = new_dims[0]*new_dims[1]*new_dims[2];
        int nnewcells = 1;
        nnewcells *= (new_dims[0] > 1 ? new_dims[0]-1 : 1);
        nnewcells *= (new_dims[1] > 1 ? new_dims[1]-1 : 1);
        nnewcells *= (new_dims[2] > 1 ? new_dims[2]-1 : 1);

        newCD->CopyAllocate(oldCD, nnewcells);
        for (i = 0 ; i < pt_dims[0]-1 ; i++)
            for (j = 0 ; j < pt_dims[2]-1 ; j++)
            {
                int new_index;
                if (xVariesFastest)
                    new_index = j*(pt_dims[0]-1) + i;
                else
                    new_index = i*(pt_dims[2]-1) + j;
                int old_index = j*(pt_dims[1]-1)*(pt_dims[0]-1) 
                              + i + jOffset*(pt_dims[0]-1);
                newCD->CopyData(oldCD, old_index, new_index);
            }
            
        newPD->CopyAllocate(oldPD, nnewcells);
        for (i = 0 ; i < pt_dims[0] ; i++)
            for (j = 0 ; j < pt_dims[2] ; j++)
            {
                int new_index;
                if (xVariesFastest)
                    new_index = j*(pt_dims[0]) + i;
                else
                    new_index = i*(pt_dims[2]) + j;
                int old_index1 = j*(pt_dims[1])*(pt_dims[0]) 
                               + i + jOffset*pt_dims[0];
                int old_index2 = j*(pt_dims[1])*(pt_dims[0]) 
                               + i + (jOffset+1)*pt_dims[0];
                newPD->InterpolateEdge(oldPD, new_index, 
                                       old_index1, old_index2, percent);
            }
    }
    else if (normal[2] != 0.)
    {
        //
        // Determine where the origin intersects our slice.
        //
        int kOffset = -1;
        float percent = 0.;
        for (i = 0 ; i < pt_dims[2]-1 ; i++)
        {
            if (Z[i] <= cachedOrigin[2] && cachedOrigin[2] <= Z[i+1])
            {
                kOffset = i;
                if (Z[i] != Z[i+1])
                    percent = ((cachedOrigin[2] - Z[i]) / (Z[i+1] - Z[i]));
                break;
            }
        }
        if (kOffset == -1)
        {
            // return with grid not populated / empty grid.
            output->Delete();
            flat_dim->Delete();
            delete [] X; delete [] Y; delete [] Z;
            return NULL;
        }

        //
        // Now create a new rectilinear grid along the plane.
        //
        int new_dims[3];
        bool xVariesFastest = true;
        if (project)
        {
            output->SetZCoordinates(flat_dim);
            new_dims[2] = 1;
            if (up[0] == 1. && up[1] == 0. && up[2] == 0.)
            {
                xVariesFastest = false;
                output->SetXCoordinates(transY);
                output->SetYCoordinates(transX);
                new_dims[0] = pt_dims[1];
                new_dims[1] = pt_dims[0];
            }
            else if (up[0] == 0. && up[1] == 1. && up[2] == 0.)
            {
                xVariesFastest = true;
                output->SetXCoordinates(transX);
                output->SetYCoordinates(transY);
                new_dims[0] = pt_dims[0];
                new_dims[1] = pt_dims[1];
            }
            else
                EXCEPTION0(ImproperUseException);
        }
        else
        {
            xVariesFastest = true;
            output->SetXCoordinates(oldX);
            output->SetYCoordinates(oldY);
            output->SetZCoordinates(flat_dim);
            new_dims[0] = pt_dims[0];
            new_dims[1] = pt_dims[1];
            new_dims[2] = 1;
        }

        output->SetDimensions(new_dims);
        int nnewpts = new_dims[0]*new_dims[1]*new_dims[2];
        int nnewcells = 1;
        nnewcells *= (new_dims[0] > 1 ? new_dims[0]-1 : 1);
        nnewcells *= (new_dims[1] > 1 ? new_dims[1]-1 : 1);
        nnewcells *= (new_dims[2] > 1 ? new_dims[2]-1 : 1);

        newCD->CopyAllocate(oldCD, nnewcells);
        for (i = 0 ; i < pt_dims[0]-1 ; i++)
            for (j = 0 ; j < pt_dims[1]-1 ; j++)
            {
                int new_index;
                if (xVariesFastest)
                    new_index = j*(pt_dims[0]-1) + i;
                else
                    new_index = i*(pt_dims[1]-1) + j;
                int old_index = j*(pt_dims[0]-1) + i 
                              + kOffset*(pt_dims[0]-1)*(pt_dims[1]-1);
                newCD->CopyData(oldCD, old_index, new_index);
            }
            
        newPD->CopyAllocate(oldPD, nnewcells);
        for (i = 0 ; i < pt_dims[0] ; i++)
            for (j = 0 ; j < pt_dims[1] ; j++)
            {
                int new_index;
                if (xVariesFastest)
                    new_index = j*(pt_dims[0]) + i;
                else
                    new_index = i*(pt_dims[1]) + j;
                int old_index1 = j*(pt_dims[0]) + i 
                               + kOffset*(pt_dims[0])*(pt_dims[1]);
                int old_index2 = j*(pt_dims[0]) + i 
                               + (kOffset+1)*pt_dims[0]*pt_dims[1];
                newPD->InterpolateEdge(oldPD, new_index, 
                                       old_index1, old_index2, percent);
            }
    }

    //
    // Convert vectors to 2D if necessary
    //
    if (project)
    {
        int numCellArrays = output->GetCellData()->GetNumberOfArrays();
        int numPtArrays   = output->GetPointData()->GetNumberOfArrays();
        for (int a = numCellArrays+numPtArrays-1 ; a>=0 ; a--)
        {
            vtkDataSetAttributes *dsa = NULL;
            if ((a<numCellArrays))
                dsa = output->GetCellData();
            else
                dsa = output->GetPointData();
            int idx = ((a<numCellArrays) ? a : a-numCellArrays);
            vtkDataArray *arr = dsa->GetArray(idx);
            if (arr == NULL || arr->GetNumberOfComponents() != 3)
                continue;
            vtkDataArray *outArray = 
                          vtkDataArray::CreateDataArray(arr->GetDataType());
            outArray->SetName(arr->GetName());
            outArray->SetNumberOfComponents(3);
            int nTups = arr->GetNumberOfTuples();
            outArray->SetNumberOfTuples(nTups);
            double oldtup[3], newtup[3] = {0,0,0};
            for (int v = 0 ; v < nTups ; v++)
            {
                arr->GetTuple(v, oldtup);
                if      (normal[0] != 0    &&   up[1] == 1) // x slice, y up
                {
                    newtup[0] = oldtup[2];
                    newtup[1] = oldtup[1];
                }
                else if (normal[0] != 0    &&   up[2] == 1) // x slice, z up
                {
                    newtup[0] = oldtup[1];
                    newtup[1] = oldtup[2];
                }
                else if (normal[1] != 0    &&   up[0] == 1) // y slice, x up
                {
                    newtup[0] = oldtup[2];
                    newtup[1] = oldtup[0];
                }
                else if (normal[1] != 0    &&   up[2] == 1) // y slice, z up
                {
                    newtup[0] = oldtup[0];
                    newtup[1] = oldtup[2];
                }
                else if (normal[2] != 0    &&   up[0] == 1) // z slice, x up
                {
                    newtup[0] = oldtup[1];
                    newtup[1] = oldtup[0];
                }
                else if (normal[2] != 0    &&   up[1] == 1) // z slice, y up
                {
                    newtup[0] = oldtup[0];
                    newtup[1] = oldtup[1];
                }
                else
                {
                    EXCEPTION0(ImproperUseException);
                }
                outArray->SetTuple(v, newtup);
            }
            bool isActiveVector = (dsa->GetVectors() == arr);
            dsa->RemoveArray(arr->GetName());
            dsa->AddArray(outArray);
            outArray->Delete();
            if (isActiveVector)
               dsa->SetActiveVectors(outArray->GetName());
        }
    }
    delete [] X;
    delete [] Y;
    delete [] Z;
    flat_dim->Delete();

    if (atts.GetOriginType() == SliceAttributes::Point)
    {
        transX->Delete();
        transY->Delete();
        transZ->Delete();
    }

    output->GetFieldData()->ShallowCopy(rg->GetFieldData());
    return output;
}


// ****************************************************************************
//  Method: avtSliceFilter::ReleaseData
//
//  Purpose:
//      Releases the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Sep 16 17:39:49 PDT 2002
//    Clean up more bloat.
//
//    Hank Childs, Mon Jun  9 09:20:43 PDT 2003
//    Use the new vtkSlicer class.
//
//    Hank Childs, Thu Mar  3 17:41:30 PST 2005
//    If we make the output's NULL, we will have problems with DLB.
//
//    Hank Childs, Fri Mar 11 07:37:05 PST 2005
//    Fix non-problem size leak introduced with last fix.
//
//    Kathleen Bonnell, Wed May 17 10:28:40 PDT 2006 
//    VTK filters no longer have a SetOutput method, Use SetOuputData from the
//    filter's executive instead.  Use SetInputConnection to set a NULL input.
//
// ****************************************************************************

void
avtSliceFilter::ReleaseData(void)
{
    avtPluginDataTreeIterator::ReleaseData();

    slicer->SetInputConnection(0, NULL);
    vtkPolyData *p = vtkPolyData::New();
    slicer->GetExecutive()->SetOutputData(0, p);
    p->Delete();

    transform->SetInputConnection(0, NULL);
    p = vtkPolyData::New();
    transform->GetExecutive()->SetOutputData(0, p);
    p->Delete();

    if (celllist != NULL)
    {
        delete [] celllist;
        celllist = NULL;
    }
}


// ****************************************************************************
//  Method: avtSliceFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Changes to topological dimension of the output to be one less that the
//      input.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Kathleen Bonnell, Tue Mar 26 10:43:23 PST 2002 
//    Add call to validity.SetPointsWereTransformed. 
//
//    Hank Childs, Thu May 16 16:53:41 PDT 2002
//    Transform extents as well.
//
//    Sean Ahern, Fri May 17 16:43:35 PDT 2002
//    Corrected spelling of "cumulative".
//
//    Hank Childs, Tue Aug  6 11:07:14 PDT 2002
//    Tell the output that normals are not needed.
//
//    Hank Childs, Wed Jun 18 19:06:23 PDT 2003
//    Do not project the extents here, since the origin has not been
//    established yet.
//
//    Hank Childs, Wed Oct 15 21:50:27 PDT 2003
//    Re-arrange the labels when we are slicing orthogonally.
//
//    Hank Childs, Tue Mar 30 07:34:17 PST 2004
//    Do not decrease the topological dimension of points (slicing points still
//    yields points).
//
//    Hank Childs, Tue May  4 16:19:32 PDT 2004
//    Calculate normals if we are not projecting to 2D.
//
//    Brad Whitlock, Thu Jul 22 17:32:42 PST 2004
//    I added code to make sure that the units get switched the same way
//    as the labels. You can only notice it when X,Y,Z have different units.
//
//    Hank Childs, Tue Jun  7 14:11:20 PDT 2005
//    Allow normals filter to decide if we need normals; it should be able
//    to figure out if we have sliced and thus they are not needed.
//
//    Kathleen Bonnell, Thu Mar  2 14:26:06 PST 2006
//    Set ZonesSplit.
//
//    Hank Childs, Thu Oct  9 11:01:14 PDT 2008
//    Change the axis names for arbitrary slices.
//
// ****************************************************************************

void
avtSliceFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();
   
    if (inAtts.GetTopologicalDimension() >= 1)
        outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);

    outValidity.InvalidateZones();
    outValidity.ZonesSplit();

    if (atts.GetProject2d())
    {
        outAtts.SetSpatialDimension(2);
        outValidity.InvalidateSpatialMetaData();
        outValidity.SetPointsWereTransformed(true);
    }

    if (atts.GetProject2d())
    {
        const double *normal = cachedNormal;
        const double *up     = atts.GetUpAxis();
        if ((normal[0] != 0.) && (normal[1] == 0.) && (normal[2] == 0.))
        {
            if ((up[0] == 0.) && (up[1] != 0.) && (up[2] == 0.))
            {
                outAtts.SetXLabel(inAtts.GetZLabel());
                outAtts.SetZLabel(inAtts.GetXLabel());

                outAtts.SetXUnits(inAtts.GetZUnits());
                outAtts.SetZUnits(inAtts.GetXUnits());
            }
            else if ((up[0] == 0.) && (up[1] == 0.) && (up[2] != 0.))
            {
                outAtts.SetXLabel(inAtts.GetYLabel());
                outAtts.SetYLabel(inAtts.GetZLabel());
                outAtts.SetZLabel(inAtts.GetXLabel());

                outAtts.SetXUnits(inAtts.GetYUnits());
                outAtts.SetYUnits(inAtts.GetZUnits());
                outAtts.SetZUnits(inAtts.GetXUnits());
            }
        }
        else if ((normal[0] == 0.) && (normal[1] != 0.) && (normal[2] == 0.))
        {
            if ((up[0] != 0.) && (up[1] == 0.) && (up[2] == 0.))
            {
                outAtts.SetYLabel(inAtts.GetXLabel());
                outAtts.SetXLabel(inAtts.GetZLabel());
                outAtts.SetZLabel(inAtts.GetYLabel());

                outAtts.SetYUnits(inAtts.GetXUnits());
                outAtts.SetXUnits(inAtts.GetZUnits());
                outAtts.SetZUnits(inAtts.GetYUnits());
            }
            else if ((up[0] == 0.) && (up[1] == 0.) && (up[2] != 0.))
            {
                outAtts.SetYLabel(inAtts.GetZLabel());
                outAtts.SetZLabel(inAtts.GetYLabel());

                outAtts.SetYUnits(inAtts.GetZUnits());
                outAtts.SetZUnits(inAtts.GetYUnits());
            }
        }
        else if ((normal[0] == 0.) && (normal[1] == 0.) && (normal[2] != 0.))
        {
            if ((up[0] != 0.) && (up[1] == 0.) && (up[2] == 0.))
            {
                outAtts.SetXLabel(inAtts.GetYLabel());
                outAtts.SetYLabel(inAtts.GetXLabel());

                outAtts.SetXUnits(inAtts.GetYUnits());
                outAtts.SetYUnits(inAtts.GetXUnits());
            }
            else if ((up[0] == 0.) && (up[1] != 0.) && (up[2] == 0.))
            {
                // Pretty much a no-op
            }
        }
        else
        {
            double normal_mag = sqrt(normal[0]*normal[0] + 
                                    normal[1]*normal[1] + normal[2]*normal[2]);
            double nn[3];
            if (normal_mag > 0.)
            {
                nn[0] = normal[0] / normal_mag;
                nn[1] = normal[1] / normal_mag;
                nn[2] = normal[2] / normal_mag;
            }
            double up_mag = sqrt(up[0]*up[0] + up[1]*up[1] + up[2]*up[2]);
            double un[3];
            if (up_mag > 0.)
            {
                un[0] = up[0] / up_mag;
                un[1] = up[1] / up_mag;
                un[2] = up[2] / up_mag;
            }
            char ylabel[2048];
            SNPRINTF(ylabel, 2048, "(%.2f,%.2f,%.2f)-Axis", un[0],un[1],un[2]);
            outAtts.SetYLabel(ylabel);

            double cross[3];
            cross[0] = un[1]*nn[2] - un[2]*nn[1];
            cross[1] = un[2]*nn[0] - un[0]*nn[2];
            cross[2] = un[0]*nn[1] - un[1]*nn[0];
            double cross_mag = sqrt(cross[0]*cross[0] + cross[1]*cross[1] +
                                    cross[2]*cross[2]);
            if (cross_mag > 0.)
            {
                cross[0] /= cross_mag;
                cross[1] /= cross_mag;
                cross[2] /= cross_mag;
            }
            char xlabel[2048];
            SNPRINTF(xlabel, 2048, "(%.2f,%.2f,%.2f)-Axis", cross[0], cross[1], 
                                                            cross[2]);
            outAtts.SetXLabel(xlabel);
        }
    }
}


// ****************************************************************************
//  Function: ProjectExtentsCallback
//
//  Purpose:
//      A callback to project extents.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

static void
ProjectExtentsCallback(const double *in, double *out, void *args)
{
    avtSliceFilter *p = (avtSliceFilter *) args;
    p->ProjectExtents(in, out);
}


// ****************************************************************************
//  Method: avtSliceFilter::ProjectExtents
//
//  Purpose:
//      Projects extents for 3D to 2D based on a slice and projection matrix.
//
//  Arguments:
//      b       A buffer of extents.
//
//  Programmer: Hank Childs
//  Creation:   May 16, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Feb 28 07:13:28 PST 2003
//    Made a member function to avtSliceFilter.  Put in special logic for
//    slicing along faces of the extents.
//
//    Hank Childs, Mon Jun  9 09:20:43 PDT 2003
//    Use the new vtkSlicer class.
//
//    Hank Childs, Wed Jun 18 19:28:01 PDT 2003
//    Ensure that we don't affect the actual output.
//
//    Hank Childs, Mon May 24 16:11:41 PDT 2004
//    Do a better job of getting the extents if we slice a plane.
//
//    Hank Childs, Thu Jan 20 10:36:10 PST 2005
//    Added argument so this could be called through a callback.
//
//    Hank Childs, Fri Aug 19 09:00:32 PDT 2005
//    Cast the output of the transform filter to poly data.  This was necessary
//    because we changed the filter type from vtkTransformPolyDataFilter to
//    vtkTransformFilter to avoid '6471.
//
//    Kathleen Bonnell, Wed May 17 10:28:40 PDT 2006 
//    VTK filters no longer have a SetOutput method, Use SetOuputData from the
//    filter's executive instead.  
//
// ****************************************************************************

void
avtSliceFilter::ProjectExtents(const double *b_in, double *b_out)
{
    double b[6];
    b[0] = b_in[0];
    b[1] = b_in[1];
    b[2] = b_in[2];
    b[3] = b_in[3];
    b[4] = b_in[4];
    b[5] = b_in[5];

    //
    // Clean up leftovers from previous executions.
    //
    vtkPolyData *new_output = vtkPolyData::New();
    transform->GetExecutive()->SetOutputData(0, new_output);
    new_output->Delete();
    slicer->SetCellList(NULL, 0);

    //
    // It is possible that we are slicing a plane.  If so, put in a little
    // fudge factor to make sure that the rgrid we set up will actually
    // intersect the plane.
    //
    double x_dist = fabs(b[1] - b[0]);
    double y_dist = fabs(b[3] - b[2]);
    double z_dist = fabs(b[5] - b[4]);
    double max_dist = x_dist;
    max_dist = (y_dist > max_dist ? y_dist : max_dist);
    max_dist = (z_dist > max_dist ? z_dist : max_dist);
    if (b[0] == b[1])
    {
       b[0] -= 0.0001*max_dist;
       b[1] += 0.0001*max_dist;
    }
    if (b[2] == b[3])
    {
       b[2] -= 0.0001*max_dist;
       b[3] += 0.0001*max_dist;
    }
    if (b[4] == b[5])
    {
       b[4] -= 0.0001*max_dist;
       b[5] += 0.0001*max_dist;
    }

    SetPlaneOrientation(b);

    //
    // Set up a one cell-ed rectilinear grid based on the bounding box.
    //
    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(2);
    x->SetComponent(0, 0, b[0]);
    x->SetComponent(1, 0, b[1]);

    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfTuples(2);
    y->SetComponent(0, 0, b[2]);
    y->SetComponent(1, 0, b[3]);

    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfTuples(2);
    z->SetComponent(0, 0, b[4]);
    z->SetComponent(1, 0, b[5]);

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(2, 2, 2);
    rgrid->SetXCoordinates(x);
    rgrid->SetYCoordinates(y);
    rgrid->SetZCoordinates(z);

    //
    // Slice and project our bounding box to mimic what would happen to our
    // original dataset.
    //
    slicer->SetInput(rgrid);
    transform->SetInput(slicer->GetOutput());
    transform->Update();

    //
    // Now iterate through the resulting triangles and determine what the 
    // extents are.
    //
    vtkPolyData *pd = (vtkPolyData *) transform->GetOutput();
    float minmax[4] = { +FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX };
    for (int i = 0 ; i < pd->GetNumberOfCells() ; i++)
    {
        vtkCell *cell = pd->GetCell(i);
        double bounds[6];
        cell->GetBounds(bounds);
        minmax[0] = (minmax[0] < bounds[0] ? minmax[0] : bounds[0]);
        minmax[1] = (minmax[1] > bounds[1] ? minmax[1] : bounds[1]);
        minmax[2] = (minmax[2] < bounds[2] ? minmax[2] : bounds[2]);
        minmax[3] = (minmax[3] > bounds[3] ? minmax[3] : bounds[3]);
    }

    if (pd->GetNumberOfCells() > 0)
    {
        b[0] = minmax[0];
        b[1] = minmax[1];
        b[2] = minmax[2];
        b[3] = minmax[3];
    }
    else
    {
        b[0] = 0.;
        b[1] = 0.;
        b[2] = 0.;
        b[3] = 0.;
    }

    b_out[0] = b[0];
    b_out[1] = b[1];
    b_out[2] = b[2];
    b_out[3] = b[3];

    x->Delete();
    y->Delete();
    z->Delete();
    rgrid->Delete();
}


// ****************************************************************************
//  Method: avtSliceFilter::SetPlaneOrientation
//
//  Purpose:
//      The VTK slicing routines will sometimes not slice correctly when the
//      slice coincides with a face of a cell.  If this face is on the
//      boundary, we sometimes get no intersecting triangles.  To counteract
//      this, we can set the plane to be the plane with the direct opposite
//      normal.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Apr 13 09:48:57 PDT 2004
//    Make normal-flipping cases work for all normal orientations.
//
//    Kathleen Bonnell, Fri Apr 28 15:44:39 PDT 2006
//    Add an epsilon to check for origin close to bounds.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
// ****************************************************************************

void
avtSliceFilter::SetPlaneOrientation(double *b)
{
    double normal[3];
    normal[0] = cachedNormal[0];
    normal[1] = cachedNormal[1];
    normal[2] = cachedNormal[2];

    //
    // Because of the underlying implementation, we will run into cases where
    // there is no slice if it intersects the cell on the face.  Try to 
    // counter-act this.
    //
    double ox = cachedOrigin[0];
    double oy = cachedOrigin[1];
    double oz = cachedOrigin[2];

    double eps = 1e-16;
    if (normal[0] != 0. && normal[1] == 0. && normal[2] == 0.)
    {
        if ((normal[0] > 0.) && ((ox > b[0] - eps) && (ox < b[0] + eps)))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else if ((normal[0] < 0.) && ((ox > b[1] -eps) && (ox < b[1] + eps)))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else
            slicer->SetNormal(normal[0], normal[1], normal[2]);
    }
    else if (normal[0] == 0. && normal[1] != 0. && normal[2] == 0.)
    {
        if ((normal[1] > 0.) && ((oy > b[2] - eps) && (oy < b[2] + eps)))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else if ((normal[1] < 0.) && ((oy > b[3] - eps) && (oy < b[3] + eps)))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else
            slicer->SetNormal(normal[0], normal[1], normal[2]);
    }
    else if (normal[0] == 0. && normal[1] == 0. && normal[2] != 0.)
    {
        if ((normal[2] > 0.) && ((oz > b[4] - eps) && (oz < b[4] + eps)))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else if ((normal[2] < 0.) && ((oz > b[5] - eps) && (oz < b[5] + eps)))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else
            slicer->SetNormal(normal[0], normal[1], normal[2]);
    }
    else
    {
        // Just in case it got set backwards earlier.
        slicer->SetNormal(normal[0], normal[1], normal[2]);
    }
}


// ****************************************************************************
//  Method: avtSliceFilter::CalculateRectilinearCells
//
//  Purpose:
//      Calculates the cell list that intersects the slice.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Jun  9 09:20:43 PDT 2003
//    Use the new vtkSlicer class.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Normal is cached, so use that value instead.
//
// ****************************************************************************

void
avtSliceFilter::CalculateRectilinearCells(vtkRectilinearGrid *rgrid)
{
    int   handle = visitTimer->StartTimer();
    int   i;

    vtkDataArray *xc = rgrid->GetXCoordinates();
    int nx = xc->GetNumberOfTuples();
    float *x = new float[nx];
    for (i = 0 ; i < nx ; i++)
    {
        x[i] = xc->GetTuple1(i);
    }

    vtkDataArray *yc = rgrid->GetYCoordinates();
    int ny = yc->GetNumberOfTuples();
    float *y = new float[ny];
    for (i = 0 ; i < ny ; i++)
    {
        y[i] = yc->GetTuple1(i);
    }

    vtkDataArray *zc = rgrid->GetZCoordinates();
    int nz = zc->GetNumberOfTuples();
    float *z = new float[nz];
    for (i = 0 ; i < nz ; i++)
    {
        z[i] = zc->GetTuple1(i);
    }

    //
    // Come up with an appropriate upper bound for the number of cells.
    //
    if (celllist != NULL)
        delete [] celllist;
    int totalcells = nx*nx + ny*ny + nz*nz;
    celllist   = new int[totalcells];

    float plane[4];
    plane[0] = cachedNormal[0];
    plane[1] = cachedNormal[1];
    plane[2] = cachedNormal[2];
    plane[3] = D;

    int numcells = 0;
    FindCells(x, y, z, nx-1, ny-1, nz-1,celllist, numcells, plane, 0, 0, 0, 0,
              nx-1,ny-1,nz-1);
    debug5 << "The slice intersected " << numcells << " cells." << endl;

    slicer->SetCellList(celllist, numcells);

    delete [] x;
    delete [] y;
    delete [] z;

    visitTimer->StopTimer(handle, "Locating cells that intersect mesh");
}


// ****************************************************************************
//  Function: FindCells
//
//  Purpose:
//      Finds the cells from a rectilinear grid that intersect a cube.
//
//  Arguments:
//      x       A list of the x values.
//      y       A list of the y values.
//      z       A list of the z values.
//      nx      The number of x values.
//      ny      The number of y values.
//      nz      The number of z values.
//      clist   A place to store the cell list.
//      ncells  The number of cells in clist.
//      plane   The equation of a plane in (A,B,C,D).
//      dim     A hint as to what dimension to split the cube in.
//      ax      The actual x-coordinate (needed to calculate cell numbers).
//      ay      The actual y-coordinate (needed to calculate cell numbers).
//      az      The actual z-coordinate (needed to calculate cell numbers).
//      onx     The original number of values in x.
//      ony     The original number of values in y.
//      onz     The original number of values in z.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
// ****************************************************************************

void
FindCells(float *x, float *y, float *z, int nx, int ny, int nz, int *clist, 
          int &ncells, float *plane, int dim, int ax, int ay, int az, int onx,
          int ony, int onz)
{
    if (nx <= 0 || ny <= 0 || nz <= 0)
    {
        return;
    }

    float bounds[6];
    bounds[0] = x[0];
    bounds[1] = x[nx];
    bounds[2] = y[0];
    bounds[3] = y[ny];
    bounds[4] = z[0];
    bounds[5] = z[nz];
    if (!PlaneIntersectsCube(plane, bounds))
    {
        return;
    }

    if (nx == 1 && ny == 1 && nz == 1)
    {
        //
        // Here is the base case -- see if this one cell intersects the plane.
        //
        int cell = az*onx*ony + ay*onx + ax;
        clist[ncells] = cell;
        ncells++;  
    }
    else
    {
        //
        // Split the problem into smaller pieces (ie recurse).
        //
        if (dim == 0)
        {
            //
            // Split along x.
            //
            int split = nx/2;
            FindCells(x,y,z,split,ny,nz,clist,ncells,plane,1,ax,ay,az,onx,ony,
                      onz);
            FindCells(x+split,y,z,nx-split,ny,nz,clist,ncells,plane,1,
                      ax+split,ay,az, onx, ony, onz);
        }
        else if (dim == 1)
        {
            //
            // Split along y.
            //
            int split = ny/2;
            FindCells(x,y,z,nx,split,nz,clist,ncells,plane,1,ax,ay,az,
                      onx,ony,onz);
            FindCells(x,y+split,z,nx,ny-split,nz,clist,ncells,plane,2,
                      ax,ay+split,az, onx, ony, onz);
        }
        else if (dim == 2)
        {
            //
            // Split along z.
            //
            int split = nz/2;
            FindCells(x,y,z,nx,ny,split,clist,ncells,plane,1,ax,ay,az,
                      onx,ony,onz);
            FindCells(x,y,z+split,nx,ny,nz-split,clist,ncells,plane,0,
                      ax,ay,az+split, onx, ony, onz);
        }
    }
}


// ****************************************************************************
//  Function: PlaneIntersectsCube
//
//  Purpose:
//      Determines if a plane intersects a cube.
//
//  Arguments:
//      plane   The equation of a plane as (A,B,C,D).
//      bounds  The bounds of a cube as (minx,maxx,miny,maxy,minz,maxz).
//
//  Returns:    True if the plane intersects the cube, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
// ****************************************************************************

bool
PlaneIntersectsCube(float plane[4], float bounds[6])
{
    bool has_low_point  = false;
    bool has_high_point = false;
    for (int i = 0 ; i < 8 ; i++)
    {
        float x = (i&1 ? bounds[1] : bounds[0]);
        float y = (i&2 ? bounds[3] : bounds[2]);
        float z = (i&4 ? bounds[5] : bounds[4]);
        float val = plane[3] - plane[0]*x - plane[1]*y - plane[2]*z;

        if (val == 0.)  // If we are on the plane, intersect
            return true;

        if (val < 0)
            has_low_point = true;
        else
            has_high_point = true;

        if (has_low_point && has_high_point)
            return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtSliceFilter::PostExecute
//
//  Purpose:
//      This is called to set the inverse transformation matrix in the output.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003 
//
//  Modifications:
//
//    Hank Childs, Wed Jun 18 19:06:23 PDT 2003
//    Project the extents here, since the origin is now determined.
//
//    Kathleen Bonnell, Wed Jun  2 09:11:01 PDT 2004 
//    Added origTrans. 
//
//    Hank Childs, Thu Jan 20 10:44:24 PST 2005
//    Make use of new method in data attributes to transform spatial extents.
//
// ****************************************************************************

void
avtSliceFilter::PostExecute()
{
    if (atts.GetProject2d())
    {
        avtDataAttributes &inAtts     =  GetInput()->GetInfo().GetAttributes();
        avtDataAttributes &outAtts    = GetOutput()->GetInfo().GetAttributes();

        GetOutput()->GetInfo().GetAttributes().SetInvTransform((*invTrans)[0]);
        GetOutput()->GetInfo().GetAttributes().SetTransform((*origTrans)[0]);

        inAtts.TransformSpatialExtents(outAtts, ProjectExtentsCallback, this);
    }
}


