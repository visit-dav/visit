// ************************************************************************* //
//                                avtTransform.C                             //
// ************************************************************************* //

#include <avtTransform.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToHomogeneousTransform.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGrid.h>

#include <avtExtents.h>

#include <DebugStream.h>
#include <VisItException.h>


static bool   IsIdentity(vtkMatrix4x4 *);


// START VTK WORKAROUND
class vtkVisItMatrixToHomogeneousTransform : public vtkMatrixToHomogeneousTransform
{
public:
  static vtkVisItMatrixToHomogeneousTransform *New(); 
  virtual void TransformPointsNormalsVectors(vtkPoints *inPts,
                                             vtkPoints *outPts,
                                             vtkDataArray *inNms,
                                             vtkDataArray *outNms,
                                             vtkDataArray *inVrs,
                                             vtkDataArray *outVrs);
};
#include <vtkObjectFactory.h>
#include <vtkMath.h>
vtkStandardNewMacro(vtkVisItMatrixToHomogeneousTransform);

//------------------------------------------------------------------------
template <class T1, class T2, class T3>
static inline double vtkHomogeneousTransformPoint(T1 M[4][4],
                                                  T2 in[3], T3 out[3])
{
  double x = M[0][0]*in[0] + M[0][1]*in[1] + M[0][2]*in[2] + M[0][3];
  double y = M[1][0]*in[0] + M[1][1]*in[1] + M[1][2]*in[2] + M[1][3];
  double z = M[2][0]*in[0] + M[2][1]*in[1] + M[2][2]*in[2] + M[2][3];
  double w = M[3][0]*in[0] + M[3][1]*in[1] + M[3][2]*in[2] + M[3][3];
 
  double f = 1.0/w;
  out[0] = x*f;
  out[1] = y*f;
  out[2] = z*f;
 
  return f;
}

void vtkVisItMatrixToHomogeneousTransform::TransformPointsNormalsVectors(vtkPoints *inPts,
                                                            vtkPoints *outPts,
                                                            vtkDataArray *inNms, 
                                                            vtkDataArray *outNms,
                                                            vtkDataArray *inVrs, 
                                                            vtkDataArray *outVrs)
{
  int n = inPts->GetNumberOfPoints();
  double (*M)[4] = this->Matrix->Element;
  double L[4][4];
  double inPnt[3],outPnt[3],inNrm[3],outNrm[3],inVec[3],outVec[3];
  double w;
 
  this->Update();
 
  if (inNms)
    { // need inverse of the matrix to calculate normals
    vtkMatrix4x4::DeepCopy(*L,this->Matrix);
    vtkMatrix4x4::Invert(*L,*L);
    vtkMatrix4x4::Transpose(*L,*L);
    }
 
  for (int i = 0; i < n; i++)
    {
    inPts->GetPoint(i,inPnt);
 
    // do the coordinate transformation, get 1/w
    double f = vtkHomogeneousTransformPoint(M,inPnt,outPnt);
    outPts->InsertNextPoint(outPnt);
 
    if (inVrs)
      {
      inVrs->GetTuple(i,inVec);
 
      // do the linear homogeneous transformation
      outVec[0] = M[0][0]*inVec[0] + M[0][1]*inVec[1] + M[0][2]*inVec[2];
      outVec[1] = M[1][0]*inVec[0] + M[1][1]*inVec[1] + M[1][2]*inVec[2];
      outVec[2] = M[2][0]*inVec[0] + M[2][1]*inVec[1] + M[2][2]*inVec[2];
      w =         M[3][0]*inVec[0] + M[3][1]*inVec[1] + M[3][2]*inVec[2];
 
      // apply homogeneous correction: note that the f we are using
      // is the one we calculated in the point transformation
      outVec[0] = (outVec[0]-w*outPnt[0])*f;
      outVec[1] = (outVec[1]-w*outPnt[1])*f;
      outVec[2] = (outVec[2]-w*outPnt[2])*f;
 
      outVrs->InsertNextTuple(outVec);
      } 
 
    if (inNms)
      {
      inNms->GetTuple(i,inNrm);
 
      // calculate the w component of the normal
      w = -(inNrm[0]*inPnt[0] + inNrm[1]*inPnt[1] + inNrm[2]*inPnt[2]);
 
      // perform the transformation in homogeneous coordinates
      outNrm[0] = L[0][0]*inNrm[0]+L[0][1]*inNrm[1]+L[0][2]*inNrm[2]+L[0][3]*w;
      outNrm[1] = L[1][0]*inNrm[0]+L[1][1]*inNrm[1]+L[1][2]*inNrm[2]+L[1][3]*w;
      outNrm[2] = L[2][0]*inNrm[0]+L[2][1]*inNrm[1]+L[2][2]*inNrm[2]+L[2][3]*w;
 
      // re-normalize
      vtkMath::Normalize(outNrm);
      outNms->InsertNextTuple(outNrm);
      }
    }
}

