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
//                               avtCurveQuery.C                             //
// ************************************************************************* //

#include <avtCurveQuery.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtCurveConstructorFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtCurveQuery::avtCurveQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2003
//
// ****************************************************************************

avtCurveQuery::avtCurveQuery()
{
    ccf = new avtCurveConstructorFilter;
}


// ****************************************************************************
//  Method: avtCurveQuery::~avtCurveQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2003
//
// ****************************************************************************

avtCurveQuery::~avtCurveQuery()
{
    if (ccf != NULL)
    {
        delete ccf;
        ccf = NULL;
    }
}


// ****************************************************************************
//  Method: avtCurveQuery::ApplyFilters
//
//  Purpose:
//      This will apply the curve constructor filter.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

avtDataObject_p
avtCurveQuery::ApplyFilters(avtDataObject_p inData)
{
    avtPipelineSpecification_p pspec =
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    ccf->SetInput(dob);
    avtDataObject_p objOut = ccf->GetOutput();
    objOut->Update(pspec);
    return objOut;
}


// ****************************************************************************
//  Method: avtCurveQuery::Execute
//
//  Purpose:
//      This is the method for the base type to iterate through the data
//      tree.  However, after calling the curve constructor filter, we know
//      there will be exactly one node on the data tree and it will be on
//      processor 0.  So this function will be called exactly one time for
//      the whole curve.  So we can just feed that curve to our derived types.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul 27 17:43:38 PDT 2006
//    Curves now represented as 1D RectilinearGrids.
//
// ****************************************************************************

void
avtCurveQuery::Execute(vtkDataSet *ds, const int)
{
    //
    // Construct the curve.  This is heavily assuming that the input is a
    // well-formed curve from the curve constructor filter.
    //
    vtkDataArray *xc = ((vtkRectilinearGrid*)ds)->GetXCoordinates();
    vtkDataArray *sc = ds->GetPointData()->GetScalars();
    int np = xc->GetNumberOfTuples();
    float *x = new float[np];
    float *y = new float[np];
    for (int i = 0 ; i < np ; i++)
    {
         x[i] = xc->GetTuple1(i);
         y[i] = sc->GetTuple1(i);
    }

    //
    // Let the derived type do the actual query.
    //
    double val = CurveQuery(np, x, y);
    std::string msg = CreateMessage(val);

    //
    // Tell the query what the results were.
    //
    SetResultValue(val);
    SetResultMessage(msg);

    delete [] x;
    delete [] y;
}


