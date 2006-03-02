// ************************************************************************* //
//  File: avtConeFilter.C
// ************************************************************************* //

#include <avtConeFilter.h>

#include <math.h>
#include <float.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkClipPolyData.h>
#include <vtkCone.h>
#include <vtkVisItCutter.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunction.h>
#include <vtkMath.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToPolyDataFilter.h>
#include <vtkRectilinearGrid.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <avtExtents.h>

#include <BadVectorException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>


void      PolarExtents(double *, vtkTransformPolyDataFilter *, vtkVisItCutter *,
                       bool, double);
void      ProjectExtents(double *, vtkTransformPolyDataFilter *, vtkVisItCutter *,
                         bool, double);


//
// This is the definition of vtkPolarTransformFilter.  It's implementation
// is fairly custom to what the avtConeFilter is doing, hence the inclusion
// directly into the AVT file.  This filter assumes that we have 2D data
// centered around the origin.  It then converts the data to (theta, r) format.
//
// In addition, it splits cells that wrap around the theta=0 degrees axis.
// It also make assumptions about cells that are incident to the origin and
// bloats them out in a way that makes "prettier pictures".
//
class vtkPolarTransformFilter : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkPolarTransformFilter *New();
protected:
  vtkPolarTransformFilter() {;};
  ~vtkPolarTransformFilter() {;};

  void Execute();
private:
  vtkPolarTransformFilter(const vtkPolarTransformFilter&);  //Not implemented.
  void operator=(const vtkPolarTransformFilter&);  // Not implemented.
};

vtkStandardNewMacro(vtkPolarTransformFilter);