// END VTK WORKAROUND

// ****************************************************************************
//  Method: avtTransform constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTransform::avtTransform()
{
    ;
}


// ****************************************************************************
//  Method: avtTransform destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTransform::~avtTransform()
{
    ;
}


// ****************************************************************************
//  Method: avtTransform::ExecuteData
//
//  Purpose:
//      Executes the transform on the specified domain.
//
//  Arguments:
//      in_ds       The input dataset.
//      <unnamed>   The domain number.
//      <unnamed>   The label.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Feb 12 17:36:04 PST 2001
//    Added special logic for rectilinear meshes.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Renamed method from ExecuteDomain to ExecuteData. 
//
//    Hank Childs, Mon Jan 14 09:01:18 PST 2002
//    Reworked memory management to avoid memory leak.
//
//    Hank Childs, Thu Jul 25 11:47:27 PDT 2002
//    Use a different sort of matrix to do the transformation.  The net effect
//    is the same, but there was a bug in the other that caused a crash. (I 
//    fixed it in the VTK repository, but there was no reason not to switch).
//
//    Hank Childs, Mon Aug  5 11:23:48 PDT 2002
//    If we are transforming by the identity, do not do the actual 
//    transformation.
//
//    Hank Childs, Tue Sep 10 15:22:38 PDT 2002
//    Perform all memory management from within this routine.
//
//    Hank Childs, Mon Jul  7 22:31:00 PDT 2003
//    Throw an exception for unknown data types.
//
// ****************************************************************************

vtkDataSet *
avtTransform::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    vtkMatrix4x4 *mat = GetTransform();
    if (IsIdentity(mat))
    {
        return in_ds;
    }

    int  datatype = in_ds->GetDataObjectType();
    if (datatype == VTK_RECTILINEAR_GRID)
    {
        return TransformRectilinear((vtkRectilinearGrid *) in_ds);
    }

    vtkTransformFilter *transform = vtkTransformFilter::New();

    // VTK WORK-AROUND
    //vtkMatrixToHomogeneousTransform *t =vtkMatrixToHomogeneousTransform::New();
    vtkVisItMatrixToHomogeneousTransform *t =vtkVisItMatrixToHomogeneousTransform::New();
    t->SetInput(mat);
    transform->SetTransform(t);
    t->Delete();

    switch (datatype)
    {
      case VTK_POLY_DATA:
      case VTK_UNSTRUCTURED_GRID:
      case VTK_STRUCTURED_GRID:
        transform->SetInput((vtkPointSet *)in_ds);
        break;

      default:
        debug1 << "Improper input to transform." << endl;
        EXCEPTION1(VisItException, "Cannot process unknown data type.");
        break;
    }

    vtkPointSet *out_ds = transform->GetOutput();
    out_ds->Update();

    ManageMemory(out_ds);
    transform->Delete();

    return out_ds;
}


// ****************************************************************************
//  Method: avtTransform::TransformRectilinear
//
//  Purpose:
//      Transforms a rectilinear grid.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
// ****************************************************************************

vtkDataSet *
avtTransform::TransformRectilinear(vtkRectilinearGrid *rgrid)
{
    vtkDataSet *rv = NULL;
    vtkMatrix4x4 *t = GetTransform();
    if (OutputIsRectilinear(t))
    {
        rv = TransformRectilinearToRectilinear(rgrid);
    }
    else
    {
        rv = TransformRectilinearToCurvilinear(rgrid);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtTransform::OutputIsRectilinear
//
//  Purpose:
//      Determines if, based on an input matrix, the transformation of a
//      rectilinear grid would still be rectilinear.
//
//  Arguments:
//      mat     A 4x4 transformation matrix.
//
//  Returns:    true if a rectilinear matrix can be transformed by mat and
//              still be rectilinear, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
// ****************************************************************************

bool
avtTransform::OutputIsRectilinear(vtkMatrix4x4 *mat)
{
    bool  shouldBeZero[4][4] = { { false, true, true, true },
                                 { true, false, true, true },
                                 { true, true, false, true },
                                 { false, false, false, false } };
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            double e = mat->GetElement(j, i); // VTK has their ordering screwed
                                              // up (IMHO).
            if (e != 0. && shouldBeZero[i][j])
            {
                return false;
            }
        }
    }

    return true;
}


// ****************************************************************************
//  Method: avtTransform::TransformRectilinearToRectilinear
//
//  Purpose:
//      Transforms a rectilinear grid to another rectilinear grid.
//
//  Arguments:
//      rgrid   The rectilinear grid to transform.
//
//  Returns:    The transformed grid.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use NewInstance instead of MakeObject, to match new vtk api.
//
// ****************************************************************************

