// ************************************************************************* //
//                                  avtLegend.C                              //
// ************************************************************************* //

#include <avtLegend.h>

#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>


// ****************************************************************************
//  Method: avtLegend constructor
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2000
//
// ****************************************************************************

avtLegend::avtLegend()
{
    legend   = NULL;
    legendOn = true;
    currentlyDrawn = false;
    position[0] = position[1] = 0.;
    size[0] = size[1] = 0.;
    renderer = NULL;
}


// ****************************************************************************
//  Method: avtLegend destructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
// ****************************************************************************

avtLegend::~avtLegend()
{
    if (currentlyDrawn)
    {
        Remove();
    }
    if (legend != NULL)
    {
        legend->Delete();
        legend = NULL;
    }
}


// ****************************************************************************
//  Method: avtLegend::Add
//
//  Purpose:
//      The legend gets added to a separate renderer, so just return.  This is
//      overloaded so the base method won't try to add the legend to this
//      renderer.
//
//  Programmer:  Hank Childs
//  Creation:    October 4, 2000
//
// ****************************************************************************

void
avtLegend::Add(vtkRenderer *ren)
{
    if (legendOn && !currentlyDrawn)
    {
        ren->AddActor2D(legend);
        currentlyDrawn = true;
    }
    renderer = ren;
}


// ****************************************************************************
//  Method: avtLegend::Remove
//
//  Purpose:
//      Removes the legend from its renderer.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
// ****************************************************************************

void
avtLegend::Remove(void)
{
    if (currentlyDrawn)
    {
        renderer->RemoveActor2D(legend);
        renderer = NULL;
        currentlyDrawn = false;
    }
}


// ****************************************************************************
//  Method: avtLegend::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color of the legend.
//
//  Arguments:
//      col       The new color.
//
//  Programmer:   Hank Childs
//  Creation:     January 4, 2000
//
// ****************************************************************************

void
avtLegend::SetForegroundColor(const float col[3])
{
    if (legend != NULL)
    {
        //
        // VTK doesn't have all of its const's set up right, so let's help it
        // out.
        //
        float  r = col[0];
        float  g = col[1];
        float  b = col[2];
        legend->GetProperty()->SetColor(r, g, b);
    }
}


// ****************************************************************************
//  Method: avtLegend::LegendOn
//
//  Purpose:
//      Tells the legend that it is on.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2000
//
// ****************************************************************************

void
avtLegend::LegendOn(void)
{
    legendOn = true;

    //
    // If we were added previously and did not add the legend, do that now.
    //
    if (!currentlyDrawn && renderer != NULL)
    {
        Add(renderer);
    }
}


// ****************************************************************************
//  Method: avtLegend::LegendOff
//
//  Purpose:
//      Tells the legend that it is off.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2000
//
// ****************************************************************************

void
avtLegend::LegendOff(void)
{
    legendOn = false;

    //
    // We may have already drawn the renderer, so undo that now.
    //
    if (currentlyDrawn)
    {
        Remove();
    }
}


// ****************************************************************************
//  Method: avtLegend::GetLegendPosition
//
//  Purpose:
//      Gets the legend's position.
//
//  Arguments:
//      px      The x-position of the legend.
//      py      The y-position of the legend.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
// ****************************************************************************

void
avtLegend::GetLegendPosition(float &px, float &py)
{
    px = position[0];
    py = position[1];
}


// ****************************************************************************
//  Method: avtLegend::SetLegendPosition
//
//  Purpose:
//      Sets the legend's position.
//
//  Arguments:
//      px      The x-position of the legend.
//      py      The y-position of the legend.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
// ****************************************************************************

void
avtLegend::SetLegendPosition(float px, float py)
{
    position[0] = px;
    position[1] = py;
    ChangePosition(px, py);
}


// ****************************************************************************
//  Method: avtLegend::GetLegendSize
//
//  Purpose:
//      Gets the legend's size.
//
//  Arguments:
//      w        The legend's width.
//      h        The legend's height.
//
//  Programmer:  Hank Childs
//  Creation:    October 4, 2000
//
// ****************************************************************************

void
avtLegend::GetLegendSize(float &w, float &h)
{
    w = size[0];
    h = size[1];
}