void
vtkPolarTransformFilter::Execute(void)
{
    int   i, j;

    // This assumes that the data has already been projected to 2D.
    vtkPolyData *input = this->GetInput();
    vtkPolyData *output = this->GetOutput();
    vtkIdType nPts   = input->GetNumberOfPoints();
    vtkIdType nCells = input->GetNumberOfCells();

    //
    // First copy over the points.  We will transform them to polar coordinates
    // as we copy them.
    //
    vtkPoints *newPts = vtkPoints::New();
    vtkPoints *oldPts = input->GetPoints();
    newPts->Allocate(nPts + 2*nCells); // Justification for this
                                       // large of a buffer is below.

#if defined(_WIN32) && !defined(M_PI)
#define M_PI 3.14159
#endif

    for (i = 0 ; i < nPts ; i++)
    {
        float pt[3];
        oldPts->GetPoint(i, pt);
        float r = sqrt(pt[0]*pt[0] + pt[1]*pt[1]);
        float theta = 0.;
        //
        // This isn't a pretty way to calculate theta -- but I know it works.
        //
        if (pt[1] == 0.)
        {
            if (pt[0] < 0.)
            {
                theta = M_PI;
            }
            else
            {
                theta = 0.;
            }
        }
        else if (pt[0] == 0.)
        {
            if (pt[1] < 0.)
            {
                theta = M_PI * 3. / 2.;
            }
            else
            {
                theta = M_PI / 2.;
            }
        }
        else if (pt[0] > 0. && pt[1] > 0.)
        {
            theta = atan(pt[1] / pt[0]);
        }
        else if (pt[0] > 0. && pt[1] < 0.)
        {
            theta = 2*M_PI - atan(-pt[1] / pt[0]);
        }
        else if (pt[0] < 0. && pt[1] > 0.)
        {
            theta = M_PI - atan(pt[1] / -pt[0]);
        }
        else if (pt[0] < 0. && pt[1] < 0.)
        {
            theta = M_PI + atan(-pt[1] / -pt[0]);
        }
        newPts->InsertNextPoint(theta, r, 0.);
    }

    //
    // Now we want to copy over the cells.  This is not as easy as it first 
    // looks, because some cells will fall along the 0-2pi boundary and
    // they get stretched across the whole screen.  We need to split cells 
    // in that case and add new points.
    //
    vtkPointData *inPD  = input->GetPointData();
    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *inCD  = input->GetCellData();
    vtkCellData  *outCD = output->GetCellData();

    //
    // Each cell can lead to the addition of two more points.  Then we have
    // npts + 2*ncells as the possible number of points in the output.
    //
    // Each cell can be split into at most two cells.  Then we have
    // 2*ncells as the possible number of cells in the output.
    //
    outPD->CopyAllocate(inPD, nPts + 2*nCells);
    outCD->CopyAllocate(inCD, 2*nCells);
    output->Allocate(2*nCells, 3);

    for (i = 0 ; i < nPts ; i++)
    {
        outPD->CopyData(inPD, i, i);
    }

    int ptIndex = nPts;
    int currentCell = 0;
    vtkIdType leftIds[100];   // Should be sufficiently big
    vtkIdType rightIds[100];  // Should be sufficiently big
    for (i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = input->GetCell(i);
        int cellType = cell->GetCellType();
        int nPtsForThisCell = cell->GetNumberOfPoints();
        bool near0PI = false;
        bool near2PI = false;
        vtkIdList *ids = cell->GetPointIds();

        //
        // Walk through the cell and identify if it is across a boundary.
        // 
        for (j = 0 ; j < nPtsForThisCell ; j++)
        {
            float *pt = newPts->GetPoint(ids->GetId(j));
            if (pt[0] < M_PI / 4.)
            {
                near0PI = true;
            }
            if (pt[0] > 7.*M_PI / 4.)
            {
                near2PI = true;
            }
        }

        if (near0PI && near2PI)
        {
            //
            // We have a dreaded boundary cell.  Make a copy on the left and
            // on the right.
            //
            for (j = 0 ; j < nPtsForThisCell ; j++)
            {
                float *pt = newPts->GetPoint(ids->GetId(j));
                float pt2[3];
                if (pt[0] < M_PI)
                {
                    leftIds[j] = ids->GetId(j);
                    rightIds[j] = ptIndex;
                    pt2[0] = pt[0] + 2*M_PI;
                }
                else
                {
                    rightIds[j] = ids->GetId(j);
                    leftIds[j] = ptIndex;
                    pt2[0] = pt[0] - 2*M_PI;
                }
                pt2[1] = pt[1];
                pt2[2] = pt[2];
                newPts->InsertNextPoint(pt2);
                outPD->CopyData(inPD, ids->GetId(j), ptIndex);
                ptIndex++;
            }

            //
            // Copy the cell data once for the left cell and once for the
            // right cell.
            //
            outCD->CopyData(inCD, i, currentCell++);
            outCD->CopyData(inCD, i, currentCell++);
            output->InsertNextCell(cellType, nPtsForThisCell, leftIds);
            output->InsertNextCell(cellType, nPtsForThisCell, rightIds);
        }
        else
        {
            //
            // Assume this cell is in tact and copy it over.
            //
            outCD->CopyData(inCD, i, currentCell++);
            output->InsertNextCell(cellType, ids);
        }
    }

    output->SetPoints(newPts);
    newPts->Squeeze();
    outCD->Squeeze();
    outPD->Squeeze();
    output->Squeeze();
    newPts->Delete();
}


// ****************************************************************************
//  Method: avtConeFilter constructor
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed May 29 16:09:58 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 28 10:28:22 PDT 2005
//    Use vtkVisitCutter (instead of vtkCutter), since it has logic to better
//    handle CellData.
//
// ****************************************************************************

