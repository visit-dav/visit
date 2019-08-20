// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtLine3DColleague.C                         //
// ************************************************************************* //
#include <avtLine3DColleague.h>

#include <math.h>

#include <vtkActor.h>
#include <vtkConeSource.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTubeFilter.h>

#include <AnnotationObject.h>
#include <LineAttributes.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>



// ****************************************************************************
// Method: avtLine3DColleague::avtLine3DColleague
//
// Purpose:
//   Constructor for the avtLine3DColleague class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
//   Kathleen Biagas, Mon Jul 30 16:44:27 PDT 2018
//   Set initial position/size of arrows based on current extents.
//
// ****************************************************************************

avtLine3DColleague::avtLine3DColleague(VisWindowColleagueProxy &m):
    avtAnnotationColleague(m)
{
    addedToRenderer = false;
    useForegroundForLineColor = true;
    useArrow1 = false;
    useArrow2 = false;
    arrow1Added = false;
    arrow2Added = false;
    lineType = 0;

    // set up the line
    lineSource = vtkLineSource::New();
    lineSource->SetResolution(1);
    lineSource->SetPoint1(0, 0, 0);
    lineSource->SetPoint2(1, 1, 0);

    tubeFilter = vtkTubeFilter::New();
    tubeFilter->SetNumberOfSides(9);
    tubeFilter->CappingOn();

    lineMapper  = vtkPolyDataMapper::New();
    lineMapper->SetInputConnection(lineSource->GetOutputPort());

    lineActor   = vtkActor::New();
    lineActor->SetMapper(lineMapper);
    lineActor->PickableOff();


    // retrieve the bounds to use for calculating  default arrow radius/height.
    double bounds[6];
    mediator.GetBounds(bounds);
    double dX = bounds[1] - bounds[0];
    double dY = bounds[3] - bounds[2];
    double dZ = bounds[5] - bounds[4];
    double diagonal = sqrt(dX*dX + dY*dY + dZ*dZ);
    double h = 0.03*diagonal;
    double r = h/2.8;

    // Arrow resolution:
    //     1 => triangle,
    //     2 => 2 triangles,
    //    >2 => 3d cone

    // set up arrow #1
    arrow1Source = vtkConeSource::New();
    arrow1Source->SetResolution(16);
    arrow1Source->CappingOn();
    arrow1Source->SetDirection(-1, -1, 0);
    arrow1Source->SetCenter(0, 0, 0);
    arrow1Source->SetRadius(r);
    arrow1Source->SetHeight(h);

    arrow1Mapper  = vtkPolyDataMapper::New();
    arrow1Mapper->SetInputConnection(arrow1Source->GetOutputPort());

    arrow1Actor  = vtkActor::New();
    arrow1Actor->SetMapper(arrow1Mapper);
    arrow1Actor->PickableOff();


    // set up arrow #2
    arrow2Source = vtkConeSource::New();
    arrow2Source->SetResolution(16);
    arrow2Source->CappingOn();
    arrow2Source->SetDirection(1, 1, 0);
    arrow2Source->SetCenter(1, 1, 0);
    arrow2Source->SetRadius(r);
    arrow2Source->SetHeight(h);

    arrow2Mapper  = vtkPolyDataMapper::New();
    arrow2Mapper->SetInputConnection(arrow2Source->GetOutputPort());

    arrow2Actor  = vtkActor::New();
    arrow2Actor->SetMapper(arrow2Mapper);
    arrow2Actor->PickableOff();

    // Set a default color.
    double fgColor[3];
    mediator.GetForegroundColor(fgColor);
    SetForegroundColor(fgColor[0], fgColor[1], fgColor[2]);

    // Set default opacity.
    lineActor->GetProperty()->SetOpacity(1.);

    // Set a default line width.
    lineActor->GetProperty()->SetLineWidth(1);
}


