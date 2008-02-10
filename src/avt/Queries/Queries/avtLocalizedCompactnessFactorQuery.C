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
//                      avtLocalizedCompactnessFactorQuery.C                 //
// ************************************************************************* //

#include <avtLocalizedCompactnessFactorQuery.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtConstantCreatorExpression.h>
#include <avtDatasetExaminer.h>
#include <avtLocalizedCompactnessExpression.h>
#include <avtParallel.h>
#include <avtResampleFilter.h>
#include <avtSourceFromAVTDataset.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidDimensionsException.h>
#include <NonQueryableInputException.h>

#include <snprintf.h>
#include <float.h>
#include <math.h>

using     std::string;
using     std::vector;

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery constructor
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

avtLocalizedCompactnessFactorQuery::avtLocalizedCompactnessFactorQuery()
{
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

avtLocalizedCompactnessFactorQuery::~avtLocalizedCompactnessFactorQuery()
{
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtLocalizedCompactnessFactorQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

    sum = 0.;
    numEntries = 0;
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

void
avtLocalizedCompactnessFactorQuery::PostExecute(void)
{
    double fullSum;
    SumDoubleArrayAcrossAllProcessors(&sum, &fullSum, 1);
    sum = fullSum;

    int totalNumEntries;
    SumIntArrayAcrossAllProcessors(&numEntries, &totalNumEntries, 1);
    numEntries = totalNumEntries;

    double factor = (totalNumEntries > 0 ? fullSum / totalNumEntries : 0);

    char buff[1024];
    string format = "The localized compactness factor is "
                    + queryAtts.GetFloatFormat() + "\n";
    SNPRINTF(buff, 1024,format.c_str() , factor);
    SetResultMessage(buff);
    SetResultValue(factor);
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::Execute
//
//  Purpose:
//      Processes a single domain.  This will calculate the average compactness
//      over that domain.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

void
avtLocalizedCompactnessFactorQuery::Execute(vtkDataSet *ds, const int dom)
{
    vtkDataArray *varF = ds->GetPointData()->GetArray("lce"); // F = factor
    vtkDataArray *varQ = ds->GetPointData()->GetArray("is_material");
                  // Q = qualifier.  It helps us answer the question: 
                  // Should we even count this?

    if (varF == NULL || varQ == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    int nPts = ds->GetNumberOfPoints();
    for (int i = 0 ; i < nPts ; i++)
    {
        if (varQ->GetTuple1(i) != 0.)
        {
            sum += varF->GetTuple1(i);
            numEntries++;
        }
    }
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
//  Modifications:
//
//    Hank Childs, Mon May 22 15:44:46 PDT 2006
//    Make the resample region be only the region where there is actually data.
//
// ****************************************************************************

avtDataObject_p
avtLocalizedCompactnessFactorQuery::ApplyFilters(avtDataObject_p inData)
{
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    double extents[6] = { 0, 0, 0, 0, 0, 0 };
    avtDatasetExaminer::GetSpatialExtents(ds, extents);

    avtConstantCreatorExpression ccf;
    ccf.SetValue(1.0);
    ccf.SetInput(dob);
    ccf.SetOutputVariableName("is_material");
    dob = ccf.GetOutput();
   
    ResampleAttributes res_atts;
    res_atts.SetDefaultVal(0.);
    bool is2D = (dob->GetInfo().GetAttributes().GetSpatialDimension() < 3);
    int res = (is2D ? 250000 : 2000000);
    res_atts.SetTargetVal(res);
    res_atts.SetUseTargetVal(true);
    res_atts.SetUseBounds(true);
    res_atts.SetMinX(extents[0]);
    res_atts.SetMaxX(extents[1]);
    res_atts.SetMinY(extents[2]);
    res_atts.SetMaxY(extents[3]);
    res_atts.SetMinZ(extents[4]);
    res_atts.SetMaxZ(extents[5]);
    avtResampleFilter resf(&res_atts);
    resf.SetInput(dob);
    dob = resf.GetOutput();
    
    avtLocalizedCompactnessExpression lce;
    lce.SetInput(dob);
    lce.SetOutputVariableName("lce");
    lce.AddInputVariableName("is_material");
    dob = lce.GetOutput();

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract =
        inData->GetOriginatingSource()->GetGeneralContract();
    contract->GetDataRequest()->AddSecondaryVariable("is_material");
    dob->Update(contract);
    return dob;
}


