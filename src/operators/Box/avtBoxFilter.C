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
//  File: avtBoxFilter.C
// ************************************************************************* //

#include <avtBoxFilter.h>

#include <vtkBox.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSetToUnstructuredGridFilter.h>
#include <vtkExtractRectilinearGrid.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtExtents.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtSpatialBoxSelection.h>

#include <DebugStream.h>

using std::string;

// Define the VTK filter here.

class vtkBoxFilter : public vtkDataSetToUnstructuredGridFilter
{
public:
  vtkTypeMacro(vtkBoxFilter,vtkDataSetToUnstructuredGridFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkBoxFilter *New();

  vtkSetMacro(MinX, float);
  vtkGetMacro(MinX, float);
  vtkSetMacro(MaxX, float);
  vtkGetMacro(MaxX, float);

  vtkSetMacro(MinY, float);
  vtkGetMacro(MinY, float);
  vtkSetMacro(MaxY, float);
  vtkGetMacro(MaxY, float);

  vtkSetMacro(MinZ, float);
  vtkGetMacro(MinZ, float);
  vtkSetMacro(MaxZ, float);
  vtkGetMacro(MaxZ, float);

  vtkSetMacro(AllOfCell, int);
  vtkGetMacro(AllOfCell, int);
  vtkBooleanMacro(AllOfCell, int);

protected:
  vtkBoxFilter();
  ~vtkBoxFilter();
  vtkBoxFilter(const vtkBoxFilter&) {};
  void operator=(const vtkBoxFilter&) {};

  // Usual data generation method
  void Execute();

  float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
  int   AllOfCell;
};


void
vtkBoxFilter::PrintSelf(ostream &os, vtkIndent indent)
{
  vtkDataSetToUnstructuredGridFilter::PrintSelf(os,indent);
  os << indent << "Box: (" << MinX << ", " << MaxX << "), (" << MinY << ", "
     << MaxY << "), (" << MinZ << ", " << MaxZ << ")" << endl;
  os << indent << "Must have all of cell = " << AllOfCell << endl;
}

vtkBoxFilter *vtkBoxFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkBoxFilter");
  if(ret)
    {
    return (vtkBoxFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkBoxFilter;
}

vtkBoxFilter::vtkBoxFilter()
{
  AllOfCell = 0;
  MinX = MinY = MinZ = 0.;
  MaxX = MaxY = MaxZ = 1.;
}

vtkBoxFilter::~vtkBoxFilter()
{
  ;
}

// ****************************************************************************
// Modifications:
//   Akira Haddox, Fri Aug  8 13:48:22 PDT 2003
//   Rewrote algorithm so that 'SomeOfCell' selection would be accurate.
// ****************************************************************************

void
vtkBoxFilter::Execute(void)
{
    int  i, j;

    //
    // Set up some automatic variables with convenient info about the input.
    //
    vtkDataSet *input = GetInput();
    int nCells = input->GetNumberOfCells();
    int nPts   = input->GetNumberOfPoints();
    vtkPointData *inputPD = input->GetPointData();
    vtkCellData  *inputCD = input->GetCellData();
    int *pointMap = new int[nPts];
    for (i = 0 ; i < nPts ; i++)
    {
        pointMap[i] = -1;
    }
    bool *isInBox;
    char *relativeToBox[3];
    float bxpts[8][3];
    double bounds[6];

    //
    // Represents the connectivity between points that define the box.
    //
    const int boxLineMap[24] = {0, 1,   0, 2,   0, 4,
                                1, 3,   1, 5,
                                2, 3,   2, 6,
                                3, 7,
                                4, 5,   4, 6,
                                5, 7,
                                6, 7 };
                                
    
    if (AllOfCell)
    { 
        isInBox= new bool[nPts];
        for (i = 0 ; i < nPts ; i++)
        {
            double pt[3];
            input->GetPoint(i, pt);
            if (pt[0] >= MinX && pt[0] <= MaxX && 
                pt[1] >= MinY && pt[1] <= MaxY &&
                pt[2] >= MinZ && pt[2] <= MaxZ)
            {
                isInBox[i] = true;
            }
            else
            {
                isInBox[i] = false;
            }
        }
    }
    else
    {
        relativeToBox[0] = new char[nPts];
        relativeToBox[1] = new char[nPts];
        relativeToBox[2] = new char[nPts];

        bounds[0] = MinX;
        bounds[1] = MaxX;
        bounds[2] = MinY;
        bounds[3] = MaxY;
        bounds[4] = MinZ;
        bounds[5] = MaxZ;

        //
        // Fill the bxpts structure so that it holds the points that
        // define the box.
        //
#define SET_FOUR_POINTS(_a,_b,_c,_d, _x, _v)       \
                bxpts[_a][_x] = bxpts[_b][_x] =    \
                bxpts[_c][_x] = bxpts[_d][_x] = _v

        SET_FOUR_POINTS(0, 1, 2, 3,  0, MinX);
        SET_FOUR_POINTS(4, 5, 6, 7,  0, MaxX);
        SET_FOUR_POINTS(0, 1, 4, 5,  1, MinY);
        SET_FOUR_POINTS(2, 3, 6, 7,  1, MaxY);
        SET_FOUR_POINTS(0, 2, 4, 6,  2, MinZ);
        SET_FOUR_POINTS(1, 3, 5, 7,  2, MaxZ);
        
        //
        // For each point, classify as being less than, greater than, or
        // within the box range for each coordinate.
        //
        for (i = 0 ; i < nPts ; i++)
        {
            double pt[3];
            input->GetPoint(i, pt);
            int j;
            for (j = 0; j < 3; ++j)
            {
                if (pt[j] < bounds[2 * j])
                    relativeToBox[j][i] = -1;
                else if (pt[j] > bounds[2 * j + 1])
                    relativeToBox[j][i] = 1;
                else
                    relativeToBox[j][i] = 0;
            }
        } 
    }

  
    //
    // Set up some the VTK vars that will (eventually) be copied over
    // to the output.
    //
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(nPts/4,nPts);
    vtkUnstructuredGrid *output = GetOutput();
    output->Allocate(nCells);
    vtkPointData *outputPD = output->GetPointData();
    vtkCellData  *outputCD = output->GetCellData();
    outputPD->CopyAllocate(inputPD);
    outputCD->CopyAllocate(inputCD);
  
    //
    // Loop over all the cells and see which meet our criteria.  If a cell
    // meet the criteria, add it -- along with its points to our output and
    // the variable information.  Take care not to make more than one copy of
    // the point -- otherwise it would be inefficient and potentially could
    // overload the amount that we have allocated.
    //
    int nextPointIndex = 0;
    vtkIdList *newCellPts = vtkIdList::New();
    newCellPts->Allocate(VTK_CELL_SIZE);

    for (i = 0 ; i < nCells ; i++)
    {
        //
        // Get the cell.
        //
        vtkCell   *cell     = input->GetCell(i);
        vtkIdList *cellPts  = cell->GetPointIds();
        int        nCellPts = cell->GetNumberOfPoints();

        bool meetsCriteria;
        if (AllOfCell)
        {
            // Algorithm for 'AllOfCell' check:
            //   Go through and break out early if a point is outside.
            //   We meet the criteria if we didn't break out early.
            for (j = 0 ; j < nCellPts ; ++j)
            {
                int ptId = cellPts->GetId(j);
                if (!isInBox[ptId])
                    break;
            }
            meetsCriteria = (j == nCellPts);
        }
        else
        {
            // Algorithm for 'SomeOfCell' check:
            //   Go through mapping where the cell seems to be relative
            //   to the box.
            //     If we find a point in the box, break out early.
            //     Make notes of if the cell possibly spans the box.     
            //   If we broke out early, we meet the critera and we're done.
            //   If we didn't find any consistancies, we don't meet the
            //    criteria, and we're done.
            //   Otherwise, do more checks to confirm whether or not the
            //    cell is inside or outside of the box.

            bool possiblySpanning[3];
            possiblySpanning[0] = false;
            possiblySpanning[1] = false;
            possiblySpanning[2] = false;

            int fId = cellPts->GetId(0);
            for (j = 0; j < nCellPts; ++j)
            {
                int ptId = cellPts->GetId(j);
                if (!relativeToBox[0][ptId] && !relativeToBox[1][ptId] 
                                            && !relativeToBox[2][ptId])
                    break;
                int k;
                for (k = 0; k < 3; ++k)
                {
                    //
                    // To be possibly spanning in a coordinate means
                    // that either (a) the component is within the box,
                    // or that it is inconsistant with another node
                    // (in this case, the first node).
                    //
                    if (!relativeToBox[k][ptId])
                        possiblySpanning[k] = true;
                    else if (relativeToBox[k][ptId] != relativeToBox[k][fId])
                        possiblySpanning[k] = true;
                }
            }

            if (j != nCellPts)
                meetsCriteria = true;
            else if (!possiblySpanning[0] || !possiblySpanning[1]
                                          || !possiblySpanning[2])
                meetsCriteria = false;
            else
            {
                meetsCriteria = false;
                //
                // It's possible that part of this cell is contained within
                // the box, even though no vertices are within it.
                //

                //
                // We need to look at lines between vertices. We search
                // from the first vertice to all the other (since we
                // used it as a comparison node), then we search from
                // any nodes that have '0' for a relative box position.
                //
                  
                for (j = 0; j < nCellPts && !meetsCriteria; ++j)
                {
                    int ptId = cellPts->GetId(j);
                    if (j != 0 && relativeToBox[0][ptId] 
                               && relativeToBox[1][ptId]
                               && relativeToBox[2][ptId])
                    {
                        continue;
                    }

                    double pt1[3];
                    input->GetPoint(ptId, pt1);
                    
                    int k;
                    for (k = j + 1; k < nCellPts; ++k)
                    {
                        double pt2[3];
                        input->GetPoint(cellPts->GetId(k), pt2);
                        // Turn pt2 into a ray
                        double ray[3];
                        ray[0] = pt2[0] - pt1[0];
                        ray[1] = pt2[1] - pt1[1];
                        ray[2] = pt2[2] - pt1[2];
                        
                        double t;
                        if (vtkBox::IntersectBox(bounds, pt1, ray, pt2, t)
                                && t < 1)
                        {
                            meetsCriteria = true;
                            break;
                        }
                        
                    }
                }

                if (!meetsCriteria)
                {
                    //
                    // It's still possible that part of the cell is in the
                    // box. In these cases, we check to see if the lines of
                    // the box intersect the cell.
                    //
                    double xcoords[3];
                    double t;
                    double pcoords[3];
                    double dpt1[3], dpt2[3];
                    int subid;
                    for (j = 0; j < 24; j += 2)
                    {
                        float *pt1 = bxpts[boxLineMap[j]];
                        float *pt2 = bxpts[boxLineMap[j + 1]];
                        dpt1[0] = pt1[0];
                        dpt1[1] = pt1[1];
                        dpt1[2] = pt1[2];
                        dpt2[0] = pt2[0];
                        dpt2[1] = pt2[1];
                        dpt2[2] = pt2[2];
                        if (cell->IntersectWithLine(dpt1, dpt2, 1e-6, t,
                                                    xcoords, pcoords, subid))
                        {
                            meetsCriteria = true;
                            break;
                        }
                    }
                }
               
           }
        }

        if (meetsCriteria)
        {
            //
            // We must re-map the point ids from the input mesh to the
            // point ids of the output mesh.  This piece is in here to insure
            // that we don't make multiple copies of points.
            //
            newCellPts->Reset();
            for (j = 0 ; j < nCellPts ; j++)
            {
                int ptId = cellPts->GetId(j);
                if (pointMap[ptId] == -1)
                {
                    pointMap[ptId] = nextPointIndex;
                    double pt[3];
                    input->GetPoint(ptId, pt);
                    pts->InsertNextPoint(pt);
                    outputPD->CopyData(inputPD, ptId, nextPointIndex);
                    nextPointIndex++;
                }
                newCellPts->InsertId(j, pointMap[ptId]);
            }

            //
            // Now add the cell to our output mesh.
            //
            int cellType  = cell->GetCellType();
            int newCellId = output->InsertNextCell(cellType, newCellPts);
            outputCD->CopyData(inputCD, i, newCellId);
        }
    }

    //
    // Prevent leaks and set up the output.
    //
    delete [] pointMap;
    if (AllOfCell)
        delete [] isInBox;
    else
    {
        delete [] relativeToBox[0];
        delete [] relativeToBox[1];
        delete [] relativeToBox[2];
    }
    newCellPts->Delete();
    output->SetPoints(pts);
    pts->Delete();
    output->Squeeze(); // Squeeze dataset will squeeze points, vars too.
}


// ****************************************************************************
//  Method: avtBoxFilter constructor
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Mon Nov 12 16:57:31 PST 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selection id
//
// ****************************************************************************

avtBoxFilter::avtBoxFilter()
{
    selID = -1;
}


// ****************************************************************************
//  Method: avtBoxFilter destructor
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Mon Nov 12 16:57:31 PST 2001
//
//  Modifications:
//
// ****************************************************************************

avtBoxFilter::~avtBoxFilter()
{
}


// ****************************************************************************
//  Method:  avtBoxFilter::Create
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Mon Nov 12 16:57:31 PST 2001
//
// ****************************************************************************

avtFilter *
avtBoxFilter::Create()
{
    return new avtBoxFilter();
}


// ****************************************************************************
//  Method:      avtBoxFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Mon Nov 12 16:57:31 PST 2001
//
// ****************************************************************************

void
avtBoxFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const BoxAttributes*)a;
}