// ****************************************************************************
// Method: avtLine3DColleague::~avtLine3DColleague
//
// Purpose:
//   Destructor for the avtLine3DColleague class.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
// ****************************************************************************

avtLine3DColleague::~avtLine3DColleague()
{
    if(lineActor != NULL)
    {
        lineActor->Delete();
        lineActor = NULL;
    }

    if(lineMapper != NULL)
    {
        lineMapper->Delete();
        lineMapper = NULL;
    }

    if(lineSource != NULL)
    {
        lineSource->Delete();
        lineSource = NULL;
    }

    if(arrow1Actor != NULL)
    {
        arrow1Actor->Delete();
        arrow1Actor = NULL;
    }

    if(arrow1Mapper != NULL)
    {
        arrow1Mapper->Delete();
        arrow1Mapper = NULL;
    }

    if(arrow1Source != NULL)
    {
        arrow1Source->Delete();
        arrow1Source = NULL;
    }

    if(arrow2Actor != NULL)
    {
        arrow2Actor->Delete();
        arrow2Actor = NULL;
    }

    if(arrow2Mapper != NULL)
    {
        arrow2Mapper->Delete();
        arrow2Mapper = NULL;
    }

    if(arrow2Source != NULL)
    {
        arrow2Source->Delete();
        arrow2Source = NULL;
    }
    if(tubeFilter != NULL)
    {
        tubeFilter->Delete();
        tubeFilter = NULL;
    }
}


// ****************************************************************************
// Method: avtLine3DColleague::AddToRenderer
//
// Purpose:
//   This method adds the lineActor to the renderer.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
// ****************************************************************************

void
avtLine3DColleague::AddToRenderer()
{
    if(!addedToRenderer && ShouldBeAddedToRenderer())
    {
        mediator.GetCanvas()->AddActor(lineActor);
        addedToRenderer = true;
        if (useArrow1)
        {
            mediator.GetCanvas()->AddActor(arrow1Actor);
            arrow1Added = true;
        }
        if (useArrow2)
        {
            mediator.GetCanvas()->AddActor(arrow2Actor);
            arrow2Added = true;
        }
    }
}


// ****************************************************************************
// Method: avtLine3DColleague::RemoveFromRenderer
//
// Purpose:
//   This method removes the lineActor from the renderer.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
// ****************************************************************************

void
avtLine3DColleague::RemoveFromRenderer()
{
    if(addedToRenderer)
    {
        mediator.GetCanvas()->RemoveActor(lineActor);
        addedToRenderer = false;
        if (arrow1Added)
        {
            mediator.GetCanvas()->RemoveActor(arrow1Actor);
            arrow1Added = false;
        }
        if (arrow2Added)
        {
            mediator.GetCanvas()->RemoveActor(arrow2Actor);
            arrow2Added = false;
        }
    }
}


// ****************************************************************************
// Method: avtLine3DColleague::Hide
//
// Purpose:
//   This method toggles the visible flag and either adds or removes the
//   lineActor to/from the renderer.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//
// ****************************************************************************

void
avtLine3DColleague::Hide()
{
    SetVisible(!GetVisible());

    if(addedToRenderer)
        RemoveFromRenderer();
    else
        AddToRenderer();
}


// ****************************************************************************
// Method: avtLine3DColleague::ShouldBeAddedToRenderer
//
// Purpose:
//   This method returns whether or not the lineActor should be added to the
//   renderer.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//
// ****************************************************************************

bool
avtLine3DColleague::ShouldBeAddedToRenderer() const
{
    return GetVisible() && mediator.HasPlots();
}


// ****************************************************************************
// Method: avtLine3DColleague::SetOptions
//
// Purpose:
//   This method sets the lineActor's properties from the values in the
//   annotation object.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
//   Kathleen Biagas, Mon Jul 30 16:46:56 PDT 2018
//   Modify height when radius changes.
//
// ****************************************************************************

