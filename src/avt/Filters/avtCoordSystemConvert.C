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
//  File: avtCoordSystemConvert.C
// ************************************************************************* //

#include <avtCoordSystemConvert.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDoubleArray.h>

#include <vtkVisItUtility.h>
#include <vtkIdList.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>


typedef void(*xformFunc)(double *, const double *);

// ****************************************************************************
//  Method: avtCoordSystemConvert constructor
//
//  Programmer: Hank Childs
//  Creation:   Fri Jun 27 16:41:32 PST 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Aug  7 15:34:37 EDT 2009
//    Initialize vectorTransformMethod.
//
// ****************************************************************************

avtCoordSystemConvert::avtCoordSystemConvert()
{
    inputSys  = CARTESIAN;
    outputSys = CARTESIAN;
    vectorTransformMethod = AsDirection;
}


// ****************************************************************************
//  Method: avtCoordSystemConvert destructor
//
//  Programmer: Hank Childs
//  Creation:   Fri Jun 27 16:41:32 PST 2003
//
//  Modifications:
//
// ****************************************************************************

avtCoordSystemConvert::~avtCoordSystemConvert()
{
}


// ****************************************************************************
//  Function: SphericalToCartesianPoint
//
//  Purpose:
//      Converts spherical coordinates to cartesian coordinates.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Oct 17 14:24:01 PDT 2007
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  3 16:41:35 EDT 2009
//    Reversed sense of 2nd and 3rd spherical components to match ISO 31-11.
//    Removed "in2D" setting.
//
// ****************************************************************************

static void 
SphericalToCartesianPoint(double *newpt, const double *pt)
{
    newpt[0] = pt[0]*cos(pt[2])*sin(pt[1]);
    newpt[1] = pt[0]*sin(pt[2])*sin(pt[1]);
    newpt[2] = pt[0]*cos(pt[1]);
}

// ****************************************************************************
//  Function: CartesianToCylindricalPoint
//
//  Purpose:
//      Converts cartesian coordinates to cylindrical coordinates.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Oct 17 14:24:01 PDT 2007
//
//  Modifications:
//    Jeremy Meredith, Fri Aug  7 15:34:16 EDT 2009
//    Removed "in2D" setting.
// ****************************************************************************

static void 
CartesianToCylindricalPoint(double *newpt, const double *pt)
{
    newpt[1] = atan2(pt[1], pt[0]);
    if (newpt[1] < 0.)
        newpt[1] = 2*vtkMath::Pi() + newpt[1];
    newpt[0] = sqrt(pt[0]*pt[0] + pt[1]*pt[1]);
    newpt[2] = pt[2];
}

// ****************************************************************************
//  Function: CylindricalToSphericalPoint
//
//  Purpose:
//      Converts cylindrical coordinates to spherical coordinates.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Oct 17 14:24:01 PDT 2007
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  3 16:41:35 EDT 2009
//    Reversed sense of 2nd and 3rd spherical components to match ISO 31-11.
//    Removed "in2D" setting.
//
// ****************************************************************************

static void 
CylindricalToSphericalPoint(double *newpt, const double *pt)
{
    newpt[2] = pt[1];
    newpt[1] = atan2(pt[0], pt[2]);
    if (newpt[1] < 0.)
        newpt[1] = 2*vtkMath::Pi() + newpt[2];
    newpt[0] = sqrt(pt[0]*pt[0] + pt[2]*pt[2]);
}

// ****************************************************************************
//  Function:  TransformSingleVector
//
//  Purpose:
//    Transform a single vector, given its old value, its 3d location, 
//    and a selection of one of the vector transformation methods.
//
//  Arguments:
//    method     the choice of xform method
//    xf         the coordinate system conversion function
//    newvec     the output vector value
//    vec        the input vector value
//    newpt      the new point the vector will originate from
//    pt         the original point the vector originated from
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  7, 2009
//
// ****************************************************************************

