// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtWellBoreNameMapper.C                           //
// ************************************************************************* //

#include <avtWellBoreNameMapper.h>

#include <vtkCharArray.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <avtLabelActor.h>


// ****************************************************************************
//  Method: avtWellBoreNameMapper constructor
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

avtWellBoreNameMapper::avtWellBoreNameMapper()
{
    labelVis      = true;
    scale         = 0.2;
    labelColor[0] = labelColor[1] = labelColor[2] = 0.; // black
}


// ****************************************************************************
//  Method: avtWellBoreNameMapper destructor
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

avtWellBoreNameMapper::~avtWellBoreNameMapper()
{
}


// ****************************************************************************
//  Method: avtWellBoreNameMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows this mapper to maintain
//      a correct state. 
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

void
avtWellBoreNameMapper::CustomizeMappers(void)
{
    SetLabelColor(labelColor);
    SetLabelVisibility(labelVis);
    SetScale(scale);
}


// ****************************************************************************
//  Method: avtWellBoreNameMapper::SetDatasetInput
//
//  Purpose:
//      Allows the well bore name mapper to operator on the input dataset. 
//
//  Arguments:
//      ds        The upstream dataset.
//      inNum     The input number.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

void
avtWellBoreNameMapper::SetDatasetInput(vtkDataSet *ds, int inNum)
{
    if (ds == NULL || ds->GetNumberOfPoints() == 0 ||
        ds->GetNumberOfCells() == 0)
    {
        return;
    }

    //
    // Get the label to display.
    //
    vtkDataArray *arr = ds->GetFieldData()->GetArray("well_name");
    if (arr == NULL)
    {
        return;
    }
    char *name = vtkCharArray::SafeDownCast(arr)->GetPointer(0);

    //
    // Display the label.
    //
    double pos[3];        

    ds->GetPoint(0, pos);
    avtLabelActor_p la = new avtLabelActor;
    la->SetAttachmentPoint(pos);
    la->SetDesignator(name);
    la->SetForegroundColor(labelColor);
    la->SetScale(scale);
    actors.push_back(la);
}


// ****************************************************************************
//  Method: avtWellBoreNameMapper::SetScale
//
//  Purpose:
//      Sets the scale for the label actors.
//
//  Arguments:
//      s         The new scale.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

void
avtWellBoreNameMapper::SetScale(double s)
{
    scale = s;
    for (size_t i = 0; i < actors.size(); i++)
    {
        actors[i]->SetScale(s);
    }
}


// ****************************************************************************
//  Method: avtWellBoreNameMapper::SetLabelColor
//
//  Purpose:
//      Sets the color for the label actors. 
//
//  Arguments:
//      col       The new color.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

void
avtWellBoreNameMapper::SetLabelColor(double col[3])
{
    labelColor[0] = col[0];
    labelColor[1] = col[1];
    labelColor[2] = col[2];
    for (size_t i = 0; i < actors.size(); i++)
    {
        actors[i]->SetForegroundColor(labelColor);
    }
}


// ****************************************************************************
//  Method: avtWellBoreNameMapper::SetLabelColor
//
//  Purpose:
//      Sets the color for the label actors. 
//
//  Arguments:
//      r         The new color's red component.
//      g         The new color's green component.
//      b         The new color's blue component.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

void
avtWellBoreNameMapper::SetLabelColor(double r, double g, double b)
{
    labelColor[0] = r; 
    labelColor[1] = g; 
    labelColor[2] = b; 
    for (size_t i = 0; i < actors.size(); i++)
    {
        actors[i]->SetForegroundColor(labelColor);
    }
}


// ****************************************************************************
//  Method: avtWellBoreNameMapper::SetLabelVisibility
//
//  Purpose:
//      Sets the visibility for the label actors. 
//
//  Arguments:
//      labelsOn  The new visibility state.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008
//
// ****************************************************************************

void
avtWellBoreNameMapper::SetLabelVisibility(bool labelsOn)
{
   labelVis = labelsOn;
   if (labelsOn)
   {
       for (size_t i = 0; i < actors.size(); i++)
       {
           actors[i]->UnHide();
       }
   } 
   else 
   {
       for (size_t i = 0; i < actors.size(); i++)
       {
           actors[i]->Hide();
       }
   } 
}