avtConeFilter::avtConeFilter()
{
    cone = vtkCone::New();
    plane = vtkPlane::New();
    allFunctions = vtkImplicitBoolean::New();
    allFunctions->AddFunction(cone);
    allFunctions->AddFunction(plane);
    allFunctions->SetOperationTypeToIntersection();
    cutter = vtkVisItCutter::New();
    cutter->SetCutFunction(allFunctions);
    transform = vtkTransformPolyDataFilter::New();
    polar = vtkPolarTransformFilter::New();

    //
    // We have enough information to set up the clipOffSides in entirety.
    //
    clipOffSides = vtkClipPolyData::New();
    vtkImplicitBoolean *clipPlane = vtkImplicitBoolean::New();

    vtkPlane *leftPlane = vtkPlane::New();
    leftPlane->SetNormal(1., 0., 0.);
    leftPlane->SetOrigin(0, 0., 0.);

    vtkPlane *rightPlane = vtkPlane::New();
    rightPlane->SetNormal(-1., 0., 0.);
    rightPlane->SetOrigin(2*M_PI, 0., 0.);

    clipPlane->AddFunction(leftPlane);
    clipPlane->AddFunction(rightPlane);
    clipPlane->SetOperationTypeToUnion();  // I think this should be
                                           // intersection, but that doesn't
                                           // work.  Union does.
    clipOffSides->SetClipFunction(clipPlane);
    clipOffSides->DebugOn();

    leftPlane->Delete();
    rightPlane->Delete();
    clipPlane->Delete();

    //
    // We may want to clip off the end of the cone (if the user specifies to).
    //
    clipByLength = vtkClipPolyData::New();
    planeToClipByLength = vtkPlane::New();
    clipByLength->SetClipFunction(planeToClipByLength);

    //
    // We have to play tricks in terms of splitting points to make our cone
    // look good after it is split.
    //
    clipBottom = vtkClipPolyData::New();
    planeToClipBottom = vtkPlane::New();
    clipBottom->SetClipFunction(planeToClipBottom);
}


// ****************************************************************************
//  Method: avtConeFilter destructor
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed May 29 16:09:58 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtConeFilter::~avtConeFilter()
{
    if (clipOffSides != NULL)
    {
        clipOffSides->Delete();
        clipOffSides = NULL;
    }
    if (cone != NULL)
    {
        cone->Delete();
        cone = NULL;
    }
    if (cutter != NULL)
    {
        cutter->Delete();
        cutter = NULL;
    }
    if (allFunctions != NULL)
    {
        allFunctions->Delete();
        allFunctions = NULL;
    }
    if (plane != NULL)
    {
        plane->Delete();
        plane = NULL;
    }
    if (polar != NULL)
    {
        polar->Delete();
        polar = NULL;
    }
    if (transform != NULL)
    {
        transform->Delete();
        transform = NULL;
    }
    if (clipBottom != NULL)
    {
        clipBottom->Delete();
        clipBottom = NULL;
    }
    if (planeToClipBottom != NULL)
    {
        planeToClipBottom->Delete();
        planeToClipBottom = NULL;
    }
    if (clipByLength != NULL)
    {
        clipByLength->Delete();
        clipByLength = NULL;
    }
    if (planeToClipByLength != NULL)
    {
        planeToClipByLength->Delete();
        planeToClipByLength = NULL;
    }
}


// ****************************************************************************
//  Method:  avtConeFilter::Create
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed May 29 16:09:58 PST 2002
//
// ****************************************************************************

avtFilter *
avtConeFilter::Create()
{
    return new avtConeFilter();
}


// ****************************************************************************
//  Method:      avtConeFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed May 29 16:09:58 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue May 20 16:02:52 PDT 2003 
//    Added tests for bad normal, upAxis.
//
// ****************************************************************************

void
avtConeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ConeAttributes*)a;
    const double *n= atts.GetNormal();
    if (n[0] == 0. && n[1] == 0. && n[2] == 0.)
    {
        EXCEPTION1(BadVectorException, "Normal");
        return;
    }
    if (atts.GetRepresentation() == ConeAttributes::Flattened ||
        atts.GetRepresentation() == ConeAttributes::R_Theta )
    {
        n = atts.GetUpAxis();
        if (n[0] == 0. && n[1] == 0. && n[2] == 0.)
        {
            EXCEPTION1(BadVectorException, "UpAxis");
            return;
        }
    }
    SetUpCone();
    SetUpProjection();
    SetUpClipping();
}


// ****************************************************************************
//  Method: avtConeFilter::SetUpCone
//
//  Purpose:
//      Sets up the cutter that clips to a cone.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2002
//
// ****************************************************************************