inline static void 
TransformSingleVector(avtCoordSystemConvert::VectorTransformMethod method,
                      xformFunc xf,
                      double *newvec, const double *const vec,
                      const double *const newpt, const double *const pt)
{
    const double instantEps    = 1.e-5;
    const double instantEpsInv = 1.e+5;

    switch (method)
    {
      case avtCoordSystemConvert::None:
        newvec[0] = vec[0];
        newvec[1] = vec[1];
        newvec[2] = vec[2];
        break;
      case avtCoordSystemConvert::AsPoint:
        xf(newvec, vec);
        break;
      case avtCoordSystemConvert::AsDisplacement:
        {
        double tmpvec[3] = {vec[0] + pt[0],
                            vec[1] + pt[1],
                            vec[2] + pt[2]};
        xf(newvec, tmpvec);
        newvec[0] -= newpt[0];
        newvec[1] -= newpt[1];
        newvec[2] -= newpt[2];
        }
        break;
      case avtCoordSystemConvert::AsDirection:
        {
        double tmpvec[3] = {instantEps*vec[0] + pt[0],
                            instantEps*vec[1] + pt[1],
                            instantEps*vec[2] + pt[2]};
        xf(newvec, tmpvec);
        newvec[0] = (newvec[0] - newpt[0]) * instantEpsInv;
        newvec[1] = (newvec[1] - newpt[1]) * instantEpsInv;
        newvec[2] = (newvec[2] - newpt[2]) * instantEpsInv;
        }
        break;
    }
}

// ****************************************************************************
//  Function: Transform
//
//  Purpose:
//      Converts one coordinate system to another.
//
//      More info at: 
//      http://www.geom.uiuc.edu/docs/reference/CRC-formulas/node42.html
//      (For the record, I did this myself and only *confirmed* the formulas at
//      this site.)
//
//  Programmer: Hank Childs
//  Creation:   June 28, 2003
//
//  Modifications:
//    Hank Childs, Tue Nov 15 15:40:04 PST 2005
//    Add support for 2D.  Also re-order coordinates.
//
//    Dave Bremer, Wed Oct 17 14:24:01 PDT 2007
//    Factored core function out, and added transform of vector data.
//
//    Jeremy Meredith, Fri Aug  7 15:37:28 EDT 2009
//    Made this function apply to all types of coordinate transforms
//    to avoid code duplication.  Added four different user-selectable
//    vector transformation methods.  Added vector transform code
//    to this function; since some of the methods require the original
//    and transformed cell/point coordinates, so it has to be done here.
//
// ****************************************************************************

