// ************************************************************************* //
//                                avtMapper.C                                //
// ************************************************************************* //

#include <avtMapper.h>

#include <float.h>
#include <vector>
#include <string>

#include <vtkActor.h>
#include <vtkDataObjectCollection.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtGeometryDrawable.h>
#include <avtTransparencyActor.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtMapper constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Mar 15 19:15:10 PST 2001
//    Added initialization of new members nRenderingModes, modeVisibility,
//    modeRepresentation.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001 
//    Removed functionality related to having multiple rendering modes.
//    No longer needed.
//
//    Hank Childs, Sun Jul  7 12:31:10 PDT 2002
//    Initialize transparency actor as NULL.
//
//    Brad Whitlock, Mon Sep 23 16:56:08 PST 2002
//    I initialized the new immediateMode flag.
//
//    Kathleen Bonnell, Sat Oct 19 15:08:41 PDT 2002  
//    Initialize globalAmbient.
//
//    Mark C. Miller, Thu Jan  9 13:31:32 PST 2003
//    Added initializiation for transparencyIndex data member to -1
//    Added initializations for externally rendered images actor
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Removed extRenderedImagesActor data member
//
// ****************************************************************************

avtMapper::avtMapper()
{
    immediateMode = avtCallback::GetNowinMode();
    drawable = NULL;
    mappers  = NULL;
    actors   = NULL;
    nMappers = 0;
    transparencyActor = NULL;
    transparencyIndex = -1;
    globalAmbient = 0.;
}


// ****************************************************************************
//  Method: avtMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//    Kathleen Bonnell, Thu Mar 15 19:15:10 PST 2001
//    Added destruction of new members modeVisibility, modeRepresentation.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001 
//    Removed functionality related to having multiple rendering modes.
//    No longer needed.
//
// ****************************************************************************

avtMapper::~avtMapper()
{
    ClearSelf();
}


// ****************************************************************************
//  Method: avtMapper::ChangedInput
//
//  Purpose:
//      A hook from avtDatasetSink telling us the input has changed.  We will
//      invalidate the drawable we previously had.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

void
avtMapper::ChangedInput(void)
{
    MapperChangedInput();
    ClearSelf();
}


// ****************************************************************************
//  Method: avtMapper::InputIsReady
//
//  Purpose:
//      Since we now know that the input is ready, this sets up the mappers.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

void
avtMapper::InputIsReady(void)
{
    SetUpMappers();
}


// ****************************************************************************
//  Method: avtMapper::MapperChangedInput
//
//  Purpose:
//      This is a hook to allow derived types of avtMapper to reset their
//      state.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

void
avtMapper::MapperChangedInput(void)
{
    ;
}


// ****************************************************************************
//  Method: avtMapper::ClearSelf
//
//  Purpose:
//      Cleans up memory so a new drawable can be created.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

void
avtMapper::ClearSelf(void)
{
    //
    // This probably doesn't need to be done, but it will guarantee that we are
    // never in an inconsistent state (where we have a valid drawable, but no
    // mappers).
    //
    drawable = NULL;

    if (mappers != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (mappers[i] != NULL)
            {
                mappers[i]->Delete();
            }
        }
        delete [] mappers;
    }
    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                actors[i]->Delete();
            }
        }
        delete [] actors;
    }

    //
    // Nothing should be done with the transparency actor -- we don't own it.
    //
    mappers = NULL;
    actors = NULL;
}

void
avtMapper::ReleaseData(void)
{
   ClearSelf();
}


// ****************************************************************************
//  Method: avtMapper::GetDrawable
//
//  Purpose:
//      Gets a drawable from the input dataset.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

