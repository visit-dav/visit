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

#include <avtLabelActor.h>

#include <vtkCellArray.h>
#include <vtkFollower.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkVectorText.h>


// ****************************************************************************
//  Constructor:  avtLabelActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

avtLabelActor:: avtLabelActor()
{
    attach[0] = attach[1] = attach[2] = 0.;

    labelActor = vtkFollower::New(); 
        labelActor->GetProperty()->SetColor(0., 0., 0.);
        labelActor->SetScale(0.5);
        labelActor->PickableOff();

    renderer = NULL; 
}


// ****************************************************************************
//  Destructor:  avtLabelActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

avtLabelActor::~avtLabelActor()
{
    Remove();
    if (labelActor != NULL)
    {
        labelActor->Delete();
        labelActor = NULL;
    }
}


// ****************************************************************************
//  Method:  avtLabelActor::Add
//
//  Purpose:  Add actors to the renderer, and renderer to this class.
//
//  Arguments:
//    ren     The renderer to use.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::Add(vtkRenderer *ren)
{
    renderer = ren;
    labelActor->SetCamera(renderer->GetActiveCamera());
    renderer->AddActor(labelActor);
}


// ****************************************************************************
//  Method:  avtLabelActor::Remove
//
//  Purpose:  Remove actors from the renderer, and renderer from this class.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::Remove()
{
    if (renderer != NULL)
    {
        renderer->RemoveActor(labelActor);
        renderer = NULL;
    }
}


// ****************************************************************************
//  Method:  avtLabelActor::Shift
//
//  Purpose:  Shift the actors by the specified vector.
//
//  Arguments:
//    vec     The vector to use for shifting.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::Shift(const double vec[3])
{
    double newPos[3];
    newPos[0] = attach[0] * vec[0];
    newPos[1] = attach[1] * vec[1];
    newPos[2] = attach[2] * vec[2] ;
    labelActor->SetPosition(newPos[0], newPos[1], newPos[2]);
}

// ****************************************************************************
//  Method:  avtLabelActor::SetAttachmentPoint
//
//  Purpose: Set actors' postion.
//
//  Arguments:
//    pos    The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::SetAttachmentPoint(const double pos[3])
{
    attach[0] = pos[0];
    attach[1] = pos[1];
    attach[2] = pos[2];
    labelActor->SetPosition(pos[0], pos[1], pos[2]);
}


// ****************************************************************************
//  Method:  avtLabelActor::SetScale
//
//  Purpose:  Set the scale for labelActor.
//
//  Arguments:
//    s       The scale factor to use.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void
avtLabelActor::SetScale(double s)
{
    labelActor->SetScale(s);
}


// ****************************************************************************
//  Method:  avtLabelActor::SetDesignator
//
//  Purpose:  Sets the designator text for this actor. 
//
//  Arguments:
//    l       The designator string.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::SetDesignator(const char *l)
{
    vtkVectorText *vecText = vtkVectorText::New();
    vecText->SetText(l);

    vtkPolyDataMapper *labelMapper = vtkPolyDataMapper::New();
    labelMapper->SetInput(vecText->GetOutput());

    labelActor->SetMapper(labelMapper);

    labelMapper->Delete();
    vecText->Delete();
}


// ****************************************************************************
//  Method:  avtLabelActor::SetMarker
//
//  Purpose:  Sets the marker for this actor. 
//
//  Arguments:
//    index   The index of the marker.
//
//  Programmer:  Eric Brugger
//  Creation:    December 9, 2008 
//
// ****************************************************************************

void avtLabelActor::SetMarker(const int index)
{
    //
    // Create the poly data for the marker.
    //
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *lines = vtkCellArray::New();

    double xLine[3];
    vtkIdType vtkPointIDs[2];

    xLine[2] = 0.;
    switch (index)
    {
      //
      // An 'X'.
      //
      case 0:
        xLine[0] = -0.5;
        xLine[1] = -0.5;
        points->InsertNextPoint(xLine);
        xLine[0] =  0.5;
        xLine[1] =  0.5;
        points->InsertNextPoint(xLine);
        xLine[0] =  0.5;
        xLine[1] = -0.5;
        points->InsertNextPoint(xLine);
        xLine[0] = -0.5;
        xLine[1] =  0.5;
        points->InsertNextPoint(xLine);

        vtkPointIDs[0] = 0;
        vtkPointIDs[1] = 1;
        lines->InsertNextCell(2, vtkPointIDs);
        vtkPointIDs[0] = 2;
        vtkPointIDs[1] = 3;
        lines->InsertNextCell(2, vtkPointIDs);
        break;

      case 1:
        //
        // A cirle.
        //
        for (int k = 0; k < 12; k++)
        {
            //
            // The magic number 0.52359878 = 2 * pi / 12.
            //
            xLine[0] = sin(0.52359878 * double(k)) * 0.3;
            xLine[1] = cos(0.52359878 * double(k)) * 0.3;
            points->InsertNextPoint(xLine);
        }

        for (int k = 0; k < 12 - 1; k++)
        {
            vtkPointIDs[0] = k;
            vtkPointIDs[1] = k + 1;
            lines->InsertNextCell(2, vtkPointIDs);
        }
        vtkPointIDs[0] = 11;
        vtkPointIDs[1] = 0;
        lines->InsertNextCell(2, vtkPointIDs);
        break;
    }

    vtkPolyData *polyData =  vtkPolyData::New();

    polyData->SetLines(lines);
    polyData->SetPoints(points);
    lines->Delete();
    points->Delete();

    //
    // Create the actor.
    //
    vtkPolyDataMapper *labelMapper = vtkPolyDataMapper::New();
    labelMapper->SetInput(polyData);

    labelActor->SetMapper(labelMapper);

    labelMapper->Delete();
    polyData->Delete();
}