static vtkDataSet *
Transform(vtkDataSet *in_ds,
          avtCoordSystemConvert::VectorTransformMethod vectorTransformMethod,
          xformFunc xf)
{
    int numPts  = in_ds->GetNumberOfPoints();
    int numCells = in_ds->GetNumberOfCells();

    //
    // Transform the points
    //
    vtkPoints *pts = vtkVisItUtility::GetPoints(in_ds);
    vtkPoints *newPts = vtkPoints::New();
    newPts->SetNumberOfPoints(numPts);
    for (int i = 0 ; i < numPts ; i++)
    {
        double pt[3], newpt[3];
        pts->GetPoint(i, pt);
        xf(newpt, pt);
        newPts->SetPoint(i, newpt);
    }
    vtkDataSet *rv = CreateNewDataset(in_ds, newPts);

    //
    // Transform the vectors if necessary
    //
    enum Pass { PointPass, CellPass };
    for (int p = 0; p <= 1; p++)
    {
        Pass pass = Pass(p);

        // Skip out early for no-op
        if (vectorTransformMethod == avtCoordSystemConvert::None)
            continue;

        // Get some values for this pass
        vtkDataSetAttributes *dsatts;
        if (pass==PointPass)
            dsatts = rv->GetPointData();
        else
            dsatts = rv->GetCellData();
        int numTuples = (pass==PointPass) ? numPts : numCells;
        int numArrays = dsatts->GetNumberOfArrays();
        vtkDataArray *activeArr = dsatts->GetVectors();

        // Are we doing a transform that needs point coordinates?
        bool needPointCoordinates =
            (vectorTransformMethod == avtCoordSystemConvert::AsDisplacement ||
             vectorTransformMethod == avtCoordSystemConvert::AsDirection);

        // Point coordinates for the Cell pass means cell centroids
        // Calclate those (original and transformed) here.
        vtkDoubleArray *cellCoordsOld = vtkDoubleArray::New();
        vtkDoubleArray *cellCoordsNew = vtkDoubleArray::New();
        if (pass == CellPass && needPointCoordinates)
        {
            cellCoordsOld->SetNumberOfComponents(3);
            cellCoordsOld->SetNumberOfTuples(numCells);
            cellCoordsNew->SetNumberOfComponents(3);
            cellCoordsNew->SetNumberOfTuples(numCells);
            vtkIdList *ptIds = vtkIdList::New();
            double pt[3]    = {0,0,0};
            double newpt[3] = {0,0,0};
            for (int n = 0 ; n < numCells ; n++)
            {
                in_ds->GetCellPoints(n, ptIds);
                int numCellPts = ptIds->GetNumberOfIds();
                pt[0] = pt[1] = pt[2] = 0.0;
                for (int j = 0; j < numCellPts; j++)
                {
                    double tmppt[3];
                    pts->GetPoint(ptIds->GetId(j), tmppt);
                    pt[0] += tmppt[0];
                    pt[1] += tmppt[1];
                    pt[2] += tmppt[2];
                }
                pt[0] /= double(numCellPts);
                pt[1] /= double(numCellPts);
                pt[2] /= double(numCellPts);
                xf(newpt, pt); // need to transform it, too
                cellCoordsOld->SetTuple(n, pt);
                cellCoordsNew->SetTuple(n, newpt);
            }
            ptIds->Delete();
        }

        //
        // Loop over the arrays
        //
        for (int k = numArrays-1 ; k >= 0 ; k--)
        {
            // Make sure we want to transform this array
            vtkDataArray *arr = dsatts->GetArray(k);
            int ncomp = arr->GetNumberOfComponents();
            if (ncomp != 2 && ncomp != 3)
                continue;
            if (arr->GetName() != NULL)
            {
                std::string name = arr->GetName();
                if (name == "avtOriginalNodeNumbers" ||
                    name == "avtOriginalCellNumbers" ||
                    name == "avtSubsets")
                    continue;
            }

            // Create an output array to fill
            vtkDataArray *arr_new = vtkDataArray::CreateDataArray(arr->GetDataType());
            if (arr->GetName() != NULL)
                arr_new->SetName(arr->GetName());
            arr_new->SetNumberOfComponents(ncomp);
            arr_new->SetNumberOfTuples(numTuples);

            // fill the output array
            for (int n = 0 ; n < numTuples ; n++)
            {
                // Get the old and new point locations.
                double tup[3] = {0,0,0}, newtup[3] = {0,0,0};
                double pt[3]  = {0,0,0}, newpt[3]  = {0,0,0};
                if (needPointCoordinates && pass==PointPass)
                {
                    pts->GetPoint(n, pt);
                    newPts->GetPoint(n, newpt);
                }
                else if (needPointCoordinates && pass==CellPass)
                {
                    cellCoordsOld->GetTuple(n, pt);
                    cellCoordsNew->GetTuple(n, newpt);
                }

                // Get the old vector values.
                arr->GetTuple(n, tup);
                // Transform it.
                TransformSingleVector(vectorTransformMethod, xf,
                                      newtup, tup, newpt, pt);
                // And store it.
                arr_new->SetTuple(n, newtup);
            }

            // Set up the active variables as necessary
            bool isActive = false;
            if (activeArr &&
                activeArr->GetName() != NULL &&
                arr->GetName() != NULL &&
                strcmp(activeArr->GetName(), arr->GetName()) == 0)
                isActive = true;
            if (arr->GetName() != NULL)
                dsatts->RemoveArray(arr->GetName());
            dsatts->AddArray(arr_new);
            arr_new->Delete();
            if (isActive)
                dsatts->SetVectors(arr_new);
        }
        cellCoordsOld->Delete();
        cellCoordsNew->Delete();
    }

    pts->Delete();
    newPts->Delete();

    return rv;
}