// ****************************************************************************
//  Method: avtBoxFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtBoxFilter with the given
//      parameters would result in an equivalent avtBoxFilter.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Mon Nov 12 16:57:31 PST 2001
//
// ****************************************************************************

bool
avtBoxFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(BoxAttributes*)a);
}


// ****************************************************************************
//  Method: avtBoxFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Box filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Mon Nov 12 16:57:31 PST 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 10 16:38:01 PDT 2002
//    Better memory management.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added code to bypass the operator if a plugin has applied the selection
//
//    Hank Childs, Sun Apr 24 11:11:46 PDT 2005
//    Add special support for rectilinear grids.
//
// ****************************************************************************

vtkDataSet *
avtBoxFilter::ExecuteData(vtkDataSet *in_ds, int, string)
{
    //
    // If the selection this filter exists to create has already been handled,
    // then we can skip execution
    //
    if (GetInput()->GetInfo().GetAttributes().GetSelectionApplied(selID))
    {
        debug1 << "Bypassing Box operator because database plugin "
                  "claims to have applied the selection already" << endl;
        return in_ds;
    }

    vtkDataSet *outDS = NULL;
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int dims[3] = {0, 0, 0};
        ((vtkRectilinearGrid*)in_ds)->GetDimensions(dims);
        if (dims[1] <= 1 && dims[2] <= 1 &&
          GetInput()->GetInfo().GetAttributes().GetVariableType() == AVT_CURVE)
        {
            outDS = CurveExecute((vtkRectilinearGrid *) in_ds);
        }
        else
        {
            outDS = RectilinearExecute((vtkRectilinearGrid *) in_ds);
        }
    }
    else 
    {
        outDS = GeneralExecute(in_ds);
    }

    vtkDataSet *rv = outDS;
    if (outDS != NULL)
    {
        if (outDS->GetNumberOfCells() <= 0)
            rv = NULL;
        else
            ManageMemory(outDS);
        outDS->Delete();  // We have to remove the extra reference.
    }

    return rv;
}