// ****************************************************************************
//  Method:  avtLabelActor::SetForegroundColor
//
//  Purpose:  Set the actors' color.
//
//  Arguments:
//    fg         The rgb components of the foreground color.
//   
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::SetForegroundColor(double fg[3])
{
    labelActor->GetProperty()->SetColor(fg);
}


// ****************************************************************************
//  Method:  avtLabelActor::SetForegroundColor
//
//  Purpose: Set the actors' color.
//
//  Arguments:
//    r      The red component of the foreground color.
//    g      The green component of the foreground color.
//    b      The blue component of the foreground color.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::SetForegroundColor(double r, double g, double b)
{
    labelActor->GetProperty()->SetColor(r, g, b);
}


// ****************************************************************************
//  Method:  avtLabelActor::SetForegroundColor
//
//  Purpose: Set the actors' color.
//
//  Arguments:
//    r      The red component of the foreground color.
//    g      The green component of the foreground color.
//    b      The blue component of the foreground color.
//    a      The alpha component of the foreground color.
//
//  Programmer:  Eric Brugger
//  Creation:    March 9, 2009 
//
// ****************************************************************************

void avtLabelActor::SetForegroundColor(double r, double g, double b, double a)
{
    labelActor->GetProperty()->SetColor(r, g, b);
    labelActor->GetProperty()->SetOpacity(a);
}


// ****************************************************************************
//  Method:  avtLabelActor::Hide
//
//  Purpose: Make the actors invisible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::Hide()
{
    labelActor->VisibilityOff();
}


// ****************************************************************************
//  Method:  avtLabelActor::UnHide
//
//  Purpose:  Make the actors visible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::UnHide()
{
    labelActor->VisibilityOn();
}


// ****************************************************************************
//  Method:  avtLabelActor::ComputeScaleFactor
//
//  Purpose:  Computes the scale for this actor to use, based on the
//            current settings in the renderer, and the actors position. 
//
//  Notes:    This code was duplicated from /components/VisWindow/Colleagues/
//            VisWinRendering.  If this code is modified in any significant way
//            (change in heuristics, etc.) the code in VisWinRendering should 
//            also be modified.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 19, 2002 
//
// ****************************************************************************

double 
avtLabelActor::ComputeScaleFactor()
{
    if (renderer == NULL)
    {
        return -1.;
    }
    double pos[3];
    labelActor->GetPosition(pos);
 
    double newX = pos[0], newY = pos[1], newZ = pos[2];
    renderer->WorldToView(newX, newY, newZ);
    renderer->ViewToNormalizedViewport(newX, newY, newZ);
    renderer->NormalizedViewportToViewport(newX, newY);
    renderer->ViewportToNormalizedDisplay(newX, newY);

    //
    //  Assuming NormalizedDisplay coordinates run from 0 .. 1 in both
    //  x and y directions, then the normalized dispaly creates a square, whose
    //  diagonal length is sqrt(2) or 1.4142134624.  
    // 
    const double displayDiag = 1.4142135624; 

    // 
    //  We want to find a position P2, that creates a diagonal with the 
    //  original point that is 1/20th of the display diagonal.
    //

    const double target = displayDiag * 0.0125;

    //
    //  Since we are dealing with a right-isosceles-triangle the new position
    //  will be offset in both x and y directions by target * cos(45); 
    //
  
    const double cos45 = 0.7604059656;

    double offset = target * cos45;

    newX += offset;
    newY += offset;

    //
    // Now convert our new position from NormalizedDisplay to World Coordinates.
    //
 
    renderer->NormalizedDisplayToViewport(newX, newY);
    renderer->ViewportToNormalizedViewport(newX, newY);
    renderer->NormalizedViewportToView(newX, newY, newZ);
    renderer->ViewToWorld(newX, newY, newZ);

    //
    // Experimental results, using vtkVectorText and vtkPolyDataMapper, 
    // smallest 'diagonal size' is from the letter 'r' at 0.917883
    // largest  'diagonal size' is from the letter 'W' at 1.78107
    // thus, the average diagonal size of vtkVectorText is:  1.3494765
    // (for alpha text only, in world coordinats, with scale factor of 1.0).  
    // THIS MAY BE A DISPLAY-DEPENDENT RESULT! 
    // 

    const double avgTextDiag = 1.3494765;

    //
    //  Calculate our scale factor, by determining the new target and using
    //  the avgTextDiag to normalize the results across all the pick letters.
    //

    double dxsqr = (newX - pos[0]) * (newX - pos[0]);
    double dysqr = (newY - pos[1]) * (newY - pos[1]);
    double dzsqr = (newZ - pos[2]) * (newZ - pos[2]);
    double worldTarget = sqrt(dxsqr + dysqr + dzsqr); 

    double scale = worldTarget / avgTextDiag;
    SetScale(scale);
    return scale;
}