// ****************************************************************************
//  Function: FixWraparounds
//
//  Purpose:
//      Locates cells where the dataset has been wrapped-around from 0 radians
//      to 2*pi radians.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2003
//
// ****************************************************************************

static vtkDataSet *
FixWraparounds(vtkDataSet *in_ds, int comp_idx)
{
    int   i, j;

    if (in_ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        in_ds->Register(NULL);
        return in_ds;
    }

    vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) in_ds;
    vtkPoints *pts = ugrid->GetPoints();
    int npts = pts->GetNumberOfPoints();

    vtkPoints *new_pts = vtkPoints::New();
    new_pts->SetNumberOfPoints(2*npts);
    vtkUnstructuredGrid *new_grid = vtkUnstructuredGrid::New();
    new_grid->SetPoints(new_pts);
    vtkPointData *out_pd = new_grid->GetPointData();
    vtkPointData *in_pd  = in_ds->GetPointData();
    out_pd->CopyAllocate(in_pd, 2*npts);
   
    for (i = 0 ; i < npts ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        new_pts->SetPoint(2*i, pt);
        if (pt[comp_idx] > vtkMath::Pi())
            pt[comp_idx] -= 2*vtkMath::Pi();
        else
            pt[comp_idx] += 2*vtkMath::Pi();
        new_pts->SetPoint(2*i+1, pt);
        out_pd->CopyData(in_pd, i, 2*i);
        out_pd->CopyData(in_pd, i, 2*i+1);
    }

    int ncells = ugrid->GetNumberOfCells();
    new_grid->Allocate(2*ncells*8);
    vtkCellData *out_cd = new_grid->GetCellData();
    vtkCellData *in_cd  = in_ds->GetCellData();
    out_cd->CopyAllocate(in_cd, 2*ncells);
   
    float pi = vtkMath::Pi();
    float twoPiCutoff = 2*vtkMath::Pi()*0.95;
    float zeroPiCutoff = vtkMath::Pi()*0.1;
    int cellCnt = 0;
    for (i = 0 ; i < ncells ; i++)
    {
        vtkIdType *ids;
        int cellNPts;
        ugrid->GetCellPoints(i, cellNPts, ids);
        bool closeToZero = false;
        bool closeToTwoPi = false;
        bool closeToLow[8];
        for (j = 0 ; j < cellNPts ; j++)
        {
            double pt[3];
            pts->GetPoint(ids[j], pt);
            if (pt[comp_idx] > twoPiCutoff)
                closeToTwoPi = true;
            if (pt[comp_idx] < zeroPiCutoff)
                closeToZero  = true;
            closeToLow[j] = (pt[comp_idx] < pi ? false : true);
        }
        if (closeToTwoPi && closeToZero)
        {
            // Make two cells -- start with the one close to 0 radians.
            vtkIdType low_ids[8];
            for (j = 0 ; j < cellNPts ; j++)
                low_ids[j] = (closeToLow[j] ? 2*ids[j] : 2*ids[j]+1);
            new_grid->InsertNextCell(ugrid->GetCellType(i), cellNPts, low_ids);
            out_cd->CopyData(in_cd, i, cellCnt++);
            
            vtkIdType hi_ids[8];
            for (j = 0 ; j < cellNPts ; j++)
                hi_ids[j] = (!closeToLow[j] ? 2*ids[j] : 2*ids[j]+1);
            new_grid->InsertNextCell(ugrid->GetCellType(i), cellNPts, hi_ids);
            out_cd->CopyData(in_cd, i, cellCnt++);
        }
        else
        {
            vtkIdType new_ids[8];
            for (j = 0 ; j < cellNPts ; j++)
                new_ids[j] = 2*ids[j];
            new_grid->InsertNextCell(ugrid->GetCellType(i), cellNPts, new_ids);
            out_cd->CopyData(in_cd, i, cellCnt++);
        }
    }
    new_grid->Squeeze();
    new_pts->Delete();

    return new_grid;
}