// ****************************************************************************
//  Method: avtBoxFilter::GeneralExecute
//
//  Purpose:
//      Intersects the box using the vtkBoxFilter.  Works on any grid type.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2005
// 
//  Modifications:
//    Kathleen Bonnell, Wed May 17 10:46:58 PDT 2006
//    Removed call to SetSource(NULL) as it now removes information necessary
//    for the dataset.
//
// ****************************************************************************

vtkUnstructuredGrid *
avtBoxFilter::GeneralExecute(vtkDataSet *in_ds)
{
    vtkBoxFilter *bf = vtkBoxFilter::New();
    bf->SetInput(in_ds);

    //
    // Make the filter appropriate for our attributes.
    //
    if (atts.GetAmount() == BoxAttributes::Some)
    {
        bf->AllOfCellOff();
    }
    else
    {
        bf->AllOfCellOn();
    }
    bf->SetMinX(atts.GetMinx());
    bf->SetMaxX(atts.GetMaxx());
    bf->SetMinY(atts.GetMiny());
    bf->SetMaxY(atts.GetMaxy());
    bf->SetMinZ(atts.GetMinz());
    bf->SetMaxZ(atts.GetMaxz());

    //
    // Make this a dataset we can return even after we have freed memory.
    //
    vtkUnstructuredGrid *newDS = bf->GetOutput();
    newDS->Update();
    newDS->Register(NULL);
    //newDS->SetSource(NULL);
    bf->Delete();
    return newDS;

}


