// ************************************************************************* //
//  File: avtBoxFilter.C
// ************************************************************************* //

#include <avtBoxFilter.h>

#include <vtkBox.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSetToUnstructuredGridFilter.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

#include <avtExtents.h>

#include <DebugStream.h>

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
    float bounds[6];

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
            float pt[3];
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
            float pt[3];
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

                    float pt1[3];
                    input->GetPoint(ptId, pt1);
                    
                    int k;
                    for (k = j + 1; k < nCellPts; ++k)
                    {
                        float pt2[3];
                        input->GetPoint(cellPts->GetId(k), pt2);
                        // Turn pt2 into a ray
                        float ray[3];
                        ray[0] = pt2[0] - pt1[0];
                        ray[1] = pt2[1] - pt1[1];
                        ray[2] = pt2[2] - pt1[2];
                        
                        float t;
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
                    float xcoords[3];
                    float t;
                    float pcoords[3];
                    int subid;
                    for (j = 0; j < 24; j += 2)
                    {
                        float *pt1 = bxpts[boxLineMap[j]];
                        float *pt2 = bxpts[boxLineMap[j + 1]];
                        if (cell->IntersectWithLine(pt1, pt2, 1e-6, t,
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
                    float pt[3];
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
// ****************************************************************************

avtBoxFilter::avtBoxFilter()
{
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
// ****************************************************************************

vtkDataSet *
avtBoxFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
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
    vtkDataSet *newDS = bf->GetOutput();
    newDS->Update();
    newDS->SetSource(NULL);

    vtkDataSet *outDS = newDS;
    if (newDS->GetNumberOfCells() <= 0)
    {
        outDS = newDS;
    }

    ManageMemory(outDS);
    bf->Delete();

    return outDS;
}


// ****************************************************************************
//  Method: avtBoxFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicate that the effective spatial extents have changed.
//
//  Programmer: Hank Childs
//  Creation:   November 13, 2001
//
// ****************************************************************************

void
avtBoxFilter::RefashionDataObjectInfo(void)
{
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

    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