vtkDataSet *
avtTransform::TransformRectilinearToRectilinear(vtkRectilinearGrid *rgrid)
{
    int    i;
    float  outpt[4];
    vtkMatrix4x4 *mat = GetTransform();

    vtkRectilinearGrid *out = vtkRectilinearGrid::New();
    out->ShallowCopy(rgrid);

    //
    // Transform the x-coordinates.
    //
    vtkDataArray *x_orig = rgrid->GetXCoordinates();
    vtkDataArray *x_new  = x_orig->NewInstance();
    int nx = x_orig->GetNumberOfTuples();
    x_new->SetNumberOfTuples(nx);
    float xpt[4] = { 0., 0., 0., 1. };
    for (i = 0 ; i < nx ; i++)
    {
        xpt[0] = x_orig->GetTuple1(i);
        mat->MultiplyPoint(xpt, outpt);
        x_new->SetTuple1(i, outpt[0]);
    }
    out->SetXCoordinates(x_new);
    x_new->Delete();

    //
    // Transform the y-coordinates.
    //
    vtkDataArray *y_orig = rgrid->GetYCoordinates();
    vtkDataArray *y_new  = y_orig->NewInstance();
    int ny = y_orig->GetNumberOfTuples();
    y_new->SetNumberOfTuples(ny);
    float ypt[4] = { 0., 0., 0., 1. };
    for (i = 0 ; i < ny ; i++)
    {
        ypt[1] = y_orig->GetTuple1(i);
        mat->MultiplyPoint(ypt, outpt);
        y_new->SetTuple1(i, outpt[1]);
    }
    out->SetYCoordinates(y_new);
    y_new->Delete();

    //
    // Transform the z-coordinates.
    //
    vtkDataArray *z_orig = rgrid->GetZCoordinates();
    vtkDataArray *z_new  = z_orig->NewInstance();
    int nz = z_orig->GetNumberOfTuples();
    z_new->SetNumberOfTuples(nz);
    float zpt[4] = { 0., 0., 0., 1. };
    for (i = 0 ; i < nz ; i++)
    {
        zpt[2] = z_orig->GetTuple1(i);
        mat->MultiplyPoint(zpt, outpt);
        z_new->SetTuple1(i, outpt[2]);
    }
    out->SetZCoordinates(z_new);
    z_new->Delete();

    //
    // We want to reduce the reference count of this dataset so it doesn't get
    // leaked.  But where to store it?  Fortunately, our base class handles
    // this for us.
    //
    ManageMemory(out);
    out->Delete();

    return out;
}


// ****************************************************************************
//  Method: avtTransform::TransformRectilinearToCurvilinear
//
//  Purpose:
//      Transforms a rectilinear grid into a curvilinear grid.  This is
//      required when there is a rotation.
//
//  Programmer: Hank Childs
//  Creation:   February 6, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 20 08:58:14 PST 2001
//    Copy data in a more general way.
//
//    Kathleen Bonnell, Thu Nov 15 12:48:49 PST 2001 
//    Use vtkDataArray instead of vtkScalars to retrieve vtkRectilinearGrid
//    coordinates in order to match VTK 4.0 API. 
//
//    Hank Childs, Mon Jan 14 09:01:18 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Fri Feb  1 15:09:59 PST 2002
//    Better memory management to avoid exhibiting bug.
//
//    Hank Childs, Thu Jul 25 11:47:27 PDT 2002
//    Transform vector and normal data as well.
//
//    Hank Childs, Mon Aug  5 10:29:22 PDT 2002
//    Renamed routine since we now do not always make curvilinear grids
//    out of rectilinear grids.
//
// ****************************************************************************