void
avtConeFilter::SetUpCone(void)
{
    cone->SetAngle(atts.GetAngle());

    //
    // The VTK cone must be set up as something that goes along the x-axis.
    // So set up a transform that will transform the point to something
    // that makes sense for that cone.
    //
    const double *normal = atts.GetNormal();
    const double *origin = atts.GetOrigin();

    //
    // We are going to rotate the cone's true direction of project to be
    // along the x-axis.  Formula for doing this: calculate the cross product
    // and then rotate along the axis of the vector formed by the cross
    // product.  The amount to rotate is the angle between cone's true
    // direction of projection and the x-axis -- that is their dot product.
    //
    vtkTransform *trans = vtkTransform::New();
    double normal_normalized[3];
    double mag = sqrt(normal[0]*normal[0] + normal[1]*normal[1]
                      + normal[2]*normal[2]);
    normal_normalized[0] = normal[0] / mag;
    normal_normalized[1] = normal[1] / mag;
    normal_normalized[2] = normal[2] / mag;
    double cross[3];
    cross[0] = 0.;
    cross[1] = -normal_normalized[2];
    cross[2] = normal_normalized[1];
    if (cross[0] != 0. || cross[1] != 0. || cross[2] != 0.)
    {
        double dot = normal_normalized[0];
        double angle = -acos(dot);
        angle /= vtkMath::DegreesToRadians();
        trans->RotateWXYZ(angle, cross[0], cross[1], cross[2]);
    }
    cone->SetTransform(trans);
    trans->Translate(-origin[0], -origin[1], -origin[2]);
    trans->Delete();

    //
    // VTK only seems to be happy if we use the "-" normal.  I'm not sure why
    // this is.
    //
    plane->SetNormal(-normal[0], -normal[1], -normal[2]);
    plane->SetOrigin(origin[0], origin[1], origin[2]);
}


// ****************************************************************************
//  Method: avtConeFilter::SetUpProjection
//
//  Purpose:
//      Sets up the projection transformation that will transform the cone
//      to 2D.
//
//  Notes:      This was all stolen from the code Jeremy wrote to project to
//              2D in the arbitrary slice.
//
//  Programmer: Hank Childs (Jeremy Meredith)
//  Creation:   May 31, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:37:42 PDT 2004
//    Make the transform use right-handed coords, similar to recent changes
//    made to slice filter.
//
//    Hank Childs, Fri Aug 13 08:25:52 PDT 2004
//    Use 4-tuples for multiplying points (or else VTK does a UMR).
//    Also renormalize basis vectors to ensure they are truly unit vectors.
//
//    Kathleen Bonnell, Fri May 13 15:03:26 PDT 2005
//    Fix memory leak. 
//
// ****************************************************************************

void
avtConeFilter::SetUpProjection(void)
{
    const double *Cnormal = atts.GetNormal();
    const double *Cupaxis = atts.GetUpAxis();
    const double *Corigin = atts.GetOrigin();

    double nx = Cnormal[0];
    double ny = Cnormal[1];
    double nz = Cnormal[2];
    double ux = Cupaxis[0];
    double uy = Cupaxis[1];
    double uz = Cupaxis[2];

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

    float origin[4] = {Corigin[0], Corigin[1], Corigin[2],1};
    float normal[3] = {nx,ny,nz};
    float upaxis[3] = {ux,uy,uz};

    vtkMath::Normalize(normal);
    vtkMath::Normalize(upaxis);

    //
    // The normal and up vectors for two thirds of a basis, take their
    // cross product to find the third element of the basis.
    //
    float  third[3];
    vtkMath::Cross(upaxis, normal, third);
    vtkMath::Normalize(third);

    // Make sure the up axis is orthogonal to third and normal
    vtkMath::Cross(normal, third, upaxis);
    vtkMath::Normalize(upaxis);

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
    result->Delete();
 
    vtkMatrixToLinearTransform *mtlt = vtkMatrixToLinearTransform::New();
    mtlt->SetInput(result_transposed);
    result_transposed->Delete();
 
    transform->SetTransform(mtlt);
    mtlt->Delete();

    float zdim[4];
    ftcf->MultiplyPoint(origin, zdim);
    zdim[0] = 0;
    zdim[1] = 0;
    cftf->MultiplyPoint(zdim, zdim);
    
    cftf->Delete();
    ftcf->Delete();
}