// ****************************************************************************
//  Method: avtCoordSystemConvert::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the CoordConvert filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   Fri Jun 27 16:41:32 PST 2003
//
//  Modifications:
//
//    Hank Childs, Tue Nov 15 15:40:04 PST 2005
//    Add support for 2D.
//
//    Jeremy Meredith, Fri Aug  7 15:35:29 EDT 2009
//    Call a common transform function to avoid code duplication.
//
// ****************************************************************************

vtkDataSet *
avtCoordSystemConvert::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    vector<vtkDataSet *> deleteList;

    CoordSystem ct_current = inputSys;

    vtkDataSet *cur_ds = in_ds;
    while (ct_current != outputSys)
    {
        switch (ct_current)
        {
          case CARTESIAN:
          {
            vtkDataSet *new_ds = Transform(cur_ds,
                                           vectorTransformMethod,
                                           CartesianToCylindricalPoint);
            deleteList.push_back(new_ds);
            cur_ds = new_ds;
            ct_current = CYLINDRICAL;
            break;
          }
          case CYLINDRICAL:
          {
            vtkDataSet *new_ds = Transform(cur_ds,
                                           vectorTransformMethod,
                                           CylindricalToSphericalPoint);
            deleteList.push_back(new_ds);
            cur_ds = new_ds;
            ct_current = SPHERICAL;
            break;
          }
          case SPHERICAL:
          {
            vtkDataSet *new_ds = Transform(cur_ds,
                                           vectorTransformMethod,
                                           SphericalToCartesianPoint);
            deleteList.push_back(new_ds);
            cur_ds = new_ds;
            ct_current = CARTESIAN;
            break;
          }
        }
    }

    if (outputSys == SPHERICAL)
    {
        cur_ds = FixWraparounds(cur_ds, 1);
        deleteList.push_back(cur_ds);
        cur_ds = FixWraparounds(cur_ds, 2);
        deleteList.push_back(cur_ds);
    }
    else if (outputSys == CYLINDRICAL)
    {
        cur_ds = FixWraparounds(cur_ds, 1);
        deleteList.push_back(cur_ds);
    }

    ManageMemory(cur_ds);

    for (unsigned int i = 0 ; i < deleteList.size() ; i++)
    {
         deleteList[i]->Delete();
    }

    return cur_ds;
}


// ****************************************************************************
//  Method: avtCoordSystemConvert::PostExecute
//
//  Purpose:
//      This is called to set up the output extents.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Jul  5 09:23:13 PDT 2005
//    Properly create extents.
//
// ****************************************************************************
 
void
avtCoordSystemConvert::PostExecute()
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetEffectiveSpatialExtents()->Clear();
    outAtts.GetCurrentSpatialExtents()->Clear();

    double bounds[6];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(ds, bounds);
    outAtts.GetCumulativeTrueSpatialExtents()->Set(bounds);
}


// ****************************************************************************
//  Method: avtCoordSystemConvert::TransformExtents
//
//  Purpose:
//      Transforms a bounding box to get the new extents.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
// ****************************************************************************

void
avtCoordSystemConvert::TransformExtents(double *extents)
{
    //
    // Set up a one cell-ed rectilinear grid based on the bounding box.
    //
    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(10);
    int i;
    for (i = 0 ; i < 10 ; i++)
        x->SetTuple1(i, (extents[1]-extents[0]) * ((float)i)/10. + extents[0]);
 
    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfTuples(10);
    for (i = 0 ; i < 10 ; i++)
        y->SetTuple1(i, (extents[3]-extents[2]) * ((float)i)/10. + extents[2]);
 
    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfTuples(10);
    for (i = 0 ; i < 10 ; i++)
        z->SetTuple1(i, (extents[5]-extents[4]) * ((float)i)/10. + extents[4]);
 
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(10, 10, 10);
    rgrid->SetXCoordinates(x);
    rgrid->SetYCoordinates(y);
    rgrid->SetZCoordinates(z);

    vtkDataSet *rv = ExecuteData(rgrid, -1, "");
    rv->GetBounds(extents);

    x->Delete();
    y->Delete();
    z->Delete();
    rgrid->Delete();
    //rv does not need to be deleted.
    //rv->Delete();
}


