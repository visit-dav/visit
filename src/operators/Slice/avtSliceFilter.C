// ************************************************************************* //
//                             avtSliceFilter.C                              //
// ************************************************************************* //

#include <avtSliceFilter.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSlicer.h>
#include <vtkTransformPolyDataFilter.h>

#include <avtCallback.h>
#include <avtDataset.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtSpatialBoxSelection.h>
#include <avtTerminatingSource.h>

#include <BadVectorException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

using     std::vector;


static bool      PlaneIntersectsCube(float plane[4], float bounds[6]);
static void      FindCells(float *x, float *y, float *z, int nx, int ny, 
                           int nz, int *clist, int &ncells, float *plane, 
                           int dim, int ax, int ay, int az, int onx, int ony,
                           int onz);


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
// ****************************************************************************

avtSliceFilter::avtSliceFilter()
{
    slicer = vtkSlicer::New();
    transform = vtkTransformPolyDataFilter::New();
    celllist = NULL;
    invTrans = vtkMatrix4x4::New();
    origTrans = vtkMatrix4x4::New();
    cachedOrigin[0] = 0.;
    cachedOrigin[1] = 0.;
    cachedOrigin[2] = 0.;
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
// ****************************************************************************

void
avtSliceFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const SliceAttributes*)a;

    double nx = atts.GetNormal()[0];
    double ny = atts.GetNormal()[1];
    double nz = atts.GetNormal()[2];
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
//  Method: avtSliceFilter::PerformRestriction
//
//  Purpose:
//      Calculates the restriction on the meta-data and the plane 
//      equation of the cutter.
//
//  Arguments:
//      spec    The current pipeline specification.
//
//  Returns:    The new specification.
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
// ****************************************************************************

avtPipelineSpecification_p
avtSliceFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec);

    //
    // Pick returns wrong results (even with transform) when slice lies
    // along boundary between zones.  So always turn on zone, node
    // numbers.  WE MAY WANT TO REVERT BACK IN THE FUTURE IF A
    // BETTER WAY CAN BE FOUND FOR PICK TO RETURN CORRECT RESULTS.  
    rv->GetDataSpecification()->TurnZoneNumbersOn();
    rv->GetDataSpecification()->TurnNodeNumbersOn();