// ****************************************************************************
//  Method: avtConeFilter::SetUpClipping
//
//  Purpose:
//      Sets up the clipping planes.  The clipping routines are used mostly for
//      tricks to make things look correct.
//
//  Programmer: Hank Childs
//  Creation:   June 3, 2002
//
// ****************************************************************************

void
avtConeFilter::SetUpClipping(void)
{
    //
    // If we clip by a small amount away from the origin, our conversion to
    // cylindrical coordinates will look much better around the
    // line radius == 0.
    //
    const double *normal = atts.GetNormal();
    const double *origin = atts.GetOrigin();
    float shiftedOrigin[3];
    shiftedOrigin[0] = origin[0] + 0.001*normal[0];
    shiftedOrigin[1] = origin[1] + 0.001*normal[1];
    shiftedOrigin[2] = origin[2] + 0.001*normal[2];
    float fnormal[3];
    fnormal[0] = normal[0];
    fnormal[1] = normal[1];
    fnormal[2] = normal[2];
    planeToClipBottom->SetOrigin(shiftedOrigin);
    planeToClipBottom->SetNormal(fnormal);

    //
    // The user may decide to cut off the cone after a certain length.  At the
    // very least, this makes for prettier pictures.
    //
    double length = atts.GetLength();
    double mag = normal[0]*normal[0] + normal[1]*normal[1]+normal[2]*normal[2];
    mag = sqrt(mag);
    shiftedOrigin[0] = origin[0] + length*normal[0] / mag;
    shiftedOrigin[1] = origin[1] + length*normal[1] / mag;
    shiftedOrigin[2] = origin[2] + length*normal[2] / mag;
    fnormal[0] = -normal[0];
    fnormal[1] = -normal[1];
    fnormal[2] = -normal[2];
    planeToClipByLength->SetOrigin(shiftedOrigin);
    planeToClipByLength->SetNormal(fnormal);
}


// ****************************************************************************
//  Method: avtConeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtConeFilter with the given
//      parameters would result in an equivalent avtConeFilter.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed May 29 16:09:58 PST 2002
//
// ****************************************************************************

bool
avtConeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ConeAttributes*)a);
}


// ****************************************************************************
//  Method: avtConeFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Cone filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed May 29 16:09:58 PST 2002
//
//  Modifications:
//
//    Hank Childs, Fri Jun  7 09:48:02 PDT 2002 
//    Handle multi-block case better.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002   
//    Use NewInstance instead of MakeObject, in order to match vtk's new api. 
//
// ****************************************************************************

vtkDataSet *
avtConeFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    vtkDataSet *rv = NULL;

    //
    // First clip to the cone.
    //
    cutter->SetInput(in_ds);
    vtkPolyData *cur_ds = cutter->GetOutput();

    if (atts.GetCutByLength())
    {
        clipByLength->SetInput(cur_ds);
        cur_ds = clipByLength->GetOutput();
    }

    //
    // Now apply any necessary postprocessing to get the correct
    // representation.
    //
    switch (atts.GetRepresentation())
    {
      case ConeAttributes::ThreeD:
        rv = cur_ds;
        break;

      case ConeAttributes::Flattened:
        transform->SetInput(cur_ds);
        rv = transform->GetOutput();
        break;

      case ConeAttributes::R_Theta:
        clipBottom->SetInput(cur_ds);
        transform->SetInput(clipBottom->GetOutput());
        polar->SetInput(transform->GetOutput());
        clipOffSides->SetInput(polar->GetOutput());
        rv = clipOffSides->GetOutput();
        break;

      default:
        EXCEPTION0(ImproperUseException);
    }

    rv->Update();
    vtkDataSet *ds = (vtkDataSet *) rv->NewInstance();
    ds->ShallowCopy(rv);
    ManageMemory(ds);
    ds->Delete();
    return ds;
}