avtDrawable_p
avtMapper::GetDrawable(void)
{
    avtDataObject_p input = GetInput();
    if (*drawable == NULL || *input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return drawable;
}


// ****************************************************************************
//  Method: avtMapper::SetUpMappers
//
//  Purpose:
//      Sets up the mappers and creates a drawable based on the input.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  9 15:44:57 PST 2001
//    Modified to reflect that GetInputDomain returns and avtDomainTree.
//
//    Kathleen Bonnell, Thu Feb 15 10:06:43 PST 2001 
//    Add check for NULL tree on return from call to GetInputDomain. 
//
//    Kathleen Bonnell, Thu Mar 15 19:15:10 PST 2001 
//    Added logic to handle multiple rendering modes (creating extra
//    mappers/actors to handle this), and for setting actor visibility
//    and representation. 
//
//    Hank Childs, Fri Mar 23 14:44:49 PST 2001
//    Added hook to allow derived classes to insert filters into the pipeline.
//
//    Hank Childs, Mon Mar 26 19:44:21 PST 2001
//    Added hook to allow derived classes to use their own mappers.
//
//    Kathleen Bonnell, Thu Apr 19 12:22:15 PDT 2001 
//    Reflect that input now stored as single avtDatTree instead of multiple
//    avtDomainTrees.  Changed parameter in call to InsertFilters from 
//    j (child num) to i (domain num).
//
//    Hank Childs, Sun Jun 24 19:52:13 PDT 2001
//    Added explicit initializations that only come up during error cases.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001 
//    Removed functionality related to having multiple rendering modes.
//    No longer needed.
//
//    Kathleen Bonnell, Fri Sep 21 10:54:37 PDT 2001 
//    Reflect that data trees are now compacted before being sent to viewer.  
//    
//    Kathleen Bonnell, Mon Sep 24 15:24:18 PDT 2001 
//    Retrieve labels from input tree and from info attributes.  Call 
//    SetLabels method so derived types may make us of the labels. 
//    
//    Hank Childs, Fri May 24 15:41:49 PDT 2002
//    Tell the drawable which mapper it came from.
//
//    Hank Childs, Sun Jul  7 12:31:10 PDT 2002
//    Add support for transparency.
//
//    Hank Childs, Sun Aug 18 21:06:24 PDT 2002
//    Do not use display lists if we have more than 250,000 cells.
//
//    Hank Childs, Wed Sep 18 11:04:10 PDT 2002
//    Do not use display lists if we are in no-win mode.
//
//    Brad Whitlock, Mon Sep 23 16:56:50 PST 2002
//    Changed the immediate mode rendering test so it uses a new member.
//
//    Hank Childs, Thu Mar 18 16:02:27 PST 2004
//    Do not create actors and mappers for empty datasets.
//
//    Hank Childs, Tue May 25 13:30:16 PDT 2004
//    Allow display lists of 250,000 polygons or more again, since the display
//    list generation was made much faster by breaking it into chunks.
//
// ****************************************************************************

void
avtMapper::SetUpMappers(void)
{
    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    avtDataTree_p tree = GetInputDataTree();
   
    vector<string> labels;
    tree->GetAllLabels(labels);
    if (!labels.empty() )
    {
        SetLabels(labels, true);
        labels.clear();
    }

    input->GetInfo().GetAttributes().GetLabels(labels);
    if (!labels.empty())
    {
        SetLabels(labels, false);
        labels.clear();
    }

    vtkDataSet **children;
    children = tree->GetAllLeaves(nMappers);

    mappers  = new vtkDataSetMapper*[nMappers];
    actors   = new vtkActor*[nMappers];

    for (int j = 0 ; j < nMappers ; j++)
    {
        mappers[j] = NULL;
        actors[j]  = NULL;
    }
    SetUpFilters(nMappers);
    for (int i = 0; i < nMappers; i++)
    {
        // We might have some dummy data (SR-mode).  If so, just continue.
        if (children[i] == NULL)
            continue;
        if (children[i]->GetNumberOfCells() <= 0)
            continue;

        mappers[i] = CreateMapper();
        vtkDataSet *ds = InsertFilters(children[i], i);
        mappers[i]->SetInput(ds);
        if (immediateMode)
        {
            mappers[i]->ImmediateModeRenderingOn();
        }
        actors[i]  = vtkActor::New();
        actors[i]->SetMapper(mappers[i]);
    }
    // this was allocated in GetAllLeaves, need to free it now
    delete [] children;

    PrepareExtents();

    CustomizeMappers();

    if (transparencyActor != NULL)
    {
        vector<vtkDataSet *> d;
        vector<vtkDataSetMapper *> m;
        vector<vtkActor *> a;
        for (int i = 0 ; i < nMappers ; i++)
        {
            vtkDataSet *ds = (mappers[i] != NULL ? mappers[i]->GetInput()
                                                 : NULL);
            d.push_back(ds);
            m.push_back(mappers[i]);
            a.push_back(actors[i]);
        }
        transparencyActor->ReplaceInput(transparencyIndex, d, m, a);
    }
    avtGeometryDrawable *gd = new avtGeometryDrawable(nMappers, actors);
    gd->SetMapper(this);
    drawable = gd;
}


// ****************************************************************************
//  Method: avtMapper::SetUpFilters
//
//  Purpose:
//      Sets up any filters that should be inserted into the pipeline before
//      the vtkMappers.  This is a stub for the derived classes.
//
//  Arguments:
//      <unnamed>   The number of domains.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications: 
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001 
//    Removed argument related to having multiple rendering modes.
//    No longer needed.
//
// ****************************************************************************

void
avtMapper::SetUpFilters(int)
{
    ;
}


// ****************************************************************************
//  Method: avtMapper::InsertFilters
//
//  Purpose:
//      Allows the derived types to insert filters into the pipeline before
//      the vtkMappers.  This is a stub for the derived classes.
//
//  Arguments:
//      ds          The input dataset.
//      <unnamed>   The index of the domain.
//      
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications: 
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001 
//    Removed argument related to having multiple rendering modes.
//    No longer needed.
//
// ****************************************************************************

vtkDataSet *
avtMapper::InsertFilters(vtkDataSet *ds, int)
{
    return ds;
}


// ****************************************************************************
//  Method: avtMapper::SetDefaultRange
//
//  Purpose:
//      Go through the input of all of the mappers and determine what the
//      extents are.  Then set each mapper to be in that data range.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    Removed unnecessary creation of a vtkLookupTable.
//
//    Hank Childs, Tue Sep 18 10:37:29 PDT 2001
//    Removed unused variable data.
//
//    Hank Childs, Thu Jun 17 14:29:02 PDT 2004
//    Do not make use of the mappers min/max, since this does not work with
//    scalable rendering mode.
//
// ****************************************************************************

void
avtMapper::SetDefaultRange(void)
{
    if (mappers == NULL)
    {
        //
        // We haven't set up the mappers yet.
        //
        return;
    }

    int  i;
    float minRange;
    float maxRange;
    if (!GetRange(minRange, maxRange))
    {
        minRange = 0;
        maxRange = 1;
    }

    //
    // Set each mapper with those extents.
    //
    for (i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            mappers[i]->SetScalarRange(minRange, maxRange);
        }
    }
}


