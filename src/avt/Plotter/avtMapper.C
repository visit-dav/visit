// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtMapper.C                                //
// ************************************************************************* //

#include <avtMapper.h>

#include <vector>
#include <string>

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

#include <avtCallback.h>
#include <avtGeometryDrawable.h>
#include <avtTransparencyActor.h>

#include <ColorAttribute.h>
#include <DebugStream.h>

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
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004
//    Initialize specularIsInappropriate.
//
//    Hank Childs, Sat Dec  3 20:32:37 PST 2005
//    Change test for whether or not we are doing software rendering.
//
//    Kathleen Biagas, Tue Jul 12 13:29:26 MST 2016
//    Remove drawSurfaces/Wireframe/Points and similar.
//
//    Kathleen Biagas, Thu Apr 13 09:20:38 PDT 2017
//    Some settings moved to new base class.
//
// ****************************************************************************

avtMapper::avtMapper() : avtMapperBase()
{
    mappers  = NULL;
    actors   = NULL;
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
//  Method: avtMapper::SetUpDrawable
//
//  Purpose:
//      Creates a geometry drawable.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtMapper::SetUpDrawable()
{
    avtGeometryDrawable *gd = new avtGeometryDrawable(nMappers, actors);
    gd->SetMapper(this);
    drawable = gd;
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
//  Modifications:
//    Dave Bremer, Fri Mar  7 10:43:49 PST 2008
//    When this object clears itself, it should clear the drawable's
//    reference to 'this' as well.
//
//    Kathleen Biagas, Thu Apr 13 09:22:00 PDT 2017
//    Some settings moved to new base class.
//
// ****************************************************************************

void
avtMapper::ClearSelf(void)
{
    avtMapperBase::ClearSelf();

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
    nMappers = 0;
}


// ****************************************************************************
//  Method: avtMapper::SetUpTransparencyActor
//
//  Purpose:
//      Sets up the transparancy actor with the current inputs.
//
//  Notes:  Pulled from old SetUpMappers method.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtMapper::SetUpTransparencyActor()
{
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
}


// ****************************************************************************
//  Method: avtMapper::CreateActorMapperPairs
//
//  Purpose:
//      Creates actor and mappers.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtMapper::CreateActorMapperPairs(vtkDataSet **children)
{
    mappers  = new vtkDataSetMapper*[nMappers];
    actors   = new vtkActor*[nMappers];

    SetUpFilters(nMappers);

    for (int i = 0; i < nMappers; i++)
    {
        // We might have some dummy data (SR-mode).  If so, just continue.
        if (children[i] == NULL || children[i]->GetNumberOfCells() <= 0)
        {
            mappers[i] = NULL;
            actors[i]  = NULL;
            continue;
        }
        mappers[i] = CreateMapper();
        vtkAlgorithmOutput * outputPort = InsertFilters(children[i], i);
        if (outputPort != NULL)
            mappers[i]->SetInputConnection(outputPort);
        else
            mappers[i]->SetInputData(children[i]);
        actors[i]  = vtkActor::New();
        actors[i]->SetMapper(mappers[i]);
    }
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
//    Kathleen Biagas, Wed Feb 6 19:38:27 PDT 2013
//    Changed signature of InsertFilters to return vtkAlgorithmOutput, so
//    connections are set up properly with vtk-6.
//
// ****************************************************************************

vtkAlgorithmOutput *
avtMapper::InsertFilters(vtkDataSet *ds, int)
{
    return NULL;
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

    double minRange;
    double maxRange;
    if (!GetRange(minRange, maxRange))
    {
        minRange = 0;
        maxRange = 1;
    }

    //
    // Set each mapper with those extents.
    //
    for (int i = 0 ; i < nMappers ; i++)
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
    for (int i = 0; i < nMappers; i++)
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
    for (int i = 0; i < nMappers; i++)
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
avtMapper::GlobalSetAmbientCoefficient(const double amb)
{
    globalAmbient = amb;
    for (int i = 0; i < nMappers; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetAmbient(amb);
        }
    }
}


// ****************************************************************************
//  Method: avtMapper::SetSpecularProperties
//
//  Purpose:
//      Sets the property's surface representation.
//
//  Arguments:
//      flag  :  true to enable specular, false otherwise
//      coeff :  the new specular coefficient
//      power :  the new specular power
//      color :  the new specular color
//
//  Programmer: Jeremy Meredith
//  Creation:   November 14, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep  2 08:52:56 PDT 2004
//    Moved from avtGeometryDrawable so that derived mappers may override.
//    Only set the specular properties when appropriate (eg. only for surface
//    renderings.)
//
// ****************************************************************************

void
avtMapper::SetSpecularProperties(bool flag, double coeff, double power,
                                           const ColorAttribute &color)
{
    if (specularIsInappropriate)
    {
        return;
    }

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            vtkProperty *prop = actors[i]->GetProperty();
            if(prop != NULL && prop->GetRepresentation() == VTK_SURFACE)
            {
                prop->SetSpecular(flag ? coeff : 0);
                prop->SetSpecularPower(power);
                int r = color.Red();
                int g = color.Green();
                int b = color.Blue();
                prop->SetSpecularColor(double(r)/255.,
                                       double(g)/255.,
                                       double(b)/255.);
            }
        }
    }
}