// ****************************************************************************
//  Method: avtBoxFilter::RectilinearExecute
//
//  Purpose:
//      Does a quick intersection using rectilinear properties of the mesh.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2005
//
//  Modifications:
//    Eric Brugger, Tue Apr 26 08:10:10 PDT 2005
//    I added code to properly handle the case where the number of
//    coordinates was one.  I also fixed the logic to properly handle
//    the case where the end of the range corresponded to a coordinate
//    value and the amount of cell in range flag was some.
//
//    Kathleen Bonnell, Wed May 17 10:46:58 PDT 2006
//    Removed call to SetSource(NULL) as it now removes information necessary
//    for the dataset.
//
// ****************************************************************************

static bool
GetRange(vtkDataArray *c, float Rmin, float Rmax, bool needAll,
         int &min, int &max)
{
    int i;
    int nC = c->GetNumberOfTuples();

    float cMin = c->GetTuple1(0);
    float cMax = c->GetTuple1(nC-1);

    if (cMin > Rmax)
        return false;
    if (cMax < Rmin)
        return false;

    //
    // Handle special case where the number of coordinates is one.
    //
    if (nC == 1)
    {
       min = 0;
       max = 0;
       return true;
    }

    //
    // Handle the general case.
    //
    bool setMin = false;
    bool setMax = false;
    for (i = 0 ; i < nC-1 ; i++)
    {
        float zMin = c->GetTuple1(i);
        float zMax = c->GetTuple1(i+1);
        bool allZoneInRange = (zMin >= Rmin && zMax <= Rmax);
        bool partZoneInRange = (zMax >= Rmin && zMin <= Rmax);
        bool zoneInRange = (needAll ? allZoneInRange : partZoneInRange);
        if (zoneInRange && !setMin)
        {
            min = i;
            setMin = true;
        }
        if (setMin && !setMax && !zoneInRange)
        {
            max = i;
            setMax = true;
        }
    }
    if (setMin && !setMax)
    {
        max = nC-1;
        setMax = true;
    }

    if (!setMin || !setMax)
        return false;

    return true;
}

