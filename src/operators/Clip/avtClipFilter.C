// ************************************************************************* //
//                             avtClipFilter.C                              //
// ************************************************************************* //

#include <avtClipFilter.h>

#include <vtkClipDataSet.h>
#include <vtkClipVolume.h>
#include <vtkClipPolyData.h>
#include <vtkDataSet.h>
#include <vtkImplicitBoolean.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSphere.h>
#include <vtkStructuredPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <BadVectorException.h>
#include <TimingsManager.h>


// ****************************************************************************
//  Class: vtkOrderPreservingPointLocator
// 
//  Purpose:
//      The VTK clipping code creates a new point set to operate on.  With this
//      module, we are separating out the cells that are not clipped, but they
//      are still indexed by the original point list.  So we need the clipped
//      points to also be indexed by the original point list (with the new
//      points added on to the end).  This "locator" is used by the VTK 
//      clipping code.  It overloads the correct virtual function calls to 
//      guarantee that the original points take up the first "n" positions in
//      the new point list.
//
// ****************************************************************************

class vtkOrderPreservingPointLocator : public vtkMergePoints
{
  public:
    static vtkOrderPreservingPointLocator *New();
    
    virtual int InitPointInsertion(vtkPoints *, const float bounds[6]);

    void RegisterPoints(vtkPoints *);

  protected:
    vtkOrderPreservingPointLocator();
    ~vtkOrderPreservingPointLocator();
    vtkPoints *RegisteredPoints;
private:
    // Not implemented.
    vtkOrderPreservingPointLocator(const vtkOrderPreservingPointLocator&);  
    void operator=(const vtkOrderPreservingPointLocator&); 
};


vtkStandardNewMacro(vtkOrderPreservingPointLocator);


vtkOrderPreservingPointLocator::vtkOrderPreservingPointLocator()
{
    this->RegisteredPoints = NULL;
}


vtkOrderPreservingPointLocator::~vtkOrderPreservingPointLocator()
{
    if (this->RegisteredPoints != NULL)
    {
        this->RegisteredPoints->Delete();
        this->RegisteredPoints = NULL;
    }
}

  
void
vtkOrderPreservingPointLocator::RegisterPoints(vtkPoints *pts)
{
    if (this->RegisteredPoints != NULL)
    {
        this->RegisteredPoints->Delete();
        this->RegisteredPoints = NULL;
    }

    this->RegisteredPoints = pts;
    this->RegisteredPoints->Register(this);
}


int
vtkOrderPreservingPointLocator::InitPointInsertion(vtkPoints *p, 
                                                   const float bounds[6])
{
    //
    // This whole class is a sham, so pass the function call down to the base
    // class which actually does something.
    //
    int rv = vtkMergePoints::InitPointInsertion(p, bounds);

    //
    // Now put all of the points from the original dataset into this data
    // structure.  If we simply call InsertNextPoint, the point will not get
    // put in the "bucketing" structure.  So call InsertUniquePoint.  If it
    // is not unique, then add it explicitly, so the ordering will be
    // preserved.
    //
    int p1 = visitTimer->StartTimer();
    int numPts = RegisteredPoints->GetNumberOfPoints();
    for (int i = 0 ; i < numPts ; i++)
    {
        float *pt = RegisteredPoints->GetPoint(i);
        vtkIdType t;
        if (!InsertUniquePoint(pt, t))
        {
            //
            // We need that point in the list to maintain the original
            // ordering of the points.
            //
            InsertNextPoint(pt);
        }
    }
    visitTimer->StopTimer(p1, "Inserting points");

    return rv;
}


// ****************************************************************************
//  Method: avtClipFilter constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001
//    Removed vtkGeometryFilter.
//
//    Hank Childs, Sun Aug 18 09:45:58 PDT 2002
//    Initialized some new data members for tracking connectivity.
//
// ****************************************************************************

avtClipFilter::avtClipFilter()
{
    clipData = vtkClipDataSet::New();
    clipPoly = vtkClipPolyData::New();
    clipVolume = vtkClipVolume::New();
    needsValidFaceConnectivity = false;
    subdivisionOccurred = false;
}


// ****************************************************************************
//  Method: avtClipFilter destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001
//    Removed vtkGeometryFilter.
//
// ****************************************************************************

avtClipFilter::~avtClipFilter()
{
    if (clipData != NULL)
    {
        clipData->Delete();
        clipData = NULL;
    }
    if (clipPoly != NULL)
    {
        clipPoly->Delete();
        clipPoly = NULL;
    }
    if (clipVolume != NULL)
    {
        clipVolume->Delete();
        clipVolume = NULL;
    }
}


