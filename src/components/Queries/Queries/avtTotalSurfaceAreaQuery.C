/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                   avtTotalSurfaceAreaQuery.C                              //
// ************************************************************************* //

#include <avtTotalSurfaceAreaQuery.h>

#include <avtFacelistFilter.h>
#include <avtVMetricArea.h>
#include <avtSourceFromAVTDataset.h>
#include <InvalidDimensionsException.h>


using     std::string;

// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter,
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:06:59 PDT 2003
//    Set units_append.
//
// ****************************************************************************

avtTotalSurfaceAreaQuery::avtTotalSurfaceAreaQuery() : avtSummationQuery()
{
    string      varname = "area";
    string      sum_type = "Surface Area";    
    string      units_append = "^2";    

    facelist  = new avtFacelistFilter;

    area      = new avtVMetricArea;
    area->SetOutputVariableName(varname.c_str());

    SetVariableName(varname);
    SetSumType(sum_type);
    SetUnitsAppend(units_append);
    SumGhostValues(false);

    // avtVMetricArea will return -1 for cells that are neither triangles
    // nor quads.  If the dataset has lines, will rack up a bunch of -1's,
    // so turn off the summing of negative values.
    SumOnlyPositiveValues(true);
}


// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery destructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter,
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//
// ****************************************************************************

avtTotalSurfaceAreaQuery::~avtTotalSurfaceAreaQuery()
{
    if (area != NULL)
    {
        delete area;
        area = NULL;
    }
    if (facelist != NULL)
    {
        delete facelist;
        facelist = NULL;
    }
}


// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery::VerifyInput 
//
//  Purpose:  Throw an exception for invalid input. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004
//    Retrieve units from dataAttributes to be used in output.
//
// ****************************************************************************

void 
avtTotalSurfaceAreaQuery::VerifyInput()
{
    //
    //  Since base class performs general input verification for all queries,
    //  make sure to call it's verify method first.
    //
    avtDataObjectQuery::VerifyInput();

    if  (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() < 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Surface Area query", 
                   "2D or 3D surface");
    }
    SetUnits(GetInput()->GetInfo().GetAttributes().GetXUnits());
}



// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery::ApplyFilters 
//
//  Purpose:  Apply any avt filters necessary for completion of this query.
//
//  Notes:  Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter,
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002   
//    Moved artificial pipeline creation from base class.
//
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004 
//    Added logic for time-varying case. 
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet. 
//
//    Kathleen Bonnell, Thu Jan  6 11:06:29 PST 2005 
//    Both time-varying and non use artificial pipeline. Only difference is
//    the pipeline spec used. 
//
// ****************************************************************************

avtDataObject_p 
avtTotalSurfaceAreaQuery::ApplyFilters(avtDataObject_p inData)
{
    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    if (timeVarying) 
    { 
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        pspec = new avtPipelineSpecification(newDS, pspec->GetPipelineIndex());
    }

    //
    // Create an artificial pipeline.
    //

    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    facelist->SetInput(dob);
    area->SetInput(facelist->GetOutput());
    avtDataObject_p objOut = area->GetOutput();
    objOut->Update(pspec);

    return objOut;
}