// ****************************************************************************
//  Method: avtMapper::CreateMapper
//
//  Purpose:
//      A hook to allow derived types to insert their own types of mappers.
// 
//  Returns:    A standard vtkDataSetMapper.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
// ****************************************************************************

vtkDataSetMapper *
avtMapper::CreateMapper(void)
{
    return vtkDataSetMapper::New();
}


// ****************************************************************************
//  Method: avtMapper::GetRange
//
//  Purpose:
//      Gets the range of the input.  This will looks at the extents in the
//      information if they are set and look at the dataset otherwise.
//
//  Arguments:
//      rmin    Will hold the minimum value.
//      rmax    Will hold the maximum value.
//
//  Returns:    True if the extents were calculated successfully,
//              False otherwise.
//
//  Programmer: Hank Childs
//  Creation:   April 20, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep  7 18:30:33 PDT 2001
//    Sent a double into GetDataExtents for new interface.
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Account for dataset examiner.
//
//    Sean Ahern, Thu May 16 12:13:40 PDT 2002
//    Added some debugging.
//
//    Hank Childs, Thu Oct 10 08:22:31 PDT 2002
//    Added a return value.
//
//    Brad Whitlock, Wed Dec 4 11:39:48 PDT 2002
//    I changed the code so it supports calculating extents of vector
//    magnitudes.
//
//    Hank Childs, Tue Sep 23 23:08:19 PDT 200
//    Support tensor magnitudes as the eigenvalues extents.
//
//    Kathleen Bonnell, Thu Mar 11 10:07:35 PST 2004 
//    Tensor, vectors and scalars all handled by GetDataExtents. 
//
//    Jeremy Meredith, Fri Jun 18 14:18:23 PDT 2004
//    Allow variable dimension of 4.  This will be used for colors.
//
// ****************************************************************************