// ****************************************************************************
//  Method: avtConeFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Changes to topological dimension of the output to be one less that the
//      input.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Apr 16 08:29:12 PDT 2003
//    Indicate that any transforms in the pipeline cannot be used in 
//    conjunction with this filter for Flattened, or R-theta Reps.
//
//    Kathleen Bonnell, Wed Jun  2 09:37:42 PDT 2004
//    Set CanUseInvTransform to false.  for Flattened or R-theta.
//
//    Kathleen Bonnell, Thu Mar  2 14:26:06 PST 2006 
//    Set ZonesSplit.
//
// ****************************************************************************

void
avtConeFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();
 
    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);
    outValidity.InvalidateZones();
    outValidity.ZonesSplit();

    double angle = atts.GetAngle();
    double length = atts.GetLength();
    angle *= vtkMath::DegreesToRadians();

    //
    //             |------/
    //             |     /|
    // length=====>|    / |
    //             |   /<==== side of cone
    //             |  /   |
    //             | /    |
    //             |/-----|
    //                ^-------projected to 2D.
    //
    double effLength = length / cos(angle); // the length along the cone.
    effLength *= sin(angle); // projected to 2D.

    if (atts.GetRepresentation() == ConeAttributes::Flattened)
    {
        outAtts.SetSpatialDimension(2);
        outValidity.InvalidateSpatialMetaData();
        outValidity.SetPointsWereTransformed(true);
        outAtts.SetCanUseInvTransform(false);
        outAtts.SetCanUseTransform(false);
 
        double b[6];
 
        if (inAtts.GetTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetTrueSpatialExtents()->CopyTo(b);
            ProjectExtents(b,transform,cutter,atts.GetCutByLength(),effLength);
            outAtts.GetTrueSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(b);
            ProjectExtents(b,transform,cutter,atts.GetCutByLength(),effLength);
            outAtts.GetCumulativeTrueSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetEffectiveSpatialExtents()->HasExtents())
        {
            inAtts.GetEffectiveSpatialExtents()->CopyTo(b);
            ProjectExtents(b,transform,cutter,atts.GetCutByLength(),effLength);
            outAtts.GetEffectiveSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetCurrentSpatialExtents()->HasExtents())
        {
            inAtts.GetCurrentSpatialExtents()->CopyTo(b);
            ProjectExtents(b,transform,cutter,atts.GetCutByLength(),effLength);
            outAtts.GetCurrentSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetCumulativeCurrentSpatialExtents()->HasExtents())
        {
            inAtts.GetCumulativeCurrentSpatialExtents()->CopyTo(b);
            ProjectExtents(b,transform,cutter,atts.GetCutByLength(),effLength);
            outAtts.GetCumulativeCurrentSpatialExtents()->Set(b);
        }
    }
    if (atts.GetRepresentation() == ConeAttributes::R_Theta)
    {
        outAtts.SetSpatialDimension(2);
        outValidity.InvalidateSpatialMetaData();
        outValidity.SetPointsWereTransformed(true);
        outAtts.SetCanUseInvTransform(false);
        outAtts.SetCanUseTransform(false);
 
        double b[6];
 
        if (inAtts.GetTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetTrueSpatialExtents()->CopyTo(b);
            PolarExtents(b, transform, cutter,atts.GetCutByLength(),effLength);
            outAtts.GetTrueSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(b);
            PolarExtents(b, transform, cutter,atts.GetCutByLength(),effLength);
            outAtts.GetCumulativeTrueSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetEffectiveSpatialExtents()->HasExtents())
        {
            inAtts.GetEffectiveSpatialExtents()->CopyTo(b);
            PolarExtents(b, transform, cutter,atts.GetCutByLength(),effLength);
            outAtts.GetEffectiveSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetCurrentSpatialExtents()->HasExtents())
        {
            inAtts.GetCurrentSpatialExtents()->CopyTo(b);
            PolarExtents(b, transform, cutter,atts.GetCutByLength(),effLength);
            outAtts.GetCurrentSpatialExtents()->Set(b);
        }
 
        if (inAtts.GetCumulativeCurrentSpatialExtents()->HasExtents())
        {
            inAtts.GetCumulativeCurrentSpatialExtents()->CopyTo(b);
            PolarExtents(b, transform, cutter,atts.GetCutByLength(),effLength);
            outAtts.GetCumulativeCurrentSpatialExtents()->Set(b);
        }
    }
}