// ****************************************************************************
//  Method:  avtClipFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    May  7, 2001
//
// ****************************************************************************

avtFilter *
avtClipFilter::Create()
{
    return new avtClipFilter();
}


// ****************************************************************************
//  Method:      avtClipFilter::SetAtts
//
//  Purpose:
//      Sets the attributes for the Clip operator.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
//  Modifications:
//    Kathleen Bonnell, Tue May 20 16:02:52 PDT 2003 
//    Added tests for valid normals.
//
// ****************************************************************************

void
avtClipFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ClipAttributes*)a;
    double *d; 
    if (atts.GetPlane1Status())
    {
        d = atts.GetPlane1Normal();
        if (d[0] == 0. && d[1] == 0. && d[2] == 0.)
        {  
            EXCEPTION1(BadVectorException, "Normal");
            return;
        }  
    }
    if (atts.GetPlane2Status())
    {
        d = atts.GetPlane2Normal();
        if (d[0] == 0. && d[1] == 0. && d[2] == 0.)
        {  
            EXCEPTION1(BadVectorException, "Normal");
            return;
        }  
    }
    if (atts.GetPlane3Status())
    {
        d = atts.GetPlane3Normal();
        if (d[0] == 0. && d[1] == 0. && d[2] == 0.)
        {  
            EXCEPTION1(BadVectorException, "Normal");
            return;
        }  
    }
}


// ****************************************************************************
//  Method: avtClipFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtClipFilter with the given
//      parameters would result in an equivalent avtClipFilter.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
// ****************************************************************************

bool
avtClipFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ClipAttributes*)a);
}


// ****************************************************************************
//  Method: avtClipFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Clip filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      dom        The domain number.
//      label      The label.
//
//  Returns:       The output unstructured grid.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 7, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001
//    Removed use of vtkGeometryFilter.
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added string argument, to be passed on to output. 
//
//    Hank Childs, Thu Aug 15 21:23:13 PDT 2002
//    Renamed function since this no longer inherits from data tree streamer.
//    Also called new ClipData method which should improve performance.
//
// ****************************************************************************

vtkDataSet *
avtClipFilter::ExecuteData(vtkDataSet *inDS, int dom, std::string)
{
    vtkImplicitBoolean *ifuncs = vtkImplicitBoolean::New();
 
    bool inverse = false; 
    bool funcSet = SetUpClipFunctions(ifuncs, inverse);
    if (!funcSet)
    {
        // we have no functions to work with!
        return inDS;
    }

    //
    // Set up and apply the clipping filters
    // 
    vtkDataSet *outDS = NULL;

    if (inDS->GetDataObjectType() == VTK_STRUCTURED_POINTS &&
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension() ==3)
    {
        clipVolume->SetInput((vtkStructuredPoints*)inDS);
        outDS = vtkUnstructuredGrid::New();
        clipVolume->SetOutput((vtkUnstructuredGrid*)outDS);
        clipVolume->SetClipFunction(ifuncs);
        clipVolume->GenerateClipScalarsOff();
        if (inverse)
        {
            clipVolume->InsideOutOn();
        }
        else 
        {
            clipVolume->InsideOutOff();
        }
        clipVolume->Update();
    }
    else if (inDS->GetDataObjectType() == VTK_POLY_DATA)
    {
        outDS = vtkPolyData::New();
        clipPoly->SetInput((vtkPolyData*)inDS);
        clipPoly->SetOutput((vtkPolyData*)outDS);
        clipPoly->SetClipFunction(ifuncs);
        clipPoly->GenerateClipScalarsOff();
        if (inverse)
        {
            clipPoly->InsideOutOn();
        }
        else 
        {
            clipPoly->InsideOutOff();
        }
        clipPoly->Update();
    }
    else 
    {
        outDS = ClipData(inDS, ifuncs, inverse);
    }

    ifuncs->Delete();

    if (outDS != NULL)
    {
        if (outDS->GetNumberOfCells() == 0)
        {
            return NULL;
        }
        debug5 << "After clipping, domain " << dom << " has " 
               << outDS->GetNumberOfCells() << " cells." << endl;

        ManageMemory(outDS);
        outDS->Delete();
    }

    return outDS;
}


// ****************************************************************************
//  Method: avtClipFilter::ClipData
//
//  Purpose:
//      A routine that clips 3D data using VTK routines.  It makes a decision
//      about whether it can use a fast a routine or whether it must
//      tetrahedralize every cell.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2002
//
// ****************************************************************************