bool
avtMapper::GetRange(float &rmin, float &rmax)
{
    if(*GetInput() == 0)
    {
        debug1 << "avtMapper::GetRange: The mapper has no input!" << endl;
        EXCEPTION0(ImproperUseException);
    }

    bool gotExtents = false;
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    if ((data.GetVariableDimension() == 1) ||
        (data.GetVariableDimension() <= 3) ||
        (data.GetVariableDimension() == 4) ||
        (data.GetVariableDimension() == 9))
    {
        double extents[2];
        gotExtents = data.GetDataExtents(extents);
    
        if (gotExtents)
        {
            rmin = extents[0];
            rmax = extents[1];
        }
        else
        {
            double de[2];
            avtDataset_p input = GetTypedInput();
            gotExtents = avtDatasetExaminer::GetDataExtents(input, de);

            rmin = (float) de[0];
            rmax = (float) de[1];
        }
    }
    else
    {
        debug1 << "avtMapper::GetRange: Invalid variable dimension: dim=" <<
            data.GetVariableDimension() << endl;
        EXCEPTION0(ImproperUseException);
    }

    return gotExtents;
}


// ****************************************************************************
//  Method: avtMapper::PrepareExtents
//
//  Purpose:
//      Prepares the extents of a dataset.  This means trying to figure out
//      what the current extents are by looking at the original extents and
//      finally the data itself.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Sep  5 16:56:35 PDT 2001
//    Removed unnecessary temporary variable to fix compiler warnings.
//
//    Hank Childs, Fri Sep  7 18:30:33 PDT 2001
//    Sent doubles into avtDataset::Get...Extent routines.
//
//    Kathleen Bonnell, Wed Oct 10 13:32:31 PDT 2001 
//    Retrieve/set current data extents. 
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Account for dataset examiner.
//
//    Hank Childs, Fri Feb 27 08:29:54 PST 2004
//    Account for multiple variables.
//
//    Mark C. Miller, Sun Feb 29 18:35:00 PST 2004
//    Added calls to GetAnySpatialExtents before arbitrarily setting to [0,1]
//
//    Kathleen Bonnell, Thu Mar 11 10:07:35 PST 2004 
//    DataExtents now always have only 2 components.
//
// ****************************************************************************