vtkDataSet *
avtTransform::TransformRectilinearToCurvilinear(vtkRectilinearGrid *rgrid)
{
    vtkMatrix4x4 *t = GetTransform();
    vtkMatrixToLinearTransform *trans = vtkMatrixToLinearTransform::New();
    trans->SetInput(t);

    int  dims[3];
    rgrid->GetDimensions(dims);

    int  numPts = dims[0]*dims[1]*dims[2];

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);

    vtkDataArray *x = rgrid->GetXCoordinates();
    vtkDataArray *y = rgrid->GetYCoordinates();
    vtkDataArray *z = rgrid->GetZCoordinates();

    int index = 0;
    for (int k = 0 ; k < dims[2] ; k++)
    {
        for (int j = 0 ; j < dims[1] ; j++)
        {
            for (int i = 0 ; i < dims[0] ; i++)
            {
                float inpoint[4];
                inpoint[0] = x->GetComponent(i,0);
                inpoint[1] = y->GetComponent(j,0);
                inpoint[2] = z->GetComponent(k,0);
                inpoint[3] = 1.;

                float outpoint[4];
                t->MultiplyPoint(inpoint, outpoint);

                outpoint[0] /= outpoint[3];
                outpoint[1] /= outpoint[3];
                outpoint[2] /= outpoint[3];

                pts->SetPoint(index++, outpoint);
            }
        }
    }

    vtkStructuredGrid *out = vtkStructuredGrid::New();
    out->SetDimensions(dims);
    out->SetPoints(pts);
    pts->Delete();
    out->GetCellData()->ShallowCopy(rgrid->GetCellData());
    out->GetPointData()->ShallowCopy(rgrid->GetPointData());

    //
    // Now transform vector or normal data.  We are only doing the first
    // vector or normal -- this mirrors what VTK is doing for other data
    // types, so this is meant to be consistent with that.
    //
    vtkDataArray *normals = rgrid->GetPointData()->GetNormals();
    if (normals)
    {
        vtkDataArray *arr = normals->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*normals->GetNumberOfTuples());
        trans->TransformNormals(normals, arr);
        arr->SetName(normals->GetName());
        out->GetPointData()->RemoveArray(normals->GetName());
        out->GetPointData()->SetNormals(arr);
        arr->Delete();
    }
    vtkDataArray *vectors = rgrid->GetPointData()->GetVectors();
    if (vectors)
    {
        vtkDataArray *arr = vectors->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*vectors->GetNumberOfTuples());
        trans->TransformVectors(vectors, arr);
        arr->SetName(vectors->GetName());
        out->GetPointData()->RemoveArray(vectors->GetName());
        out->GetPointData()->SetVectors(arr);
        arr->Delete();
    }
    normals = rgrid->GetCellData()->GetNormals();
    if (normals)
    {
        vtkDataArray *arr = normals->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*normals->GetNumberOfTuples());
        trans->TransformNormals(normals, arr);
        arr->SetName(normals->GetName());
        out->GetCellData()->RemoveArray(normals->GetName());
        out->GetCellData()->SetNormals(arr);
        arr->Delete();
    }
    vectors = rgrid->GetCellData()->GetVectors();
    if (vectors)
    {
        vtkDataArray *arr = vectors->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*vectors->GetNumberOfTuples());
        trans->TransformVectors(vectors, arr);
        arr->SetName(vectors->GetName());
        out->GetCellData()->RemoveArray(vectors->GetName());
        out->GetCellData()->SetVectors(arr);
        arr->Delete();
    }

    //
    // We want to reduce the reference count of this dataset so it doesn't get
    // leaked.  But where to store it?  Fortunately, our base class handles
    // this for us.
    //
    ManageMemory(out);
    trans->Delete();
    out->Delete();

    return out;
}


// ****************************************************************************
//  Method: avtTransform::RefashionDataObjectInfo
//
//  Purpose:
//      Changes the extents by the transform.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar  5 15:55:29 PST 2002
//    Also transformed current spatial extents.  Also told output that its
//    points were transformed.
//
//    Hank Childs, Fri Jan 13 09:49:08 PST 2006
//    Invalidate spatial meta-data.
//
// ****************************************************************************

void
avtTransform::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inAtts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    vtkMatrix4x4 *t = GetTransform();

    (*outAtts.GetTrueSpatialExtents()) =
        (*inAtts.GetTrueSpatialExtents());
    outAtts.GetTrueSpatialExtents()->Transform(t);

    (*outAtts.GetCumulativeTrueSpatialExtents()) =
        (*inAtts.GetCumulativeTrueSpatialExtents());
    outAtts.GetCumulativeTrueSpatialExtents()->Transform(t);

    (*outAtts.GetEffectiveSpatialExtents()) =
        (*inAtts.GetEffectiveSpatialExtents());
    outAtts.GetEffectiveSpatialExtents()->Transform(t);

    (*outAtts.GetCurrentSpatialExtents()) =
        (*inAtts.GetCurrentSpatialExtents());
    outAtts.GetCurrentSpatialExtents()->Transform(t);

    GetOutput()->GetInfo().GetValidity().SetPointsWereTransformed(true);
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


// ****************************************************************************
//  Function: IsIdentity
//
//  Purpose:
//      Identifies if a matrix is the identity matrix.
//
//  Arguments:
//      mat     The matrix to check against the identity.
//
//  Returns:    true if it is the identity matrix, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2002
//
// ****************************************************************************

bool
IsIdentity(vtkMatrix4x4 *mat)
{
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            float e = mat->GetElement(i, j);
            if (i == j && e != 1.)
                return false;
            else if (i != j && e != 0.)
                return false;
        }
    }

    return true;
}


