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
//  Method: avtClipFilter::ProcessOneChunk
//
//  Purpose:
//      Sends the specified input and output through the Clip filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      dom        The domain number.
//      label      The label.
//      isChunked  Whether or not the data set is produced by the structured
//                 mesh chunker -- not important for this module.
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
//    Hank Childs, Sun Mar 27 12:00:18 PST 2005
//    Renamed to ProcessOneChunk.  Changed memory management.  Fix small
//    memory leak in error condition.
//
// ****************************************************************************

vtkDataSet *
avtClipFilter::ProcessOneChunk(vtkDataSet *inDS, int dom, std::string, bool)
{
    vtkClipPolyData *clipPoly = vtkClipPolyData::New();
    vtkVisItClipper *fastClipper = vtkVisItClipper::New();
    vtkImplicitBoolean *ifuncs = vtkImplicitBoolean::New();
 
    bool inverse = false; 
    bool funcSet = SetUpClipFunctions(ifuncs, inverse);
    if (!funcSet)
    {
        // we have no functions to work with!
        clipPoly->Delete();
        fastClipper->Delete();
        ifuncs->Delete();
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
            outDS->Delete();
            outDS = NULL;
        }
        else
        {
            debug5 << "After clipping, domain " << dom << " has " 
                   << outDS->GetNumberOfCells() << " cells." << endl;
        }
    }

    clipPoly->Delete();
    fastClipper->Delete();
    return outDS;  // Calling function will free outDS.
}


// ****************************************************************************
//  Method: avtClipFilter::GetAssignments
//
//  Purpose:
//      Get the assignments for the clip.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

void
avtClipFilter::GetAssignments(vtkDataSet *in_ds, const int *dims,
                     std::vector<avtStructuredMeshChunker::ZoneDesignation> &d)
{
    int i, j, k;
    vtkImplicitBoolean *ifuncs = vtkImplicitBoolean::New();
 
    bool inverse = false; 
    bool funcSet = SetUpClipFunctions(ifuncs, inverse);
    if (!funcSet)
    {
        // we have no functions to work with!
        ifuncs->Delete();
        EXCEPTION0(ImproperUseException);
    }

    int pt_dims[3];
    pt_dims[0] = dims[0]+1;
    pt_dims[1] = dims[1]+1;
    pt_dims[2] = dims[2]+1;
    int npts = pt_dims[0]*pt_dims[1]*pt_dims[2];
    bool *pt_dist = new bool[npts];
    for (i = 0 ; i < npts ; i++)
    {
        double pt[3];
        in_ds->GetPoint(i, pt);
        bool pos_dist = ifuncs->EvaluateFunction(pt) >= 0;
        pt_dist[i] = (pos_dist && !inverse) || (!pos_dist && inverse);
    }

    for (k = 0 ; k < dims[2] ; k++)
        for (j = 0 ; j < dims[1] ; j++)
            for (i = 0 ; i < dims[0] ; i++)
            {
                bool oneIn = false;
                bool oneOut = false;
                for (int l = 0 ; l < 8 ; l++)
                {
                    int i2 = (l & 1 ? i+1 : i);
                    int j2 = (l & 2 ? j+1 : j);
                    int k2 = (l & 4 ? k+1 : k);
                    int index = k2*pt_dims[0]*pt_dims[1] + j2*pt_dims[0] + i2;
                    if (pt_dist[index])
                        oneIn = true;
                    else
                        oneOut = true;
                }
                int index = k*dims[0]*dims[1] + j*dims[0] + i;
                if (oneIn && oneOut)
                    d[index] = avtStructuredMeshChunker::TO_BE_PROCESSED;
                else if (!oneOut)
                    d[index] = avtStructuredMeshChunker::RETAIN;
                else if (!oneIn)
                    d[index] = avtStructuredMeshChunker::DISCARD;
                else
                {
                    EXCEPTION0(ImproperUseException); // should be impossible
                }
            }

    ifuncs->Delete();
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
            plane->SetNormal(norm);
            plane->SetOrigin(orig);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        if (atts.GetPlane2Status())
        {
            orig = atts.GetPlane2Origin();
            norm = atts.GetPlane2Normal();
            vtkPlane *plane = vtkPlane::New();
            plane->SetNormal(norm);
            plane->SetOrigin(orig);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        if (atts.GetPlane3Status())
        {
            orig = atts.GetPlane3Origin();
            norm = atts.GetPlane3Normal();
            vtkPlane *plane = vtkPlane::New();
            plane->SetNormal(norm);
            plane->SetOrigin(orig);
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
        sphere->SetCenter(cent);
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
//    Kathleen Bonnell, Thu Mar  2 14:26:06 PST 2006 
//    Set ZonesSplit.
//
//    Kathleen Bonnell, Fri Apr 28 10:57:21 PDT 2006 
//    Set OrigElementsRequiredForPick.
//
// ****************************************************************************

void
avtClipFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().ZonesSplit();
    GetOutput()->GetInfo().GetAttributes().SetOrigElementsRequiredForPick(true);
}


// ****************************************************************************
//  Method: avtClipFilter::PeformRestriction
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   Apr 28, 2006 
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtClipFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
 
    if (spec->GetDataSpecification()->MayRequireZones() ||
        spec->GetDataSpecification()->MayRequireNodes())
    {
        avtPipelineSpecification_p ns = new avtPipelineSpecification(spec);
        // Turn on both Nodes and Zones, to prevent another re-execution if 
        // user switches between zone and node pick.
        ns->GetDataSpecification()->TurnZoneNumbersOn();
        ns->GetDataSpecification()->TurnNodeNumbersOn();
        return ns;
    }
    return spec;
}