void
avtMapper::PrepareExtents(void)
{
    avtDataset_p input = GetTypedInput();

    avtDataAttributes &atts = input->GetInfo().GetAttributes();    

    int nvars = atts.GetNumberOfVariables();
    double exts[2];
    for (int var = 0 ; var < nvars ; var++)
    {
        const char *vname = atts.GetVariableName(var).c_str();
        bool gotDataExtents = atts.GetDataExtents(exts, vname);
        if (!gotDataExtents)
        {
            if (!(avtDatasetExaminer::GetDataExtents(input, exts, vname)))
            {
                exts[0] = 0.;
                exts[1] = 1.;
            }
        }
        atts.GetTrueDataExtents(vname)->Set(exts);

        bool gotCurrentDataExtents = atts.GetCurrentDataExtents(exts, vname);
        if (!gotCurrentDataExtents)
        {
            if (!(avtDatasetExaminer::GetDataExtents(input, exts, vname)))
            {
                exts[0] = 0.;
                exts[1] = 1.;
            }
        }
        atts.GetCurrentDataExtents(vname)->Set(exts);
    }

    double bounds[6];
    bool gotBounds = atts.GetSpatialExtents(bounds);
    if (!gotBounds)
    {
        if (!(avtDatasetExaminer::GetSpatialExtents(input, bounds)))
        {
            if (!atts.GetAnySpatialExtents(bounds))
            {
                for (int i = 0 ; i < 3 ; i++)
                {
                    bounds[2*i] = 0.;
                    bounds[2*i+1] = 1.;
                }
            }
        }
    }
    atts.GetTrueSpatialExtents()->Set(bounds);
}


// ****************************************************************************
//  Method: avtMapper::SetLabels
//
//  Purpose:
//      A hook to allow derived types to make use of labels. 
// 
//  Arguments:    
//    <unnamed>  The labels. 
//    <unnamed>  True if labels were retrieved from the input data tree,
//               false if labels were retrieved from input info.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 24, 2001 
//
// ****************************************************************************

void
avtMapper::SetLabels(vector<string> &, bool)
{
   ; 
}

// ****************************************************************************
//  Method: avtMapper::GetCurrentRange
//
//  Purpose:
//      Gets the currentrange of the input.  This will look at the extents 
//      in the information if they are set and look at the dataset otherwise.
//
//  Arguments:
//      rmin     Will hold the minimum value.
//      rmax     Will hold the maximum value.
//
//  Returns:    True, if it successfully calculated the extents,
//              False otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 3, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Oct 10 08:25:18 PDT 2002
//    Added a return value.
//
// ****************************************************************************

bool
avtMapper::GetCurrentRange(float &rmin, float &rmax)
{
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    if (data.GetVariableDimension() != 1)
    {
        EXCEPTION0(ImproperUseException);
    }

    double extents[2];
    bool gotExtents = data.GetCurrentDataExtents(extents);

    if (!gotExtents)
    {
        //
        //  avtDataset retrives extents from the tree, so this will get us
        //  the proper current range.
        //
        avtDataset_p input = GetTypedInput();
        gotExtents = avtDatasetExaminer::GetDataExtents(input, extents);
    }

    rmin = (float) extents[0];
    rmax = (float) extents[1];

    return gotExtents;
}


// ****************************************************************************
//  Method: avtMapper::SetTransparencyActor
//
//  Purpose:
//      Sets the transparency actor.
//
//  Arguments:
//      act      The transparency actor.  It is associated with a specific
//               visualization window and is not owned by this object.
//
//  Programmer:  Hank Childs
//  Creation:    July 7, 2002
//
//  Modifications:
//
//      Chris Wojtan, Fri Jun 25 15:15 PDT 2004
//      we are now keeping track of the dimensionality of transparent data
//
// ****************************************************************************

