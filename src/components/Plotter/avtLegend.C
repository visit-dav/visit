// ************************************************************************* //
//                                  avtLegend.C                              //
// ************************************************************************* //

#include <avtLegend.h>

#include <vtkVerticalScalarBarActor.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>


// ****************************************************************************
//  Method: avtLegend constructor
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2000
//
//  Modifications:
//
//    Eric Brugger, Mon Jul 14 15:50:29 PDT 2003
//    Added fontHeight, title, databaseInfo, varName and message.
//
//    Brad Whitlock, Tue Jul 20 16:41:11 PST 2004
//    Added varUnits.
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

    fontHeight = 0.015;
    title = NULL;
    databaseInfo = NULL;
    varName = NULL;
    varUnits = NULL;
    message = NULL;
}


// ****************************************************************************
//  Method: avtLegend destructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//
//    Eric Brugger, Mon Jul 14 15:50:29 PDT 2003
//    Added minFontSize, fontHeight, title, databaseInfo, varName and message.
//
//    Brad Whitlock, Tue Jul 20 16:41:43 PST 2004
//    Added varUnits.
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
    if (title != NULL)
    {
        delete [] title;
    }
    if (databaseInfo != NULL)
    {
        delete [] databaseInfo;
    }
    if (varName != NULL)
    {
        delete [] varName;
    }
    if (varUnits != NULL)
    {
        delete [] varUnits;
    }
    if (message != NULL)
    {
        delete [] message;
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
//  Modifications:
//    Eric Brugger, Thu Jul 17 08:14:46 PDT 2003
//    Added maxSize argument.  It is unused in this routine so it is unnamed.
//
// ****************************************************************************

void
avtLegend::GetLegendSize(float, float &w, float &h)
{
    w = size[0];
    h = size[1];
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
//  Method: avtLegend::GetLegendOn
//
//  Purpose:
//      Return the LegendOn flag.
//
//  Returns:    The LegendOn flag.
//
//  Programmer: Eric Brugger
//  Creation:   July 14, 2003
//
// ****************************************************************************

bool
avtLegend::GetLegendOn(void) const
{
    return legendOn;
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
//  Method: avtLegend::SetFontHeight
//
//  Purpose:
//      Sets the font height as a fraction of the viewport.
//
//  Arguments:
//      height    The new font height.
//
//  Programmer:   Eric Brugger
//  Creation:     July 14, 2003
//
// ****************************************************************************

void
avtLegend::SetFontHeight(float height)
{
    fontHeight = height;
}


// ****************************************************************************
//  Method: avtLegend::SetTitle
//
//  Purpose:
//      Sets the title.
//
//  Arguments:
//      str       The new title.
//
//  Programmer:   Eric Brugger
//  Creation:     July 14, 2003
//
// ****************************************************************************

void
avtLegend::SetTitle(const char *str)
{
    if (title != NULL) delete [] title;

    if (str != NULL)
    {
        title = new char[strlen(str)+1];
        strcpy(title, str);
    }
    else
    {
        title = NULL;
    }
}


// ****************************************************************************
//  Method: avtLegend::SetDatabaseInfo
//
//  Purpose:
//      Sets the database information.
//
//  Arguments:
//      str       The new database information.
//
//  Programmer:   Eric Brugger
//  Creation:     July 14, 2003
//
// ****************************************************************************

void
avtLegend::SetDatabaseInfo(const char *str)
{
    if (databaseInfo != NULL) delete [] databaseInfo;

    if (str != NULL)
    {
        databaseInfo = new char[strlen(str)+1];
        strcpy(databaseInfo, str);
    }
    else
    {
        databaseInfo = NULL;
    }
}


// ****************************************************************************
//  Method: avtLegend::SetVarName
//
//  Purpose:
//      Sets the variable name.
//
//  Arguments:
//      str       The new variable name.
//
//  Programmer:   Eric Brugger
//  Creation:     July 14, 2003
//
// ****************************************************************************

void
avtLegend::SetVarName(const char *str)
{
    if (varName != NULL) delete [] varName;

    if (str != NULL)
    {
        varName = new char[strlen(str)+1];
        strcpy(varName, str);
    }
    else
    {
        varName = NULL;
    }
}

// ****************************************************************************
// Method: avtLegend::SetVarUnits
//
// Purpose: 
//   Sets the variable units.
//
// Arguments:
//   str : The units.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 16:46:40 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetVarUnits(const char *str)
{
    if (varUnits != NULL) delete [] varUnits;

    if (str != NULL)
    {
        varUnits = new char[strlen(str)+1];
        strcpy(varUnits, str);
    }
    else
    {
        varUnits = NULL;
    }
}

// ****************************************************************************
//  Method: avtLegend::SetMessage
//
//  Purpose:
//      Sets the message.
//
//  Arguments:
//      str       The new message.
//
//  Programmer:   Eric Brugger
//  Creation:     July 14, 2003
//
// ****************************************************************************

void
avtLegend::SetMessage(const char *str)
{
    if (message != NULL) delete [] message;

    if (str != NULL)
    {
        message = new char[strlen(str)+1];
        strcpy(message, str);
    }
    else
    {
        message = NULL;
    }
}


// ****************************************************************************
//  Method: avtLegend::Update
//
//  Purpose:
//      Update the legend.
//
//  Programmer:   Eric Brugger
//  Creation:     July 14, 2003
//
//  Modifications:
//    Brad Whitlock, Tue Jul 20 16:48:56 PST 2004
//    Added varUnits.
//
// ****************************************************************************

void
avtLegend::Update()
{
    //
    // Set the font size.
    //
    legend->SetFontHeight(fontHeight);

    //
    // Set the title.
    //
    int len = 0;
    if (title != NULL)        len += strlen(title) + 1;
    if (databaseInfo != NULL) len += strlen(databaseInfo) + 1;
    if (varName != NULL)      len += strlen(varName) + 6;
    if (varUnits != NULL)     len += strlen(varUnits) + 8;
    if (message != NULL)      len += strlen(message) + 1;

    if (len != 0)
    {
        char *str = new char[len];
        char *tmpstr = str;
        str[0] = '\0';

        if (title != NULL)
        {
            strcpy(tmpstr, title);
            tmpstr += strlen(tmpstr);
        }
        if (databaseInfo != NULL)
        {
            strcpy(tmpstr, "\n");
            strcat(tmpstr, databaseInfo);
            tmpstr += strlen(tmpstr);
        }
        if (varName != NULL)
        {
            strcpy(tmpstr, "\nVar: ");
            strcat(tmpstr, varName);
            tmpstr += strlen(tmpstr);
        }
        if (varUnits != NULL)
        {
            strcpy(tmpstr, "\nUnits: ");
            strcat(tmpstr, varUnits);
            tmpstr += strlen(tmpstr);
        }
        if (message != NULL)
        {
            strcpy(tmpstr, "\n");
            strcat(tmpstr, message);
        }

        legend->SetTitle(str);
        delete [] str;
    }
}