#if 0
    if (atts.GetProject2d() && rv->GetDataSpecification()->MayRequireZones())
    {
        rv->GetDataSpecification()->TurnZoneNumbersOn();
    }
    if (atts.GetProject2d() && rv->GetDataSpecification()->MayRequireNodes())
    {
        rv->GetDataSpecification()->TurnNodeNumbersOn();
    }

    if (atts.GetOriginType() == SliceAttributes::Zone)
    {
        rv->GetDataSpecification()->TurnZoneNumbersOn();
    }
    if (atts.GetOriginType() == SliceAttributes::Node)
    {
        rv->GetDataSpecification()->TurnNodeNumbersOn();
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
        float mins[3], maxs[3];
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
        rv->GetDataSpecification()->AddDataSelection(sel);
    }

    //
    // Get the interval tree.
    //
    avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
    if (it == NULL)
    {
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
        float normal[3]
             = {atts.GetNormal()[0], atts.GetNormal()[1], atts.GetNormal()[2]};
        double origin[3];
        if (atts.GetOriginType() == SliceAttributes::Percent)
        {
            double percent = atts.GetOriginPercent() / 100.;
            float bounds[6];
            it->GetExtents(bounds);
            origin[0] = (bounds[1] - bounds[0])*percent + bounds[0];
            origin[1] = (bounds[3] - bounds[2])*percent + bounds[2];
            origin[2] = (bounds[5] - bounds[4])*percent + bounds[4];
        }
        else
        {
            GetOrigin(origin[0], origin[1], origin[2]);
        }
        float tmpD = normal[0]*origin[0] + normal[1]*origin[1] +
                     normal[2]*origin[2];
        vector<int> domains;
        it->GetDomainsList(normal, tmpD, domains);
        rv->GetDataSpecification()->GetRestriction()->RestrictDomains(domains);
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
// ****************************************************************************

void
avtSliceFilter::PreExecute(void)
{
    double nx = atts.GetNormal()[0];
    double ny = atts.GetNormal()[1];
    double nz = atts.GetNormal()[2];

    double ox = 0;
    double oy = 0;
    double oz = 0;
    GetOrigin(ox, oy, oz);

    cachedOrigin[0] = ox;
    cachedOrigin[1] = oy;
    cachedOrigin[2] = oz;

    slicer->SetOrigin(ox, oy, oz);
    slicer->SetNormal(nx, ny, nz);

    // figure out D in the plane equation
    D = nx*ox + ny*oy + nz*oz;

    if (atts.GetProject2d())
        SetUpProjection();
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
// ****************************************************************************

void
avtSliceFilter::SetUpProjection(void)
{
    double nx = atts.GetNormal()[0];
    double ny = atts.GetNormal()[1];
    double nz = atts.GetNormal()[2];

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

    float origin[4] = {ox,oy,oz,1};
    float normal[3] = {nx,ny,nz};
    float upaxis[3] = {ux,uy,uz};

    vtkMath::Normalize(normal);
    vtkMath::Normalize(upaxis);

    //
    // The normal and up vectors form two thirds of a basis, take their
    // cross product to find the third element of the basis.
    //
    float  third[3];
    vtkMath::Cross(upaxis, normal, third); // right-handed
    vtkMath::Normalize(third);  // if normal is not "orthogonal" to "third".

    // Make sure the up axis is orthogonal to third and normal
    vtkMath::Cross(normal, third, upaxis); // right-handed
    vtkMath::Normalize(upaxis); // probably not necessary since "normal" and
                                // "third" are orthogonal

    //
    // Because it is easier to find the Frame-to-Cartesian-Frame conversion
    // matrix and invert it than to calculate the Cartesian-Frame-To-Frame
    // conversion matrix, we will calculate the former matrix.
    //
    vtkMatrix4x4 *ftcf = vtkMatrix4x4::New();
    ftcf->SetElement(0, 0, third[0]);
    ftcf->SetElement(0, 1, third[1]);
    ftcf->SetElement(0, 2, third[2]);
    ftcf->SetElement(0, 3, 0.);
    ftcf->SetElement(1, 0, upaxis[0]);
    ftcf->SetElement(1, 1, upaxis[1]);
    ftcf->SetElement(1, 2, upaxis[2]);
    ftcf->SetElement(1, 3, 0.);
    ftcf->SetElement(2, 0, normal[0]);
    ftcf->SetElement(2, 1, normal[1]);
    ftcf->SetElement(2, 2, normal[2]);
    ftcf->SetElement(2, 3, 0.);
    ftcf->SetElement(3, 0, 0.);
    ftcf->SetElement(3, 1, 0.);
    ftcf->SetElement(3, 2, 0.);
    ftcf->SetElement(3, 3, 1.);

    //
    //  ftcf Transpose can be used as the inverse transform to take 
    //  a point back to its original location, so save it. 
    //
    vtkMatrix4x4::Transpose(ftcf, invTrans);

    vtkMatrix4x4 *cftf = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(ftcf, cftf);

    vtkMatrix4x4 *projTo2D = vtkMatrix4x4::New();
    projTo2D->Identity();
    projTo2D->SetElement(2, 2, 0.);

    vtkMatrix4x4 *result = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(cftf, projTo2D, result);
    projTo2D->Delete();

    //
    // VTK right-multiplies the points, so we need transpose the matrix.
    //
    vtkMatrix4x4 *result_transposed = vtkMatrix4x4::New();
    vtkMatrix4x4::Transpose(result, result_transposed);
    origTrans->DeepCopy(result_transposed); 
    result->Delete();

    vtkMatrixToLinearTransform *mtlt = vtkMatrixToLinearTransform::New();
    mtlt->SetInput(result_transposed);

    result_transposed->Delete();

    transform->SetTransform(mtlt);
    mtlt->Delete();

    //
    // Finish setting up the inverse transform.
    //
    float zdim[4];
    float zdim2[4];
    ftcf->MultiplyPoint(origin, zdim); 
    zdim[0] = 0;
    zdim[1] = 0;
    cftf->MultiplyPoint(zdim, zdim2);
    invTrans->SetElement(0, 3, zdim2[0]);
    invTrans->SetElement(1, 3, zdim2[1]);
    invTrans->SetElement(2, 3, zdim2[2]);

    ftcf->Delete();
    cftf->Delete();
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
//    Made SliceByNode use avtTerminatingSource::QueryCoord instead of
//    avtDatasetExaminer::FindNode to match the SliceByZone code so that
//    this routine succeeds more often.
//
// ****************************************************************************

void
avtSliceFilter::GetOrigin(double &ox, double &oy, double &oz)
{
    double nx = atts.GetNormal()[0];
    double ny = atts.GetNormal()[1];
    double nz = atts.GetNormal()[2];
    double nl = sqrt(nx*nx + ny*ny + nz*nz);

    // We want to make sure for orthogonal slices that "intercept" is still
    // meaningful even when the normal is pointing in the negative direction
    if (nx+ny+nz < 0 && atts.GetAxisType() != SliceAttributes::Arbitrary)
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
          avtTerminatingSource *src = GetInput()->GetTerminatingSource();
          int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
          int cellOrigin  = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
          int domain = atts.GetOriginZoneDomain();
          domain -= blockOrigin;
          int zone = atts.GetOriginZone();
          zone -= cellOrigin;
          float point[3];
          point[0] = FLT_MAX;
          point[1] = FLT_MAX;
          point[2] = FLT_MAX;
          string var = src->GetFullDataSpecification()->GetVariable();
          int    ts  = src->GetFullDataSpecification()->GetTimestep();
          bool success = src->QueryCoords(var, domain, zone, ts, point, true);

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
          avtTerminatingSource *src = GetInput()->GetTerminatingSource();
          int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
          int domain = atts.GetOriginNodeDomain();
          domain -= blockOrigin;
          int node = atts.GetOriginNode();
          float point[3];
          point[0] = DBL_MAX;
          point[1] = DBL_MAX;
          point[2] = DBL_MAX;
          string var = src->GetFullDataSpecification()->GetVariable();
          int    ts  = src->GetFullDataSpecification()->GetTimestep();
          bool success = src->QueryCoords(var, domain, node, ts, point, false);

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
                               " Using point (0., 0., 0.) instead.",
                                                    domain+blockOrigin, node);
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
// ****************************************************************************

vtkDataSet *
avtSliceFilter::ExecuteData(vtkDataSet *in_ds, int domain, std::string)
{

    //
    // We DO NOT by-pass the Slice operator even if the database applied the
    // selection associated with the slice. The reason is that the database
    // still serves up a slab of zones (a 3D mesh) from which a 2D slice is
    // computed.
    //

    //
    // First check to see if we have to slice this domain at all.
    //
    float bounds[6];
    in_ds->GetBounds(bounds);
    float normal[3];
    normal[0] = atts.GetNormal()[0];
    normal[1] = atts.GetNormal()[1];
    normal[2] = atts.GetNormal()[2];
    avtIntervalTree tree(1, 3);
    tree.AddDomain(0, bounds);
    tree.Calculate(true);
    vector<int> domains;
    tree.GetDomainsList(normal, D, domains);
    if (domains.size() <= 0)
    {
        debug5 << "Not slicing domain " << domain
               << ", it does not intersect the plane." << endl;
        return NULL;
    }

    double dbounds[6];
    dbounds[0] = bounds[0];
    dbounds[1] = bounds[1];
    dbounds[2] = bounds[2];
    dbounds[3] = bounds[3];
    dbounds[4] = bounds[4];
    dbounds[5] = bounds[5];
    SetPlaneOrientation(dbounds);

    vtkPolyData *out_ds = vtkPolyData::New();

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
        transform->SetOutput(out_ds);

        //
        // Update will check the modifed time and call Execute.  We have no
        // direct hooks into the Execute method.
        //
        transform->Update();
    }
    else
    {
        slicer->SetOutput(out_ds);

        //
        // Update will check the modifed time and call Execute.  We have no
        // direct hooks into the Execute method.
        //
        slicer->Update();
    }

    vtkDataSet *rv = out_ds;
    if (out_ds->GetNumberOfCells() == 0)
    {
        rv = NULL;
    }

    ManageMemory(rv);
    out_ds->Delete();

    return rv;
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
// ****************************************************************************

void
avtSliceFilter::ReleaseData(void)
{
    avtPluginStreamer::ReleaseData();

    slicer->SetInput(NULL);
    slicer->SetOutput(NULL);
    transform->SetInput(NULL);
    transform->SetOutput(NULL);
    if (celllist != NULL)
    {
        delete [] celllist;
        celllist = NULL;
    }
}


// ****************************************************************************
//  Method: avtSliceFilter::RefashionDataObjectInfo
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
// ****************************************************************************

void
avtSliceFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();
   
    if (inAtts.GetTopologicalDimension() >= 1)
        outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);

    if (atts.GetProject2d())
        outValidity.SetNormalsAreInappropriate(true);

    outValidity.InvalidateZones();

    if (atts.GetProject2d())
    {
        outAtts.SetSpatialDimension(2);
        outValidity.InvalidateSpatialMetaData();
        outValidity.SetPointsWereTransformed(true);
    }

    if (atts.GetProject2d())
    {
        const double *normal = atts.GetNormal();
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
        if ((normal[0] == 0.) && (normal[1] != 0.) && (normal[2] == 0.))
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
        if ((normal[0] == 0.) && (normal[1] == 0.) && (normal[2] != 0.))
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
    }
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
// ****************************************************************************

void
avtSliceFilter::ProjectExtents(double *b)
{
    //
    // Clean up leftovers from previous executions.
    //
    vtkPolyData *new_output = vtkPolyData::New();
    transform->SetOutput(new_output);
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
    vtkPolyData *pd = transform->GetOutput();
    float minmax[4] = { +FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX };
    for (int i = 0 ; i < pd->GetNumberOfCells() ; i++)
    {
        vtkCell *cell = pd->GetCell(i);
        float bounds[6];
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
    b[4] = 0.;
    b[5] = 0.;

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
// ****************************************************************************

void
avtSliceFilter::SetPlaneOrientation(double *b)
{
    double normal[3];
    normal[0] = atts.GetNormal()[0];
    normal[1] = atts.GetNormal()[1];
    normal[2] = atts.GetNormal()[2];

    //
    // Because of the underlying implementation, we will run into cases where
    // there is no slice if it intersects the cell on the face.  Try to 
    // counter-act this.
    //
    double ox = cachedOrigin[0];
    double oy = cachedOrigin[1];
    double oz = cachedOrigin[2];
    if (normal[0] != 0. && normal[1] == 0. && normal[2] == 0.)
    {
        if ((normal[0] > 0.) && (ox == b[0]))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else if ((normal[0] < 0.) && (ox == b[1]))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else
            slicer->SetNormal(normal[0], normal[1], normal[2]);
    }
    else if (normal[0] == 0. && normal[1] != 0. && normal[2] == 0.)
    {
        if ((normal[1] > 0.) && (oy == b[2]))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else if ((normal[1] < 0.) && (oy == b[3]))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else
            slicer->SetNormal(normal[0], normal[1], normal[2]);
    }
    else if (normal[0] == 0. && normal[1] == 0. && normal[2] != 0.)
    {
        if ((normal[2] > 0.) && (oz == b[4]))
            slicer->SetNormal(-normal[0], -normal[1], -normal[2]);
        else if ((normal[2] < 0.) && (oz == b[5]))
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
    plane[0] = atts.GetNormal()[0];
    plane[1] = atts.GetNormal()[1];
    plane[2] = atts.GetNormal()[2];
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

        double b[6];
 
        if (inAtts.GetTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetTrueSpatialExtents()->CopyTo(b);
            ProjectExtents(b);
            outAtts.GetTrueSpatialExtents()->Set(b);
        }

        if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(b);
            ProjectExtents(b);
            outAtts.GetCumulativeTrueSpatialExtents()->Set(b);
        }

        if (inAtts.GetEffectiveSpatialExtents()->HasExtents())
        {
            inAtts.GetEffectiveSpatialExtents()->CopyTo(b);
            ProjectExtents(b);
            outAtts.GetEffectiveSpatialExtents()->Set(b);
        }

        if (inAtts.GetCurrentSpatialExtents()->HasExtents())
        {
            inAtts.GetCurrentSpatialExtents()->CopyTo(b);
            ProjectExtents(b);
            outAtts.GetCurrentSpatialExtents()->Set(b);
        }

        if (inAtts.GetCumulativeCurrentSpatialExtents()->HasExtents())
        {
            inAtts.GetCumulativeCurrentSpatialExtents()->CopyTo(b);
            ProjectExtents(b);
            outAtts.GetCumulativeCurrentSpatialExtents()->Set(b);
        }
    }
}


