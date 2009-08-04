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
//                        avtMultiCurveLabelMapper.C                         //
// ************************************************************************* //

#include <avtMultiCurveLabelMapper.h>

#include <avtLabelActor.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkDataSet.h>
#include <BadIndexException.h>
#include <ColorAttribute.h>

const double INV_255 = 0.0039215686274509803377;

// ****************************************************************************
//  Method: avtMultiCurveLabelMapper constructor
//
//  Programmer:   Eric Brugger
//  Creation:     December 12, 2008
//
//  Modifications:
//    Eric Brugger, Wed Feb 18 12:05:44 PST 2009
//    I added the ability to display identifiers at each of the points.
//
// ****************************************************************************

avtMultiCurveLabelMapper::avtMultiCurveLabelMapper()
{
    markerVisibility = true;
    idVisibility     = false;
    scale            = 0.05;
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper destructor
//
//  Programmer:   Eric Brugger
//  Creation:     December 12, 2008
//
// ****************************************************************************

avtMultiCurveLabelMapper::~avtMultiCurveLabelMapper()
{
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows this mapper to maintain
//      a correct state. 
//
//  Programmer:   Eric Brugger
//  Creation:     December 12, 2008
//
//  Modifications:
//    Eric Brugger, Wed Feb 18 12:05:44 PST 2009
//    I added the ability to display identifiers at each of the points.
//
// ****************************************************************************

void
avtMultiCurveLabelMapper::CustomizeMappers(void)
{
    SetMarkerVisibility(markerVisibility);
    SetIdVisibility(idVisibility);
    SetScale(scale);
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper::SetDatasetInput
//
//  Purpose:
//      Allows the labeled curve mapper to operator on the input dataset. 
//
//  Arguments:
//      ds        The upstream dataset.
//      inNum     The input number.
//
//  Programmer:   Eric Brugger
//  Creation:     December 12, 2008
//
//  Modifications:
//    Eric Brugger, Wed Feb 18 12:05:44 PST 2009
//    I added the ability to display identifiers at each of the points.
//
//    Eric Brugger, Mon Mar  9 17:59:58 PDT 2009
//    I enhanced the plot so that the markers and identifiers displayed for
//    the points are in the same color as the curve, instead of always in
//    black.
//
// ****************************************************************************

void
avtMultiCurveLabelMapper::SetDatasetInput(vtkDataSet *ds, int inNum)
{
    if (ds == NULL || ds->GetNumberOfPoints() == 0 ||
        ds->GetNumberOfCells() == 0)
    {
        return;
    }

    if (inNum < 0)
    {
        EXCEPTION2(BadIndexException, inNum, 10);
    }

    double col[4];
    GetLevelColor(inNum, col);

    vtkIntArray *intArray = vtkIntArray::SafeDownCast(
        ds->GetPointData()->GetArray("CurveSymbols"));
    int *buf = (intArray == NULL) ? NULL : intArray->GetPointer(0);

    vtkIntArray *intArray2 = vtkIntArray::SafeDownCast(
        ds->GetPointData()->GetArray("CurveIds"));
    int *buf2 = (intArray2 == NULL) ? NULL : intArray2->GetPointer(0);

    double    pos[3];        
    for (vtkIdType i = 0; i < ds->GetNumberOfPoints(); i++)
    {
        // Add the marker.
        avtLabelActor_p la = new avtLabelActor;
        ds->GetPoint(i, pos);
        la->SetAttachmentPoint(pos);
        if (buf != NULL)
            la->SetMarker(buf[i]);
        else
            la->SetMarker(2);
        la->SetScale(scale);
        la->SetForegroundColor(col[0], col[1], col[2], col[3]);
        actors.push_back(la);
        colors.push_back(inNum);

        // Add the id.
        la = new avtLabelActor;
        ds->GetPoint(i, pos);
        la->SetAttachmentPoint(pos);
        char label[16];
        if (buf2 != NULL)
            sprintf(label, "%d", buf2[i]);
        else
            sprintf(label, "%d", i);
        la->SetDesignator(label);
        la->SetScale(scale);
        la->SetForegroundColor(col[0], col[1], col[2], col[3]);
        actors.push_back(la);
        colors.push_back(inNum);
    }
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper::SetScale
//
//  Purpose:
//      Sets the scale of each label.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:   Eric Brugger
//  Creation:     December 12, 2008
//
// ****************************************************************************

void
avtMultiCurveLabelMapper::SetScale(double s)
{
    scale = s;
    for (int i = 0; i < actors.size(); i++)
    {
        actors[i]->SetScale(s);
    }
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper::SetMarkerVisibility
//
//  Purpose:
//      Sets the visibility for the marker actors. 
//
//  Arguments:
//      labelsOn  The new visibility state.
//
//  Programmer:   Eric Brugger
//  Creation:     February 18, 2009
//
// ****************************************************************************

void
avtMultiCurveLabelMapper::SetMarkerVisibility(bool labelsOn)
{
   markerVisibility = labelsOn;
   if (labelsOn)
   {
       for (int i = 0; i < actors.size(); i += 2)
       {
           actors[i]->UnHide();
       }
   } 
   else 
   {
       for (int i = 0; i < actors.size(); i += 2)
       {
           actors[i]->Hide();
       }
   } 
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper::SetIdVisibility
//
//  Purpose:
//      Sets the visibility for the id actors. 
//
//  Arguments:
//      labelsOn  The new visibility state.
//
//  Programmer:   Eric Brugger
//  Creation:     February 18, 2009
//
// ****************************************************************************

void
avtMultiCurveLabelMapper::SetIdVisibility(bool labelsOn)
{
   idVisibility = labelsOn;
   if (labelsOn)
   {
       for (int i = 1; i < actors.size(); i += 2)
       {
           actors[i]->UnHide();
       }
   } 
   else 
   {
       for (int i = 1; i < actors.size(); i += 2)
       {
           actors[i]->Hide();
       }
   } 
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper::SetColors
//
//  Purpose:
//      Set the color attribute list used for colormapping the labels.
//
//  Arguments:
//      c         The new color attribute list.
//
//  Programmer:   Eric Brugger
//  Creation:     March 9, 2009
//
// ****************************************************************************

void
avtMultiCurveLabelMapper::SetColors(const ColorAttributeList &c)
{
    cal = c;
    for (int i = 0; i < actors.size(); i++)
    {
        double col[4];
        GetLevelColor(colors[i], col);
        actors[i]->SetForegroundColor(col[0], col[1], col[2], col[3]);
    }
}


// ****************************************************************************
//  Method: avtMultiCurveLabelMapper::GetLevelColor
//
//  Purpose:
//      Retrieves the color associated with levelNum.
//
//  Arguments:
//      levelNum  The level number.
//      col       A place to store the color.
//
//  Programmer:   Eric Brugger
//  Creation:     March 9, 2009
//
// ****************************************************************************

void
avtMultiCurveLabelMapper::GetLevelColor(const int levelNum, double col[4])
{
    int nc = cal.GetNumColors();
    if (nc == 1)  // constant color for all levels
    {
        col[0] = cal[0].Red()   * INV_255;
        col[1] = cal[0].Green() * INV_255;
        col[2] = cal[0].Blue()  * INV_255;
        col[3] = cal[0].Alpha() * INV_255;
        return;
    }

    if (levelNum < 0 || levelNum >= nc)
    {
        EXCEPTION2(BadIndexException, levelNum, nc);
    }

    col[0] = cal[levelNum].Red()   * INV_255;
    col[1] = cal[levelNum].Green() * INV_255;
    col[2] = cal[levelNum].Blue()  * INV_255;
    col[3] = cal[levelNum].Alpha() * INV_255;
}
