/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                         avtLabeledCurveMapper.C                           //
// ************************************************************************* //

#include <avtLabeledCurveMapper.h>

#include <avtLabelActor.h>
#include <vtkMaskPoints.h>
#include <vtkPolyData.h>
#include <BadIndexException.h>



// ****************************************************************************
//  Method: avtLabeledCurveMapper constructor
//
//  Arguments:
//      l       The label this mapper should use.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul 23 15:01:55 PDT 2002 
//    Initialize new member labelVis.
//
// ****************************************************************************

avtLabeledCurveMapper::avtLabeledCurveMapper()
{
    label         = " ";
    labelVis      = true;
    scale         = 0.2;
    filter        = 0;
    nFilters      = 0;
    labelColor[0] = labelColor[1] = labelColor[2] = 0.; // black
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

avtLabeledCurveMapper::~avtLabeledCurveMapper()
{
    if (filter != NULL)
    {
        for (int i = 0 ; i < nFilters ; i++)
        {
            if (filter[i] != NULL)
            {
                filter[i]->Delete();
            }
        }
        delete [] filter;
    }
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows this mapper to maintain
//      a correct state. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul 23 15:01:55 PDT 2002
//    Added call to SetLabelVisibility.
//
// ****************************************************************************

void
avtLabeledCurveMapper::CustomizeMappers(void)
{
    SetLabelColor(labelColor);
    SetLabelVisibility(labelVis);
    SetScale(scale);
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper::SetUpFilters
//
//  Purpose:
//      This is called before SetDatasetInput to allow for initialization work.
//      Called from within the parent class' SetUpMappers method.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtLabeledCurveMapper::SetUpFilters(int nInputs)
{
    if (filter != NULL)
    {
        for (int i = 0 ; i < nFilters ; i++)
        {
            if (filter[i] != NULL)
            {
                filter[i]->Delete();
            }
        }
        delete [] filter;
    }

    nFilters     = nInputs;
    filter       = new vtkMaskPoints*[nFilters];
    for (int i = 0 ; i < nFilters ; i++)
    {
        filter[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper::SetDatasetInput
//
//  Purpose:
//      Allows the labeled curve mapper to operator on the input dataset. 
//
//  Arguments:
//      ds        The upstream dataset.
//      inNum     The input number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 15 13:33:52 PDT 2003
//    Don't assume input contains cells and/or points.
//
// ****************************************************************************

void
avtLabeledCurveMapper::SetDatasetInput(vtkDataSet *ds, int inNum)
{
    if (ds == NULL || ds->GetNumberOfPoints() == 0 ||
        ds->GetNumberOfCells() == 0)
    {
        return;
    }

    if (inNum < 0 || inNum >= nFilters)
    {
        EXCEPTION2(BadIndexException, inNum, nFilters);
    }

    if (filter[inNum] == NULL)
    {
        filter[inNum] = vtkMaskPoints::New();
    }

    filter[inNum]->SetInput(ds);
    filter[inNum]->SetOnRatio(ds->GetNumberOfPoints()/5);
    filter[inNum]->GetOutput()->Update();
    vtkPoints *points = filter[inNum]->GetOutput()->GetPoints();
    double pos[3];        
    vtkIdType i;
    for (i = 0; i < points->GetNumberOfPoints(); i++)
    {
        points->GetPoint(i, pos);
        avtLabelActor_p la = new avtLabelActor;
        la->SetAttachmentPoint(pos);
        la->SetDesignator(label.c_str());
        la->SetForegroundColor(labelColor);
        la->SetScale(scale);
        actors.push_back(la);
    }
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper::SetScale
//
//  Purpose:
//      Sets the scale of each label.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void
avtLabeledCurveMapper::SetScale(double s)
{
    scale = s;
    for (int i = 0; i < actors.size(); i++)
    {
        actors[i]->SetScale(s);
    }
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper::SetLabelColor
//
//  Purpose:
//      Tells the label actors what color the labels should be. 
//
//  Arguments:
//      col       The new color.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 12, 2002
//
// ****************************************************************************

void
avtLabeledCurveMapper::SetLabelColor(double col[3])
{
    labelColor[0] = col[0];
    labelColor[1] = col[1];
    labelColor[2] = col[2];
    for (int i = 0; i < actors.size(); i++)
    {
        actors[i]->SetForegroundColor(labelColor);
    }
}



// ****************************************************************************
//  Method: avtLabeledCurveMapper::SetLabelColor
//
//  Purpose:
//      Tells the label actors what color the labels should be. 
//
//  Arguments:
//      r         The new color's red component.
//      g         The new color's green component.
//      b         The new color's blue component.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 12, 2002
//
// ****************************************************************************

void
avtLabeledCurveMapper::SetLabelColor(double r, double g, double b)
{
    labelColor[0] = r; 
    labelColor[1] = g; 
    labelColor[2] = b; 
    for (int i = 0; i < actors.size(); i++)
    {
        actors[i]->SetForegroundColor(labelColor);
    }
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper::SetLabel
//
//  Purpose:
//      Sets the label to be used by this mapper. 
//
//  Arguments:
//      l         The new label.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 12, 2002
//
// ****************************************************************************

void
avtLabeledCurveMapper::SetLabel(std::string &l)
{
    label = l;
}


// ****************************************************************************
//  Method: avtLabeledCurveMapper::SetLabelVisibility
//
//  Purpose:
//      Sets the visibility for the label actors. 
//
//  Arguments:
//      labelsOn  The new visibility state.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul 23 15:01:55 PDT 2002
//    Update new member labelVis.
//
// ****************************************************************************

void
avtLabeledCurveMapper::SetLabelVisibility(bool labelsOn)
{
   labelVis = labelsOn;
   if (labelsOn)
   {
       for (int i = 0; i < actors.size(); i++)
       {
           actors[i]->UnHide();
       }
   } 
   else 
   {
       for (int i = 0; i < actors.size(); i++)
       {
           actors[i]->Hide();
       }
   } 
}

