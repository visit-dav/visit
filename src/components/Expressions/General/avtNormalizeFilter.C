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
//                             avtNormalizeFilter.C                           //
// ************************************************************************* //

#include <avtNormalizeFilter.h>

#include <math.h>
#include <vector>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtNormalizeFilter constructor
//
//  Programmer: Sean Ahern
//  Creation:   Wed Feb  7 19:03:09 EST 2007
//
//  Modifications:
//
// ****************************************************************************

avtNormalizeFilter::avtNormalizeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtNormalizeFilter destructor
//
//  Programmer: Sean Ahern
//  Creation:   Wed Feb  7 19:03:22 EST 2007
//
//  Modifications:
//
// ****************************************************************************

avtNormalizeFilter::~avtNormalizeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtNormalizeFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern (mostly copied from avtMagnitudeFilter)
//  Creation:     Wed Feb  7 19:14:48 EST 2007
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtNormalizeFilter::DeriveVariable(vtkDataSet *in)
{
    // The base class will set the variable of interest to be the
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.

    const char *varname = activeVariable;

    vtkDataArray *vectorValues = in->GetPointData()->GetArray(varname);

    if (vectorValues == NULL)
    {
        vectorValues = in->GetCellData()->GetArray(varname);
    }
    if (vectorValues == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Unable to locate variable for normalize expression");
    }

    if (vectorValues->GetNumberOfComponents() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can only normalize vectors.");
    }

    int ntuples = vectorValues->GetNumberOfTuples();

    vtkDataArray *results = vectorValues->NewInstance();
    results->SetNumberOfComponents(3);
    results->SetNumberOfTuples(ntuples);

    for (int i=0; i<ntuples; i++)
    {
        float xin = vectorValues->GetComponent(i, 0);
        float yin = vectorValues->GetComponent(i, 1);
        float zin = vectorValues->GetComponent(i, 2);

        float mag = sqrt(xin*xin + yin*yin + zin*zin);

        float xout, yout, zout;
        if (mag == 0.)
            xout = yout = zout = 0.;
        else
        {
            xout = xin / mag;
            yout = yin / mag;
            zout = zin / mag;
        }

        results->SetComponent(i, 0, xout);
        results->SetComponent(i, 1, yout);
        results->SetComponent(i, 2, zout);
    }

    return results;
}
