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
//                              avtTruecolorFilter.C                         //
// ************************************************************************* //

#include <avtTruecolorFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>



// ****************************************************************************
//  Method: avtTruecolorFilter constructor
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtTruecolorFilter::avtTruecolorFilter()
{
}


// ****************************************************************************
//  Method: avtTruecolorFilter destructor
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtTruecolorFilter::~avtTruecolorFilter()
{
}


// ****************************************************************************
//  Method: avtTruecolorFilter::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
//  Modifications:
//
//     Chris Wojtan Mon Jun 21 15:44 PDT 2004
//     Changed vector dimension from 3D to 4D
//
//     Hank Childs, Wed Nov 10 11:33:59 PST 2004
//     Fix memory leak.
//
//     Hank Childs, Fri May 20 14:52:21 PDT 2005
//     Add support for nodal colors.
//
//     Brad Whitlock, Mon Apr 23 16:35:43 PST 2007
//     Convert data arrays to 4 component tuples if they are not already
//     4-component.
//
// ****************************************************************************

inline unsigned char 
DoubleToColor(const double c)
{
    int ic = (int)c;
    if(ic < 0) ic = 0;
    if(ic > 255) ic = 255;
    return (unsigned char)ic;
}

vtkDataSet *
avtTruecolorFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    // if we do not know the name of the data array to display,
    // we cannot display it
    if (variable_name == NULL)
        return inDS;

    vtkDataSet *outDS = (vtkDataSet *) inDS->NewInstance();
    outDS->ShallowCopy(inDS);

    // convert RGB vectors into a RGBA unsigned char array
    // and use these data as colors
    bool isZonal = true;
    vtkDataArray *vecdata = inDS->GetCellData()->GetArray(variable_name);
    if (vecdata == NULL)
    {
        vecdata = inDS->GetPointData()->GetArray(variable_name);
        if (vecdata == NULL)
            return inDS;
        isZonal = false;
    }

    vtkUnsignedCharArray *color_array = vtkUnsignedCharArray::New();
    if(vecdata->GetNumberOfComponents() != 4)
    {
        // Convert N components to 4 uchar components.
        color_array->SetNumberOfComponents(4);
        color_array->SetNumberOfTuples(vecdata->GetNumberOfTuples());
        unsigned char *pixels = (unsigned char *)color_array->GetVoidPointer(0);
        if(vecdata->GetNumberOfComponents() == 1)
        {
            for(vtkIdType id = 0; id < vecdata->GetNumberOfTuples(); ++id)
            {
                pixels[0] = DoubleToColor(vecdata->GetTuple1(id));
                pixels[1] = 0;
                pixels[2] = 0;
                pixels[3] = 255;
                pixels += 4;
            }
        }
        else if(vecdata->GetNumberOfComponents() == 2)
        {
            for(vtkIdType id = 0; id < vecdata->GetNumberOfTuples(); ++id)
            {
                const double *src = vecdata->GetTuple2(id);
                pixels[0] = DoubleToColor(src[0]);
                pixels[1] = DoubleToColor(src[1]);
                pixels[2] = 0;
                pixels[3] = 255;
                pixels += 4;
            }
        }
        if(vecdata->GetNumberOfComponents() == 3)
        {
            for(vtkIdType id = 0; id < vecdata->GetNumberOfTuples(); ++id)
            {
                const double *src = vecdata->GetTuple3(id);
                pixels[0] = DoubleToColor(src[0]);
                pixels[1] = DoubleToColor(src[1]);
                pixels[2] = DoubleToColor(src[2]);
                pixels[3] = 255;
                pixels += 4;
            }
        }
        else
        {
            // More than 4 components.
            double *tmp = new double[vecdata->GetNumberOfComponents()];
            for(vtkIdType id = 0; id < vecdata->GetNumberOfTuples(); ++id)
            {
                // Get all components in the tuple.
                vecdata->GetTuple(id, tmp);
                // Use the first 4 components in the tuple.
                color_array->SetTuple(id, tmp);
            }
            delete [] tmp;
        }
    }
    else
    {
        // 4 components. Take a shortcut.
        color_array->DeepCopy(vecdata);
    }

    // Add scalar array to the output dataset and make it active
    color_array->SetName("ucharColors");
    if (isZonal)
    {
        outDS->GetCellData()->AddArray(color_array);
        outDS->GetCellData()->SetActiveScalars("ucharColors");
    }
    else
    {
        outDS->GetPointData()->AddArray(color_array);
        outDS->GetPointData()->SetActiveScalars("ucharColors");
    }

    color_array->Delete();

    ManageMemory(outDS);
    outDS->Delete();

    return outDS;
}