void
avtLine3DColleague::SetOptions(const AnnotationObject &annot)
{
    // Get the current options.
    AnnotationObject currentOptions;
    GetOptions(currentOptions);

    const MapNode &newOpts = annot.GetOptions();
    const MapNode &curOpts = currentOptions.GetOptions();

    //
    // Set the position coordinates if they are different
    //
    if (!currentOptions.FieldsEqual(4, &annot) ||
        !currentOptions.FieldsEqual(5, &annot))
    {
        const double *p1 = annot.GetPosition();
        const double *p2 = annot.GetPosition2();

        double dir1[3] = {p1[0]-p2[0],p1[1]-p2[1],p1[2]-p2[2]};
        double dir2[3] = {p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]};

        lineSource->SetPoint1(p1[0], p1[1], p1[2]);
        lineSource->SetPoint2(p2[0], p2[1], p2[2]);

        arrow1Source->SetDirection(dir1);
        arrow1Source->SetCenter(p1[0], p1[1], p1[2]);

        arrow2Source->SetDirection(dir2);
        arrow2Source->SetCenter(p2[0], p2[1], p2[2]);
    }

    // strange, but the 'new' annotation object may not have 'Options' set, happens
    // with java
    if (newOpts.GetNumEntries() == curOpts.GetNumEntries() && ! (curOpts == newOpts))
    {
        lineActor->GetProperty()->SetLineWidth(newOpts.GetEntry("width")->AsInt()+1);
        lineType = newOpts.GetEntry("lineType")->AsInt();
        if (lineType)
        {
            tubeFilter->SetInputConnection(lineSource->GetOutputPort());
            lineMapper->SetInputConnection(tubeFilter->GetOutputPort());
        }
        else
        {
            lineMapper->SetInputConnection(lineSource->GetOutputPort());
        }

        int qual = newOpts.GetEntry("tubeQuality")->AsInt();
        if (qual == 0)
            tubeFilter->SetNumberOfSides(3);
        else if (qual == 1)
            tubeFilter->SetNumberOfSides(9);
        else
            tubeFilter->SetNumberOfSides(15);
        tubeFilter->SetRadius(newOpts.GetEntry("tubeRadius")->AsDouble());


        useArrow1 = newOpts.GetEntry("arrow1")->AsBool();
        arrow1Source->SetResolution(newOpts.GetEntry("arrow1Resolution")->AsInt());
        arrow1Source->SetRadius(newOpts.GetEntry("arrow1Radius")->AsDouble());
        arrow1Source->SetHeight(newOpts.GetEntry("arrow1Height")->AsDouble());

        useArrow2 = newOpts.GetEntry("arrow2")->AsBool();
        arrow2Source->SetResolution(newOpts.GetEntry("arrow2Resolution")->AsInt());
        arrow2Source->SetRadius(newOpts.GetEntry("arrow2Radius")->AsDouble());
        arrow2Source->SetHeight(newOpts.GetEntry("arrow2Height")->AsDouble());
    }

    //
    // The line color has changed or the useForegroundForTextColor flag
    // has changed.
    //
    if(annot.GetUseForegroundForTextColor() != useForegroundForLineColor ||
       annot.GetColor1() != lineColor)
    {
        // Record the line color that should be used when we're not using
        // the foreground text color.
        lineColor = annot.GetColor1();
        useForegroundForLineColor = annot.GetUseForegroundForTextColor();

        // Compute the line opacity.
        double lc[4];
        lc[3] = double(lineColor.Alpha()) / 255.;

        // Set the line color using the foreground color or the line color.
        if(useForegroundForLineColor)
        {
            // Get the foreground color.
            double fgColor[3];
            mediator.GetForegroundColor(fgColor);
            lineActor->GetProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);
            arrow1Actor->GetProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);
            arrow2Actor->GetProperty()->SetColor(fgColor[0], fgColor[1], fgColor[2]);
        }
        else
        {
            // Compute the line color as double.
            lc[0] = double(lineColor.Red()) / 255.;
            lc[1] = double(lineColor.Green()) / 255.;
            lc[2] = double(lineColor.Blue()) / 255.;
            lineActor->GetProperty()->SetColor(lc[0], lc[1], lc[2]);
            arrow1Actor->GetProperty()->SetColor(lc[0], lc[1], lc[2]);
            arrow2Actor->GetProperty()->SetColor(lc[0], lc[1], lc[2]);
        }

        lineActor->GetProperty()->SetOpacity(double(lc[3]));
        arrow1Actor->GetProperty()->SetOpacity(double(lc[3]));
        arrow2Actor->GetProperty()->SetOpacity(double(lc[3]));
    }

    //
    // Set the object's visibility.
    //
    if(currentOptions.GetVisible() != annot.GetVisible())
    {
        SetVisible(annot.GetVisible());
        if(annot.GetVisible())
            AddToRenderer();
        else
            RemoveFromRenderer();
    }
}