vtkRectilinearGrid *
avtBoxFilter::RectilinearExecute(vtkRectilinearGrid *in_ds)
{
    bool needAll = (atts.GetAmount() == BoxAttributes::All);

    float minX = atts.GetMinx();
    float maxX = atts.GetMaxx();
    vtkDataArray *x = in_ds->GetXCoordinates();
    int firstCellX = -1, lastCellX = -1;
    if (!GetRange(x, minX, maxX, needAll, firstCellX, lastCellX))
        return NULL;

    float minY = atts.GetMiny();
    float maxY = atts.GetMaxy();
    vtkDataArray *y = in_ds->GetYCoordinates();
    int firstCellY = -1, lastCellY = -1;
    if (!GetRange(y, minY, maxY, needAll, firstCellY, lastCellY))
        return NULL;

    float minZ = atts.GetMinz();
    float maxZ = atts.GetMaxz();
    vtkDataArray *z = in_ds->GetZCoordinates();
    int firstCellZ = -1, lastCellZ = -1;
    if (!GetRange(z, minZ, maxZ, needAll, firstCellZ, lastCellZ))
        return NULL;

    vtkExtractRectilinearGrid *extract = vtkExtractRectilinearGrid::New();
    int voi[6] = {firstCellX, lastCellX, firstCellY, lastCellY,
                  firstCellZ, lastCellZ };
    extract->SetInput(in_ds);
    extract->SetVOI(voi);
    extract->Update();

    vtkRectilinearGrid *rv = extract->GetOutput();
    rv->Register(NULL);
    //rv->SetSource(NULL);
    extract->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtBoxFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that the effective spatial extents have changed.
//
//  Programmer: Hank Childs
//  Creation:   November 13, 2001
//
//  Modifications:
//    Kathleen Bonnell,
//    Ensure that Zones and Nodes are invalidated, regardless of dimension.
// 
// ****************************************************************************

void
avtBoxFilter::UpdateDataObjectInfo(void)
{
    // Zone and node numberings will change
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateNodes();

    avtDataAttributes &a = GetOutput()->GetInfo().GetAttributes();
    avtExtents *exts = a.GetEffectiveSpatialExtents();
    if (exts->GetDimension() != 3)
    {
        debug1 << "avtBoxFilter applied to data object with dimension "
               << exts->GetDimension() << endl;
        debug1 << "Not bothering with extents work." << endl;
        return;
    }
    double bounds[6];
    bounds[0] = atts.GetMinx();
    bounds[1] = atts.GetMaxx();
    bounds[2] = atts.GetMiny();
    bounds[3] = atts.GetMaxy();
    bounds[4] = atts.GetMinz();
    bounds[5] = atts.GetMaxz();
    exts->Set(bounds);
}

// ****************************************************************************
//  Method: avtBoxFilter::ModifyContract
//
//  Purpose:
//      Restricts the SIL to the domains requested by the user.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 13:40:39 PDT 2005
//    Add support for interval trees.
//
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
//    Hank Childs, Tue Dec  4 13:06:42 PST 2007
//    Do not use an interval tree if the spatial meta data has been invalidated
//
// ****************************************************************************

avtContract_p
avtBoxFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);

    //
    // First tell the file format reader that we are going to be doing a box,
    // in case it can pull out exactly the zones that are within the box.
    //
    avtSpatialBoxSelection *sel = new avtSpatialBoxSelection;
    if (atts.GetAmount() == BoxAttributes::Some)
        sel->SetInclusionMode(avtSpatialBoxSelection::Partial);
    else
        sel->SetInclusionMode(avtSpatialBoxSelection::Whole);
    double mins[3] = {atts.GetMinx(), atts.GetMiny(), atts.GetMinz()};
    double maxs[3] = {atts.GetMaxx(), atts.GetMaxy(), atts.GetMaxz()};
    sel->SetMins(mins);
    sel->SetMaxs(maxs);
    selID = rv->GetDataRequest()->AddDataSelection(sel);

    //
    // Now, if the file format reader has produced an interval tree, determine
    // which domains fall within the box and make sure we only read in those.
    //
    avtIntervalTree *it = NULL;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
        it = GetMetaData()->GetSpatialExtents();

    if (it != NULL)
    {
        vector<int> dl;
        it->GetElementsListFromRange(mins, maxs, dl);
        rv->GetDataRequest()->GetRestriction()->RestrictDomains(dl);
    }

    return rv;
}