// ****************************************************************************
//  Function: CreateNewDataset
//
//  Purpose:
//      Creates a dataset just like the input, except with the new set of
//      points.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
// ****************************************************************************

static vtkDataSet *
CreateNewDataset(vtkDataSet *in_ds, vtkPoints *newPts)
{
    vtkDataSet *rv = NULL;

    int dstype = in_ds->GetDataObjectType();
    if (dstype == VTK_STRUCTURED_GRID || dstype == VTK_POLY_DATA ||
        dstype == VTK_UNSTRUCTURED_GRID)
    {
        vtkPointSet *rv2 = (vtkPointSet *) in_ds->NewInstance();
        rv2->ShallowCopy(in_ds);
        rv2->SetPoints(newPts);

        rv = rv2;
    }
    else if (dstype == VTK_RECTILINEAR_GRID)
    {
        int dims[3];
        vtkRectilinearGrid *rg = (vtkRectilinearGrid *) in_ds;
        rg->GetDimensions(dims);

        vtkStructuredGrid *rv2 = vtkStructuredGrid::New();
        rv2->SetDimensions(dims);
        rv2->GetPointData()->ShallowCopy(in_ds->GetPointData());
        rv2->GetCellData()->ShallowCopy(in_ds->GetCellData());
        rv2->SetPoints(newPts);

        rv = rv2;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtCoordSystemConvert::UpdateDataObjectInfo
//
//  Purpose:
//      Changes the labels of the axes
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2005
//
//  Modifications:
//
//    Hank Childs, Tue Nov 15 15:40:04 PST 2005
//    Re-order coordinates.
//
// ****************************************************************************

void
avtCoordSystemConvert::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    if (inputSys == CARTESIAN)
    {
        if (outputSys == SPHERICAL)
        {
            if (inAtts.GetXLabel() == "X-Axis" ||
                inAtts.GetXLabel() == "X Axis")
                outAtts.SetXLabel("Radius");
            else
                outAtts.SetXLabel(std::string("Radius / ") + 
                                  inAtts.GetXLabel());

            if (inAtts.GetYLabel() == "Y-Axis" ||
                inAtts.GetYLabel() == "Y Axis")
                outAtts.SetYLabel("Theta");
            else
                outAtts.SetYLabel(std::string("Theta / ") + 
                                  inAtts.GetYLabel());

            if (inAtts.GetZLabel() == "Z-Axis" ||
                inAtts.GetZLabel() == "Z Axis")
                outAtts.SetZLabel("Phi");
            else
                outAtts.SetZLabel(std::string("Phi / ") + 
                                  inAtts.GetZLabel());
        }
        else if (outputSys == CYLINDRICAL)
        {
            if (inAtts.GetXLabel() == "X-Axis" ||
                inAtts.GetXLabel() == "X Axis")
                outAtts.SetXLabel("Radius");
            else
                outAtts.SetXLabel(std::string("Radius / ") + 
                                   inAtts.GetXLabel());

            if (inAtts.GetYLabel() == "Y-Axis" ||
                inAtts.GetYLabel() == "Y Axis")
                outAtts.SetYLabel("Theta");
            else
                outAtts.SetYLabel(std::string("Theta / ") + 
                                   inAtts.GetYLabel());

            if (inAtts.GetZLabel() == "Z-Axis" ||
                inAtts.GetZLabel() == "Z Axis")
                outAtts.SetZLabel("Height");
            else
                outAtts.SetZLabel(std::string("Height / ") + 
                                   inAtts.GetZLabel());
        }
    }
    if (outputSys == SPHERICAL)
    {
        outAtts.SetYUnits("radians");
        outAtts.SetZUnits("radians");
    }
    else if (outputSys == CYLINDRICAL)
    {
        outAtts.SetYUnits("radians");
    }
    GetOutput()->GetInfo().GetValidity().SetPointsWereTransformed(true);
}