// ****************************************************************************
//  Method: avtConeFilter::ReleaseData
//
//  Purpose:
//      Release the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
//    Hank Childs, Fri Mar 11 07:37:05 PST 2005
//    Fix non-problem size leak introduced with last fix.
//
// ****************************************************************************

void
avtConeFilter::ReleaseData(void)
{
    avtPluginStreamer::ReleaseData();

    cutter->SetInput(NULL);
    vtkPolyData *p = vtkPolyData::New();
    cutter->SetOutput(p);
    p->Delete();

    polar->SetInput(NULL);
    p = vtkPolyData::New();
    polar->SetOutput(p);
    p->Delete();

    transform->SetInput(NULL);
    p = vtkPolyData::New();
    transform->SetOutput(p);
    p->Delete();

    clipOffSides->SetInput(NULL);
    p = vtkPolyData::New();
    clipOffSides->SetOutput(p);
    p->Delete();

    clipBottom->SetInput(NULL);
    p = vtkPolyData::New();
    clipBottom->SetOutput(p);
    p->Delete();

    clipByLength->SetInput(NULL);
    p = vtkPolyData::New();
    clipByLength->SetOutput(p);
    p->Delete();

}


// ****************************************************************************
//  Function: ProjectExtents
//
//  Purpose:
//      Projects extents for 3D to 2D based on a cutter and projection matrix.
//
//  Arguments:
//      b       A buffer of extents.
//      trans   The transformation.
//      cutter  The slice to use.
//
//  Notes:      Stolen from avtAsliceFilter.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Jul 2 16:16:58 PST 2002
//    Made it work on Windows.
//
//    Kathleen Bonnell, Thu Apr 28 10:28:22 PDT 2005
//    Use vtkVisitCutter (instead of vtkCutter), since it has logic to better
//    handle CellData.
//
// ****************************************************************************
 
void
ProjectExtents(double *b, vtkTransformPolyDataFilter *trans, 
               vtkVisItCutter *cutter, bool cutOff, double length)
{
    if (cutOff)
    {
        b[0] = -length;
        b[1] = +length;
        b[2] = -length;
        b[3] = +length;
        b[4] = 0.;
        b[5] = 0.;
        return;
    }

    int i;
    int dim = 10;

    //
    // Set up a one-thousand celled rectilinear grid based on the bounding box.
    // We need that many cells to resolve the curve correctly.
    //
    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(dim);
    double step = (b[1] - b[0]) / (dim-1);
    for (i = 0 ; i < dim ; i++)
    {
        x->SetComponent(i, 0, b[0]+i*step);
    }
 
    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfTuples(dim);
    step = (b[3] - b[2]) / (dim-1);
    for (i = 0 ; i < dim ; i++)
    {
        y->SetComponent(i, 0, b[2]+i*step);
    }
 
    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfTuples(dim);
    step = (b[5] - b[4]) / (dim-1);
    for (i = 0 ; i < dim ; i++)
    {
        z->SetComponent(i, 0, b[4]+i*step);
    }
 
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dim, dim, dim);
    rgrid->SetXCoordinates(x);
    rgrid->SetYCoordinates(y);
    rgrid->SetZCoordinates(z);
 
    //
    // Slice and project our bounding box to mimic what would happen to our
    // original dataset.
    //
    cutter->SetInput(rgrid);
    trans->SetInput(cutter->GetOutput());
    trans->Update();
 
    //
    // Now iterate through the resulting triangles and determine what the
    // extents are.
    //
    vtkPolyData *pd = trans->GetOutput();
    float minmax[4] = { +FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX };
    for (i = 0 ; i < pd->GetNumberOfCells() ; i++)
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
//  Function: PolarExtents
//
//  Purpose:
//      Polar extents based on a cutter and projection matrix.
//
//  Arguments:
//      b       A buffer of extents.
//      trans   The transformation.
//      cutter  The slice to use.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 28 10:28:22 PDT 2005
//    Use vtkVisitCutter (instead of vtkCutter), since it has logic to better
//    handle CellData.
//
// ****************************************************************************
 