// ****************************************************************************
//  Method: avtBoxFilter::CurveExecute
//
//  Purpose:
//    Execution for curves.  Creates multiple discontinuous curves if the box 
//    cuts off original curve.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 1, 2008
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:00:00 PDT 2008
//    Changed creation of curves.
//
//    Kathleen Bonnell, Mon Mar 23 15:38:23 PDT 2009
//    Modified code to create single curve.
//
// ****************************************************************************

vtkRectilinearGrid *
avtBoxFilter::CurveExecute(vtkRectilinearGrid *in_ds)
{
    bool needAll = (atts.GetAmount() == BoxAttributes::All);
    float minX = atts.GetMinx();
    float maxX = atts.GetMaxx();
    vtkDataArray *x = in_ds->GetXCoordinates();
    int firstCellX = -1, lastCellX = -1;
    if (!GetRange(x, minX, maxX, needAll, firstCellX, lastCellX))
        return NULL;

    float minY = atts.GetMiny();
    float maxY = atts.GetMaxy();

    vtkDataArray *y = in_ds->GetPointData()->GetScalars();
    int nPts = lastCellX - firstCellX + 1;


    vtkDataArray *xc = x->NewInstance();
    xc->SetNumberOfComponents(1);
    xc->SetNumberOfTuples(nPts);

    vtkDataArray *yv = y->NewInstance();
    yv->SetNumberOfComponents(1);
    yv->SetNumberOfTuples(nPts);
    yv->SetName(y->GetName());

    int count = 0;
    for (int i = firstCellX; i <= lastCellX; ++i)
    {
        float val = (float)y->GetComponent(i, 0);
        if (val >= minY && val <= maxY)
        {
            xc->SetTuple1(count, x->GetTuple1(i));
            yv->SetTuple1(count, y->GetTuple1(i));
            count++;
        }
    }
    xc->Resize(count);
    yv->Resize(count);

    vtkDataArray *yz = vtkDataArray::CreateDataArray(x->GetDataType());
    yz->SetNumberOfComponents(1);
    yz->SetNumberOfTuples(1);
    yz->SetTuple1(0, 0.);

    vtkRectilinearGrid  *curve = vtkRectilinearGrid::New();
    curve->SetDimensions(count, 1,1 );
    curve->SetXCoordinates(xc);
    curve->SetYCoordinates(yz);
    curve->SetZCoordinates(yz);
    curve->GetPointData()->SetScalars(yv);
    xc->Delete();
    yz->Delete();
    yv->Delete();
    curve->Register(NULL);
    return curve;
}

