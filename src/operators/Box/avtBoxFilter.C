// ************************************************************************* //
//  File: avtBoxFilter.C
// ************************************************************************* //

#include <avtBoxFilter.h>

#include <vtkDataSetToUnstructuredGridFilter.h>
#include <vtkObjectFactory.h>
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
    bool *isInBox = new bool[nPts];
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

        //
        // Determine which points are inside the box and which are outside
        // the box.  Keep track of whether they are all inside and whether
        // at least one is inside.
        //
        bool allInsideCell  = true;
        bool oneInsideCell  = false;
        for (j = 0 ; j < nCellPts ; j++)
        {
            int ptId = cellPts->GetId(j);
            if (isInBox[ptId])
            {
                oneInsideCell = true;
            }
            else
            {
                allInsideCell = false;
            }
        }

        bool meetsCriteria = (oneInsideCell && AllOfCell == 0) ||
                             (allInsideCell && AllOfCell != 0);
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
    delete [] isInBox;
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


