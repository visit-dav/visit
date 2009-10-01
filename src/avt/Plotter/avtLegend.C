/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//  Modifications:
//
//    Eric Brugger, Mon Jul 14 15:50:29 PDT 2003
//    Added fontHeight, title, databaseInfo, varName and message.
//
//    Brad Whitlock, Tue Jul 20 16:41:11 PST 2004
//    Added varUnits.
//
//    Kathleen Bonnell, Thu Aug 12 13:07:29 PDT 2004 
//    Initialize globalVisibility.
//
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Init orientation member
//
// ****************************************************************************

avtLegend::avtLegend()
{
    legend   = NULL;
    legendOn = true;
    globalVisibility = true;
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
    orientation = VerticalTextOnRight;
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
//  Modifications:
//    Kathleen Bonnell, Thu Aug 12 13:07:29 PDT 2004
//    Only add if globalVisibility is true.
//
// ****************************************************************************

void
avtLegend::Add(vtkRenderer *ren)
{
    if (globalVisibility && legendOn && !currentlyDrawn)
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
avtLegend::GetLegendPosition(double &px, double &py)
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
avtLegend::SetLegendPosition(double px, double py)
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
avtLegend::GetLegendSize(double, double &w, double &h)
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
//  Method: avtLegend::GetCurrentlyDrawn
//
//  Purpose:
//      Return the currentlyDrawn flag.
//
//  Returns:    The currentlyDrawn flag.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Mar 20 16:50:27 PST 2007
//
// ****************************************************************************

bool
avtLegend::GetCurrentlyDrawn() const
{
    return currentlyDrawn;
}

// ****************************************************************************
// Method: avtLegend::SetTitleVisibility
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetTitleVisibility(bool)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::GetTitleVisibility
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtLegend::GetTitleVisibility() const
{
    return true;
}

// ****************************************************************************
// Method: avtLegend::SetLabelVisibility
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//    Kathleen Bonnell, Thu Oct  1 14:19:27 PDT 2009
//    Changed arg from bool to int, to support multiple modes.
//   
// ****************************************************************************

void
avtLegend::SetLabelVisibility(int)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::GetLabelVisibility
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//    Kathleen Bonnell, Thu Oct  1 14:19:27 PDT 2009
//    Changed retrun type from bool to int, to support multiple modes.
//   
// ****************************************************************************

int
avtLegend::GetLabelVisibility() const
{
    return 1;
}

// ****************************************************************************
// Method: avtLegend::SetMinMaxVisibility
//
// Programmer: Hank Childs
// Creation:   January 23, 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetMinMaxVisibility(bool)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::GetMinMaxVisibility
//
// Programmer: Hank Childs
// Creation:   January 23, 2009
//
// Modifications:
//   
// ****************************************************************************

bool
avtLegend::GetMinMaxVisibility() const
{
    return true;
}

// ****************************************************************************
// Method: avtLegend::SetNumberFormat
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetNumberFormat(const char *)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::SetLegendScale
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetLegendScale(double xScale, double yScale)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::SetBoundingBoxVisibility
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetBoundingBoxVisibility(bool)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::SetBoundingBoxColor
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetBoundingBoxColor(const double *)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::SetOrientation
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    This was a noop.  Now, orientation is set.
//   
// ****************************************************************************

void
avtLegend::SetOrientation(LegendOrientation o)
{
    orientation = o;
}

// ****************************************************************************
// Method: avtLegend::SetFont
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:09:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetFont(int family, bool bold, bool italic, bool shadow)
{
    // Do nothing
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
avtLegend::SetForegroundColor(const double col[3])
{
    if (legend != NULL)
    {
        //
        // VTK doesn't have all of its const's set up right, so let's help it
        // out.
        //
        double  r = col[0];
        double  g = col[1];
        double  b = col[2];
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
avtLegend::SetFontHeight(double height)
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
//    Kathleen Bonnell, Tue Oct 25 11:13:14 PDT 2005 
//    Call ChangeFontHeight/ChangeTitle instead of legend->SetFontHeight/
//    legend->SetTitle.
//
// ****************************************************************************

void
avtLegend::Update()
{
    //
    // Set the font size.
    //
    ChangeFontHeight(fontHeight);

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
        ChangeTitle(str);
        delete [] str;
    }
}


// ****************************************************************************
//  Method: avtLegend::SetGlobalVisibility
//
//  Purpose:
//    Sets the state global legend visibility. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2004 
//
// ****************************************************************************

void
avtLegend::SetGlobalVisibility(bool v)
{
    globalVisibility = v;
}


// ****************************************************************************
// Method: avtLegend::SetNumTicks
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void                          
avtLegend::SetNumTicks(int)
{
    // Do nothing
}


// ****************************************************************************
// Method: avtLegend::SetUseSuppliedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetUseSuppliedLabels(bool)
{
    // Do nothing
}


// ****************************************************************************
// Method: avtLegend::SetMinMaxInclusive
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetMinMaxInclusive(bool)
{
    // Do nothing
}


// ****************************************************************************
// Method: avtLegend::SetSuppliedValues
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetSuppliedValues(const doubleVector &)
{
    // Do nothing
}


// ****************************************************************************
// Method: avtLegend::SetSuppliedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::SetSuppliedLabels(const stringVector &)
{
    // Do nothing
}


// ****************************************************************************
// Method: avtLegend::GetCalculatedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::GetCalculatedLabels(doubleVector &)
{
    // Do nothing
}

// ****************************************************************************
// Method: avtLegend::GetCalculatedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtLegend::GetCalculatedLabels(stringVector &)
{
    // Do nothing
}
