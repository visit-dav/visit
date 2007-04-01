// ************************************************************************* //
//                             avtClipFilter.C                              //
// ************************************************************************* //

#include <avtClipFilter.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkClipPolyData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkImplicitBoolean.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSphere.h>
#include <vtkStructuredPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItClipper.h>
#include <vtkVisItUtility.h>
#include <vtkDataSetWriter.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <BadVectorException.h>
#include <TimingsManager.h>


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
//    Jeremy Meredith, Fri Aug  8 09:18:40 PDT 2003
//    Removed subdivision and connectivity flags.  Added fastClipper.
//
//    Jeremy Meredith, Wed May  5 13:05:35 PDT 2004
//    Made my fast clipper support 2D, and removed the old generic
//    VTK data set clipper.
//
//    Hank Childs, Thu Mar 10 14:33:32 PST 2005
//    Remove filters.  They are now instantiated on the fly.
//
// ****************************************************************************

avtClipFilter::avtClipFilter()
{
    ;
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
//    Jeremy Meredith, Mon Aug 11 17:04:29 PDT 2003
//    Added fastClipper.
//
//    Jeremy Meredith, Wed May  5 13:05:35 PDT 2004
//    Made my fast clipper support 2D, and removed the old generic
//    VTK data set clipper.
//
//    Hank Childs, Thu Mar 10 14:33:32 PST 2005
//    Remove filters.  They are now instantiated on the fly.
//
// ****************************************************************************

avtClipFilter::~avtClipFilter()
{
}

// ****************************************************************************
//  Method:  avtClipFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  8, 2003
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
//  Programmer: Jeremy Meredith
//  Creation:   August  8, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed May  5 13:05:35 PDT 2004
//    Made my fast clipper support 2D, and removed the old generic
//    VTK data set clipper.
//
//    Hank Childs, Thu Mar 10 14:33:32 PST 2005
//    Instantiate filters on the fly.
//
// ****************************************************************************

vtkDataSet *
avtClipFilter::ExecuteData(vtkDataSet *inDS, int dom, std::string)
{
    vtkClipPolyData *clipPoly = vtkClipPolyData::New();
    vtkVisItClipper *fastClipper = vtkVisItClipper::New();
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

    if (inDS->GetDataObjectType() == VTK_POLY_DATA)
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
        outDS = vtkUnstructuredGrid::New();
        fastClipper->SetInput(inDS);
        fastClipper->SetOutput((vtkUnstructuredGrid*)outDS);
        fastClipper->SetClipFunction(ifuncs);
        fastClipper->SetInsideOut(inverse);

        fastClipper->Update();
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

    clipPoly->Delete();
    fastClipper->Delete();
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
//    Hank Childs, Wed Jun  6 13:32:28 PDT 2001
//    Renamed from CopyDatasetMetaData.
//
// ****************************************************************************

void
avtClipFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


