/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         avtHSVColorComposeFilter.C                           //
// ************************************************************************* //

#include <avtHSVColorComposeFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtHSVColorComposeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2007
//
// ****************************************************************************

avtHSVColorComposeFilter::avtHSVColorComposeFilter()
{
}


// ****************************************************************************
//  Method: avtHSVColorComposeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2007
//
// ****************************************************************************

avtHSVColorComposeFilter::~avtHSVColorComposeFilter()
{
    ;
}

// ****************************************************************************
//  Method: avtHSVColorComposeFilter::DeriveVariable
//
//  Purpose:
//      Creates a vector variable from components.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Notes: Mostly taken from avtColorComposeFilter.C
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2007
//
//  Modifications:
//
// ****************************************************************************

inline unsigned char 
ClampColor(const double c)
{
    int ic = (int)c;
    if(ic < 0) ic = 0;
    if(ic > 255) ic = 255;
    return (unsigned char)ic;
}

vtkDataArray *
avtHSVColorComposeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int numinputs = varnames.size();
    if (numinputs != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Required three arguments.");
    }

    //
    // Our first operand is in the active variable. We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    //
    vtkDataArray *cell_data1 = in_ds->GetCellData()->GetArray(varnames[0]);
    vtkDataArray *point_data1 = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *data1 = NULL, *data2 = NULL, *data3 = NULL;

    avtCentering centering;
    if (cell_data1 != NULL)
    {
        data1 = cell_data1;
        centering = AVT_ZONECENT;
    }
    else
    {
        data1 = point_data1;
        centering = AVT_NODECENT;
    }
    if (data1 != NULL && data1->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first variable is not a scalar.");
    }

    // Get the second variable.
    if (centering == AVT_ZONECENT)
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
    else
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);

    if (data2 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first two variables have different centering.");
    }
    if (data2->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The second variable is not a scalar.");
    }

    // Get the third variable.
    if (centering == AVT_ZONECENT)
        data3 = in_ds->GetCellData()->GetArray(varnames[2]);
    else
        data3 = in_ds->GetPointData()->GetArray(varnames[2]);
    
    if (data3 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first and third variables have different centering.");
    }
    if (data3->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The third variable is not a scalar.");
    }

    vtkIdType nvals = data1->GetNumberOfTuples();
    vtkDataArray *dv = data1->NewInstance();
    dv->SetNumberOfComponents(4);
    dv->SetNumberOfTuples(data1->GetNumberOfTuples());
    for(vtkIdType id = 0; id < nvals; ++id)
    {
        double r, g, b;
        double h = data1->GetTuple1(id);
        double s = data2->GetTuple1(id);
        double v = data3->GetTuple1(id);
        if (h<0) h=0;
        if (h>1) h=1;
        if (s<0) s=0;
        if (s>1) s=1;
        if (v<0) v=0;
        if (v>1) v=1;
        h *= 6;
        int sector = int(floor(h));
        double f = h - double(sector);
        if (sector == 6)
        {
            sector = 0;
            f = 0.;
        }
        double x = v * (1 - s);
        double y = v * (1 - s*f);
        double z = v * (1 - s*(1-f));

        switch (sector)
        {
          case 0:  r = v; g = z; b = x; break;
          case 1:  r = y; g = v; b = x; break;
          case 2:  r = x; g = v; b = z; break;
          case 3:  r = x; g = y; b = v; break;
          case 4:  r = z; g = x; b = v; break;
          default: r = v; g = x; b = y; break;
        }

        double val1 = ClampColor(255.*r);
        double val2 = ClampColor(255.*g);
        double val3 = ClampColor(255.*b);
        dv->SetTuple4(id, val1, val2, val3, 255.);
    }

    return dv;
}


