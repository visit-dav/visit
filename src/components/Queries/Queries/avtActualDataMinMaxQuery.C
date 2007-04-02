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
//                        avtActualDataMinMaxQuery.C                         //
// ************************************************************************* //

#include <avtActualDataMinMaxQuery.h>

#include <avtCondenseDatasetFilter.h>
#include <avtTerminatingSource.h>
#include <avtSourceFromAVTDataset.h>


// ****************************************************************************
//  Method: avtActualDataMinMaxQuery::avtActualDataMinMaxQuery
//
//  Purpose:
//      Construct an avtActualDataMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     February 10, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004
//    Add min/max arguments, passed to parent class.
//
//    Kathleen Bonnell, Wed Apr 14 18:00:49 PDT 2004 
//    Tell condense filter to bypass its heuristic and force execution of
//    relevant points filter. 
//
// ****************************************************************************

avtActualDataMinMaxQuery::avtActualDataMinMaxQuery(bool min, bool max)
    : avtMinMaxQuery(min, max)
{
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
    condense->BypassHeuristic(true);
}


// ****************************************************************************
//  Method: avtActualDataMinMaxQuery::~avtActualDataMinMaxQuery
//
//  Purpose:
//      Destruct an avtActualDataMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     February 10, 2040 
//
//  Modifications:
//
// ****************************************************************************

avtActualDataMinMaxQuery::~avtActualDataMinMaxQuery()
{
    if (condense != NULL)
    {
        delete condense;
        condense = NULL;
    }
}


// ****************************************************************************
//  Method: avtActualDataMinMaxQuery::ApplyFilters
//
//  Purpose:
//    Applies the condense filter to input and returns the filter's output. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004
//    Added code for time-varying case.
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet. 
//
//    Kathleen Bonnell, Thu May  6 17:36:43 PDT 2004 
//    Request OriginalCellsArray if zones have not been preserved. 
//
// ****************************************************************************

avtDataObject_p
avtActualDataMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    Preparation(inData);

    bool zonesPreserved  = GetInput()->GetInfo().GetValidity().GetZonesPreserved();
    if (!timeVarying && zonesPreserved)
    {
        avtPipelineSpecification_p pspec = 
            inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

        avtDataset_p ds;
        CopyTo(ds, inData);
        avtSourceFromAVTDataset termsrc(ds);
        avtDataObject_p obj = termsrc.GetOutput();
        condense->SetInput(obj);
        avtDataObject_p retObj = condense->GetOutput();
        retObj->Update(pspec);
        return retObj;
    }
    else 
    {
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        if (!zonesPreserved)
            newDS->TurnZoneNumbersOn();

        avtPipelineSpecification_p pspec = 
            new avtPipelineSpecification(newDS, queryAtts.GetPipeIndex());

        avtDataObject_p temp;
        CopyTo(temp, inData);
        condense->SetInput(temp);
        avtDataObject_p retObj = condense->GetOutput();
        retObj->Update(pspec);
        return retObj;
    }
}