int 
avtMapper::SetTransparencyActor(avtTransparencyActor *act)
{
    // record whether this data is all 2D or all 3D
    if(GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
        act->SetIs2Dimensional(true);
    else
        act->SetIs2Dimensional(false);

    if (transparencyActor == act)
    {
        //
        // This is our current transparency actor anyway.  No need to do
        // anything new.
        //
        return transparencyIndex;
    }

    transparencyActor = act;

    vector<vtkDataSet *> d;
    vector<vtkDataSetMapper *> m;
    vector<vtkActor *> a;
    for (int i = 0 ; i < nMappers ; i++)
    {
        vtkDataSet *ds = (mappers[i] != NULL ? mappers[i]->GetInput() : NULL);
        d.push_back(ds);
        m.push_back(mappers[i]);
        a.push_back(actors[i]);
    }

    transparencyIndex = transparencyActor->AddInput(d, m, a);
    return transparencyIndex;
}


// ****************************************************************************
//  Method: avtMapper::GetLighting
//
//  Purpose:
//      Returns the lighting state for a particular mapper. 
//      Derived classes may override as necessary.
//
//  Returns:
//      True if lighting for this mapper is ON, false otherwise. 
//      Default is ON.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002
//
// ****************************************************************************

bool
avtMapper::GetLighting()
{
   return true; 
}


// ****************************************************************************
//  Method: avtMapper::GlobalLightingOn
//
//  Purpose:
//      Sets the lighting coefficients necessary for a Lights On state. 
//      Derived classes may override as necessary.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002
//
//  Modifications:
//    Kathleen Bonnell, Sat Oct 19 15:08:41 PDT 2002  
//    Allow lighting only for surface representations.
//
// ****************************************************************************

void
avtMapper::GlobalLightingOn()
{
    if (!GetLighting())
    {
        //
        // Don't override the derived class lighting state if off.
        //
        return;
    }
    int i; 
    for (i = 0; i < nMappers; i++)
    { 
        if (actors[i] != NULL)
        {
            vtkProperty *prop = actors[i]->GetProperty();
            if (prop->GetRepresentation() == VTK_SURFACE)
            {
                prop->SetAmbient(0.);
                prop->SetDiffuse(1.);
            }
            else 
            {
                prop->SetAmbient(1.);
                prop->SetDiffuse(0.);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtMapper::GlobalLightingOff
//
//  Purpose:
//      Sets the lighting coefficients necessary for a Lights Off state. 
//      Derived classes may override if necessary.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002
//
// ****************************************************************************

void
avtMapper::GlobalLightingOff()
{
    int i; 
    for (i = 0; i < nMappers; i++)
    { 
        if (actors[i] != NULL)
        {
            vtkProperty *prop = actors[i]->GetProperty();
            prop->SetAmbient(1.);
            prop->SetDiffuse(0.);
        }
    }
}


// ****************************************************************************
//  Method: avtMapper::GlobalSetAmbientCoefficient
//
//  Purpose:
//      Sets the ambient lighting coefficient to the specified value.
//      Derived classes may override if necessary.
//
//  Arguments:
//      amb    The new ambient lighting coefficient.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002
//
//  Modifications:
//    Kathleen Bonnell, Sat Oct 19 15:08:41 PDT 2002  
//    Store the coefficient in globalAmbient.
//
// ****************************************************************************

void
avtMapper::GlobalSetAmbientCoefficient(const float amb)
{
    int i; 
    globalAmbient = amb;
    for (i = 0; i < nMappers; i++)
    { 
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetAmbient(amb);
        }
    }
}

// ****************************************************************************
// Method: avtMapper::SetImmediateNodeRendering
//
// Purpose: 
//   Sets the immediate mode flag into all of the vtkMappers.
//
// Arguments:
//   val : The new immediate mode flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 17:01:30 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtMapper::SetImmediateModeRendering(bool val)
{
    if(!avtCallback::GetNowinMode())
    {
        immediateMode = val;
        for (int i = 0; i < nMappers; i++)
        {
            if(mappers[i] != NULL)
            {
                if(immediateMode != (mappers[i]->GetImmediateModeRendering()?1:0))
                    mappers[i]->SetImmediateModeRendering(immediateMode?1:0);
            }
        }
    }
}

// ****************************************************************************
// Method: avtMapper::GetImmediateModeRendering
//
// Purpose: 
//   Returns the immediate rendering flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 17:02:43 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtMapper::GetImmediateModeRendering()
{
    return immediateMode;
}