// ****************************************************************************
// Method: avtLine3DColleague::GetOptions
//
// Purpose:
//   This method stores the attributes in an object that can
//   be passed back to the client.
//
// Arguments:
//   annot : The AnnotationObject to populate.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
// ****************************************************************************

void
avtLine3DColleague::GetOptions(AnnotationObject &annot)
{
    annot.SetObjectType(AnnotationObject::Line3D);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());

    annot.SetPosition(lineSource->GetPoint1());
    annot.SetPosition2(lineSource->GetPoint2());

    annot.SetColor1(lineColor);
    annot.SetUseForegroundForTextColor(useForegroundForLineColor);

    MapNode opts;
    opts["width"] = (int)lineActor->GetProperty()->GetLineWidth()-1;
    opts["lineType"] = lineType;
    int ns = tubeFilter->GetNumberOfSides();
    if (ns ==3)
        ns = 0; // low quality
    if (ns ==9)
        ns = 1; // medium quality
    if (ns ==15)
        ns = 2; // high quality
    opts["tubeQuality"] = ns;
    opts["tubeRadius"] = tubeFilter->GetRadius();

    opts["arrow1"] = useArrow1;
    opts["arrow1Resolution"] = arrow1Source->GetResolution();
    opts["arrow1Radius"] = arrow1Source->GetRadius();
    opts["arrow1Height"] = arrow1Source->GetHeight();

    opts["arrow2"] = useArrow2;
    opts["arrow2Resolution"] = arrow2Source->GetResolution();
    opts["arrow2Radius"] = arrow2Source->GetRadius();
    opts["arrow2Height"] = arrow2Source->GetHeight();

    annot.SetOptions(opts);
}


// ****************************************************************************
// Method: avtLine3DColleague::HasPlots
//
// Purpose:
//   This method is called when the vis window gets some plots. We use this
//   signal to add the lineActor to the renderer.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//
// ****************************************************************************

void
avtLine3DColleague::HasPlots(void)
{
    AddToRenderer();
}


// ****************************************************************************
// Method: avtLine3DColleague::NoPlots
//
// Purpose:
//   This method is called when the vis window has no plots. We use this signal
//   to remove the lineActor from the renderer.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//
// ****************************************************************************

void
avtLine3DColleague::NoPlots(void)
{
    RemoveFromRenderer();
}


// ****************************************************************************
// Method: avtLine3DColleague::SetForegroundColor
//
// Purpose:
//   This method is called when the vis window's foreground color changes.
//
// Arguments:
//   r,g,b : The new foreground color.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:35:47 PDT 2015
//   Add support for arrows and tube.
//
// ****************************************************************************

void
avtLine3DColleague::SetForegroundColor(double r, double g, double b)
{
    if(useForegroundForLineColor)
    {
        lineActor->GetProperty()->SetColor(r, g, b);
        arrow1Actor->GetProperty()->SetColor(r, g, b);
        arrow2Actor->GetProperty()->SetColor(r, g, b);
    }
}