void
PolarExtents(double *b, vtkTransformPolyDataFilter *trans, vtkVisItCutter *cutter,
             bool cutByLength, double length)
{
    if (cutByLength)
    {
        b[0] = 0.;
        b[1] = 2*M_PI;
        b[2] = 0.;
        b[3] = length;
        b[4] = 0.;
        b[5] = 0.;
        return;
    }

    int i;
    int dim = 10;

    //
    // Set up a one-thousand celled rectilinear grid based on the bounding box.
    // We need that many cells to resolve the curve correctly.
    //
    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(dim);
    double step = (b[1] - b[0]) / (dim-1);
    for (i = 0 ; i < dim ; i++)
    {
        x->SetComponent(i, 0, b[0]+i*step);
    }
 
    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfTuples(dim);
    step = (b[3] - b[2]) / (dim-1);
    for (i = 0 ; i < dim ; i++)
    {
        y->SetComponent(i, 0, b[2]+i*step);
    }
 
    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfTuples(dim);
    step = (b[5] - b[4]) / (dim-1);
    for (i = 0 ; i < dim ; i++)
    {
        z->SetComponent(i, 0, b[4]+i*step);
    }
 
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dim, dim, dim);
    rgrid->SetXCoordinates(x);
    rgrid->SetYCoordinates(y);
    rgrid->SetZCoordinates(z);
 
    //
    // Slice and project our bounding box to mimic what would happen to our
    // original dataset.
    //
    cutter->SetInput(rgrid);
    trans->SetInput(cutter->GetOutput());
    trans->Update();
 
    //
    // Now iterate through the resulting triangles and determine what the
    // extents are.
    //
    vtkPolyData *pd = trans->GetOutput();
    float minmax[4] = { +FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX };
    for (i = 0 ; i < pd->GetNumberOfCells() ; i++)
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
        float r02 = sqrt(minmax[0]*minmax[0] + minmax[2]*minmax[2]);
        float r03 = sqrt(minmax[0]*minmax[0] + minmax[3]*minmax[3]);
        float r12 = sqrt(minmax[1]*minmax[1] + minmax[2]*minmax[2]);
        float r13 = sqrt(minmax[1]*minmax[1] + minmax[3]*minmax[3]);
        float max1 = (r02 > r03 ? r02 : r03);
        float max2 = (r12 > r13 ? r12 : r13);
        float max  = (max1 > max2 ? max1 : max2);
        b[0] = 0.;
        b[1] = 2*M_PI;
        b[2] = 0.;
        b[3] = max;
    }
    else
    {
        b[0] = 0.;
        b[1] = 2*M_PI;
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
//  Method: avtConeFilter::PerformRestriction
//
//  Purpose:
//    Turn on Zone numbers flag if needed, so that original cell array
//    will be propagated throught the pipeline.
//
//  Arguments:
//      spec    The current pipeline specification.
//
//  Returns:    The new specification.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 20, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:21:46 PDT 2004
//    Turn on node numbers when appropriate.
//
// ****************************************************************************

avtPipelineSpecification_p
avtConeFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec);

    if (atts.GetRepresentation() == ConeAttributes::Flattened)
    {
        if (rv->GetDataSpecification()->MayRequireZones())
        {
            rv->GetDataSpecification()->TurnZoneNumbersOn();
        }
        if (rv->GetDataSpecification()->MayRequireNodes())
        {
            rv->GetDataSpecification()->TurnNodeNumbersOn();
        }
    }
    return rv;
}