vtkDataSet *
avtClipFilter::ClipData(vtkDataSet *inDS, vtkImplicitFunction *ifuncs,
                         bool inverse)
{
    vtkDataSet *rv = NULL;
    if (needsValidFaceConnectivity)
    {
        clipData->SetInput(inDS);
        clipData->SetClipFunction(ifuncs);
        if (inverse)
        {
            clipData->InsideOutOn();
        }
        else 
        {
            clipData->InsideOutOff();
        }
        clipData->GenerateClipScalarsOff();
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        clipData->SetOutput(ugrid);
        clipData->Update();
        rv = ugrid;
    }
    else
    {
        rv = FastClipData(inDS, ifuncs, inverse);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtClipFilter::FastClipData
//
//  Purpose:
//      A more efficient version of clip data that separates out the zones that
//      are completely within the structure and passes them straight through.
//      This depends on a facelist filter that can match up zones that have
//      faces that match up triangles and quads.
//
//  Programmer: Hank Childs
//  Creation:   August 18, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 21:42:31 PDT 2002
//    Clean up memory management for early return case.
//
// ****************************************************************************

vtkDataSet *
avtClipFilter::FastClipData(vtkDataSet *inDS, vtkImplicitFunction *ifuncs,
                             bool inverse)
{
    int   i, j;

    vtkPoints *pts = vtkVisItUtility::GetPoints(inDS);

    //
    // Classify each of the points ourselves to avoid redundant work.
    //
    int p2 = visitTimer->StartTimer();
    int numPts = pts->GetNumberOfPoints();
    int numCells = inDS->GetNumberOfCells();
    float *ptVal = new float[numPts];
    float *pts_ptr = (float *) pts->GetVoidPointer(0);
    bool all_above = true;
    bool all_below = true;
    for (i = 0 ; i < numPts ; i++)
    {
        ptVal[i] = ifuncs->FunctionValue(pts_ptr);
        if (ptVal[i] == 0.)
        {
            all_above = false;
            all_below = false;
        }
        else if (ptVal[i] > 0.)
        {
            all_below = false;
        }
        else
        {
            all_above = false;
        }
        pts_ptr += 3;
    }
    visitTimer->StopTimer(p2, "Classifying points");

    //
    // If the whole dataset is outside the clip region, then we don't have
    // to examine the cells on a case by case basis -- either we want the
    // whole domain or we don't.
    //
    if (all_above)
    {
        pts->Delete();
        inDS->Register(NULL); // The calling function assumes it owns this obj.
        return (!inverse ? inDS : NULL);
    }
    if (all_below)
    {
        pts->Delete();
        inDS->Register(NULL); // The calling function assumes it owns this obj.
        return (inverse ? inDS : NULL);
    }

    vtkUnstructuredGrid *outDS = vtkUnstructuredGrid::New();

    //
    // Now go through the cells and decide if we need to clip it or not.
    // If so, add it to the "toBeClipped" ugrid.  Otherwise add it to the
    // "fineAsIs" ugrid.
    //
    int p3 = visitTimer->StartTimer();
    vtkUnstructuredGrid *toBeClipped = vtkUnstructuredGrid::New();
    toBeClipped->SetPoints(pts);
    toBeClipped->GetPointData()->PassData(inDS->GetPointData());
    toBeClipped->GetCellData()->CopyAllocate(inDS->GetCellData(), numCells);
    toBeClipped->Allocate(numCells);

    vtkUnstructuredGrid *fineAsIs    = vtkUnstructuredGrid::New();
    fineAsIs->SetPoints(pts);
    fineAsIs->GetPointData()->PassData(inDS->GetPointData());
    fineAsIs->GetCellData()->CopyAllocate(inDS->GetCellData(), numCells);
    fineAsIs->Allocate(numCells);

    vtkCellData *inCD = inDS->GetCellData();
    vtkCellData *faiCD = fineAsIs->GetCellData();
    vtkCellData *tbcCD = toBeClipped->GetCellData();
    int  faiCount = 0;
    int  tbcCount = 0;
    int  notCount = 0;
    for (i = 0 ; i < numCells ; i++)
    {
        vtkCell *cell = inDS->GetCell(i);
        vtkIdList *id_list = cell->GetPointIds();
        vtkIdType *ids = id_list->GetPointer(0);
        vtkIdType nIds = id_list->GetNumberOfIds();

        bool below = false;
        bool above = false;
        for (j = 0 ; j < nIds ; j++)
        {
            if (ptVal[ids[j]] < 0.)
            {
                below = true;
            }
            else
            {
                above = true;
            }
        }
        if (above && below)
        {
            toBeClipped->InsertNextCell(inDS->GetCellType(i), id_list);
            tbcCD->CopyData(inCD, i, tbcCount);
            tbcCount++;
        }
        else if (below && inverse)
        {
            fineAsIs->InsertNextCell(inDS->GetCellType(i), id_list);
            faiCD->CopyData(inCD, i, faiCount);
            faiCount++;
        }
        else if (above && !inverse)
        {
            fineAsIs->InsertNextCell(inDS->GetCellType(i), id_list);
            faiCD->CopyData(inCD, i, faiCount);
            faiCount++;
        }
        else
        {
            notCount++;
        }
    }

    debug5 << "Decided to clip " << tbcCount << " leave whole " << faiCount
           << " and throw out " << notCount << " cells." << endl;
    fineAsIs->Squeeze();
    toBeClipped->Squeeze();
    visitTimer->StopTimer(p3, "Classifying cells");

    //
    // Clip the portion that needs clipping.
    //
    int p4 = visitTimer->StartTimer();
    vtkOrderPreservingPointLocator *loc =vtkOrderPreservingPointLocator::New();
    loc->RegisterPoints(pts);
    clipData->SetLocator(loc);
    clipData->SetInput(toBeClipped);
    clipData->SetClipFunction(ifuncs);
    if (inverse)
    {
        clipData->InsideOutOn();
    }
    else 
    {
        clipData->InsideOutOff();
    }
    clipData->GenerateClipScalarsOff();
    vtkUnstructuredGrid *tmp = vtkUnstructuredGrid::New();
    clipData->SetOutput(tmp);
    clipData->Update();

    //
    // We created our own locator to make sure that all of our points got
    // copied across (this was a VTK hack -- there is more information with
    // the class description of vtkOrderPreservingPointLocator), but that 
    // didn't copy over the point data.  Do that now.
    //
    int p6 = visitTimer->StartTimer();
    vtkPointData *tmpPD = tmp->GetPointData();
    vtkPointData *tbcPD = toBeClipped->GetPointData();
    int np = pts->GetNumberOfPoints();
    for (i = 0 ; i < np ; i++)
    {
        tmpPD->CopyData(tbcPD, i, i);
    }
    visitTimer->StopTimer(p6, "Copying over point data");
    visitTimer->StopTimer(p4, "Clipping cells on boundary");

    //
    // We now need to unify our two dataset back into one.  The faces may not
    // match up, but we have taken care to ensure that the points do.
    //
    if (clipData->GetOutput()->GetNumberOfCells() > 0)
    {
        int p5 = visitTimer->StartTimer();
        vtkUnstructuredGrid *clipped = clipData->GetOutput();
        int numClippedCells = clipped->GetNumberOfCells();
        int numCleanCells   = fineAsIs->GetNumberOfCells();
        int numCells = numClippedCells + numCleanCells;
        outDS->Allocate(numCells);
        outDS->SetPoints(clipped->GetPoints());
        outDS->GetPointData()->PassData(clipped->GetPointData());
        vtkCellData *outCD = outDS->GetCellData();
        outCD->CopyAllocate(clipped->GetCellData(), numCells);
        
        vtkCellData *newCD = clipped->GetCellData();
        for (i = 0 ; i < numClippedCells ; i++)
        {
            vtkCell *cell = clipped->GetCell(i);
            vtkIdList *id_list = cell->GetPointIds();
            outDS->InsertNextCell(clipped->GetCellType(i), id_list);
            outCD->CopyData(newCD, i, i);
        }
        newCD = fineAsIs->GetCellData();
        for (i = 0 ; i < numCleanCells ; i++)
        {
            vtkCell *cell = fineAsIs->GetCell(i);
            vtkIdList *id_list = cell->GetPointIds();
            outDS->InsertNextCell(fineAsIs->GetCellType(i), id_list);
            outCD->CopyData(newCD, i, i+numClippedCells);
        }
        visitTimer->StopTimer(p5, "Unifying two datasets");
    }
    else
    {
        outDS->ShallowCopy(fineAsIs);
    }

    //
    // Clean up memory.
    //
    fineAsIs->Delete();
    toBeClipped->Delete();
    tmp->Delete();
    delete [] ptVal;
    pts->Delete();
    loc->Delete();

    subdivisionOccurred = true;

    return outDS;
}


// ****************************************************************************
//  Method: avtClipFilter::SetUpClipFunctions
//
//  Purpose:
//      Reads in atts-information needed in setting up clip functions. 
//
//  Arguments:
//
//  Returns:   True if set-up of clip functions succesful, false otherwise.
//       
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//    Brad Whitlock, Thu Oct 31 11:55:34 PDT 2002
//    Updated it so it works with the new state object.
//
// ****************************************************************************

bool
avtClipFilter::SetUpClipFunctions(vtkImplicitBoolean *funcs, bool &inv)
{
    funcs->SetOperationTypeToUnion();

    bool success = false;
    if (atts.GetFuncType() == ClipAttributes::Plane)
    {
        double *orig, *norm;
        if (atts.GetPlane1Status())
        {
            orig = atts.GetPlane1Origin();
            norm = atts.GetPlane1Normal();
            vtkPlane *plane = vtkPlane::New();
            // vtk requires floats, so send individually and do an
            // implicit cast.
            plane->SetNormal(norm[0], norm[1], norm[2]);
            plane->SetOrigin(orig[0], orig[1], orig[2]);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        if (atts.GetPlane2Status())
        {
            orig = atts.GetPlane2Origin();
            norm = atts.GetPlane2Normal();
            vtkPlane *plane = vtkPlane::New();
            // vtk requires floats, so send individually and do an
            // implicit cast.
            plane->SetNormal(norm[0], norm[1], norm[2]);
            plane->SetOrigin(orig[0], orig[1], orig[2]);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        if (atts.GetPlane3Status())
        {
            orig = atts.GetPlane3Origin();
            norm = atts.GetPlane3Normal();
            vtkPlane *plane = vtkPlane::New();
            // vtk requires floats, so send individually and do an
            // implicit cast.
            plane->SetNormal(norm[0], norm[1], norm[2]);
            plane->SetOrigin(orig[0], orig[1], orig[2]);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        //
        //  Okay, this may seem weird, but for 'erasing' with planes,
        //  'inside-out' is our 'normal' case, and 'inside-right'
        //  is our 'inverse'.  So use the opposite of the user setting 
        //  so that the correct 'side' will get set in the 
        //  clipping filter.
        //
        inv = !atts.GetPlaneInverse();
    }
    else if (atts.GetFuncType() == ClipAttributes::Sphere)
    {
        double  rad = atts.GetRadius();
        double *cent;
        cent = atts.GetCenter();
        vtkSphere *sphere = vtkSphere::New();
        // vtk requires floats, so send individually and do an
        // implicit cast.
        sphere->SetCenter(cent[0], cent[1], cent[2]);
        sphere->SetRadius(rad);
        funcs->AddFunction(sphere);
        sphere->Delete();
        success = true;
        inv = atts.GetSphereInverse();
    }

    return success;
}



// ****************************************************************************
//  Method: avtClipFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//
//    Hank Childs, Wed Jun  6 13:32:28 PDT 2001
//    Renamed from CopyDatasetMetaData.
//
// ****************************************************************************

void
avtClipFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


// ****************************************************************************
//  Method: avtClipFilter::PerformRestriction
//
//  Purpose:
//      This is defined so that we can examine the pipeline specification and
//      infer information about what is needed downstream.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2002
//
// ****************************************************************************

avtPipelineSpecification_p
avtClipFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtDataSpecification_p data = spec->GetDataSpecification();
    needsValidFaceConnectivity = data->NeedValidFaceConnectivity();
    return spec;
}


// ****************************************************************************
//  Method: avtClipFilter::PreExecute
//
//  Purpose:
//      Sets up a data member that will let us know if any subdivision 
//      occurred throughout the execution.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2002
//
// ****************************************************************************

void
avtClipFilter::PreExecute(void)
{
    subdivisionOccurred = false;
}


// ****************************************************************************
//  Method: avtClipFilter::PostExecute
//
//  Purpose:
//      This is called to set up some data attributes in the output.  If we 
//      clipped some of the cells (making them tetrahedrons), but left others
//      clean, then we should capture this.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2002
//
// ****************************************************************************

void
avtClipFilter::PostExecute(void)
{
    if (subdivisionOccurred)
    {
        GetOutput()->GetInfo().GetValidity().SetSubdivisionOccurred(
                                                        subdivisionOccurred);
        GetOutput()->GetInfo().GetValidity().SetNotAllCellsSubdivided(
                                                        subdivisionOccurred);
    }
}


// ****************************************************************************
//  Method: avtClipFilter::ReleaseData
//
//  Purpose:
//      Free up all problem size memory associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 21:42:31 PDT 2002
//    Release the locator, too.
//
// ****************************************************************************

void
avtClipFilter::ReleaseData(void)
{
    avtPluginStreamer::ReleaseData();

    clipData->SetInput(NULL);
    clipData->SetOutput(NULL);
    clipData->SetLocator(NULL);
    clipPoly->SetInput(NULL);
    clipPoly->SetOutput(NULL);
    clipVolume->SetInput(NULL);
    clipVolume->SetOutput(NULL);
}


