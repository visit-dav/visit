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
//                         avtVariableSummationQuery.C                       //
// ************************************************************************* //

#include <avtVariableSummationQuery.h>

#include <avtCondenseDatasetFilter.h>
#include <avtTerminatingSource.h>
#include <BadIndexException.h>

using     std::string;


// ****************************************************************************
//  Method: avtVariableSummationQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Add condense filter.
//
// ****************************************************************************

avtVariableSummationQuery::avtVariableSummationQuery() : avtSummationQuery()
{
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
    condense->BypassHeuristic(true);
}


// ****************************************************************************
//  Method: avtVariableSummationQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Add condense filter.
//
// ****************************************************************************

avtVariableSummationQuery::~avtVariableSummationQuery()
{
    if (condense != NULL)
    {
        delete condense;
        condense = NULL;
    }
}


// ****************************************************************************
//  Method: avtVariableSummationQuery::VerifyInput
//
//  Purpose:
//      Now that we have an input, we can determine what the variable name is
//      and tell the base class about it.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004
//    Retrieve variable's units, if available.
//
//    Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005 
//    Remove TRY-CATCH block in favor of testing for ValidVariable.
//
// ****************************************************************************

void
avtVariableSummationQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtSummationQuery::VerifyInput();

    avtDataSpecification_p dspec = GetInput()->GetTerminatingSource()
                                     ->GetFullDataSpecification();

    avtDataAttributes &dataAtts = GetInput()->GetInfo().GetAttributes();

    string varname = dspec->GetVariable();
    SetVariableName(varname);
    SumGhostValues(false);
    SetSumType(varname);
    if (dataAtts.ValidVariable(varname.c_str()))
    {
        //
        // Set the base class units to be used in output.
        //
        SetUnits(dataAtts.GetVariableUnits(varname.c_str()));
    }
}


// ****************************************************************************
//  Method: avtVariableSummationQuery::ApplyFilters
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 24, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Thu May 11 10:32:54 PDT 2006
//    Use condense filter for pointdata when the mesh has been mat selected.
//
// ****************************************************************************

avtDataObject_p
avtVariableSummationQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataValidity &dval = GetInput()->GetInfo().GetValidity();
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    bool cellData = false;

    avtCentering cent = AVT_UNKNOWN_CENT;
    if (datts.ValidVariable(variableName))
    {
        cent = datts.GetCentering(variableName.c_str());
        cellData = (cent != AVT_NODECENT);
    }
    else 
    {
        // we can't determine the centering, assume zone-centered
        cellData = true;
    }

    if ((dval.SubdivisionOccurred() || 
       (!dval.GetOriginalZonesIntact() && cellData)) ||
       (!cellData && !dval.GetZonesPreserved()))
    {
        // This will work for time-varying data, too.

        // tell parent class to sum from original element values.
        // e.g. each 'original' cell/node constributes only once to
        // the sum.
        SumFromOriginalElement(true);

        // Need to request original cell and/or node numbers
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        if (cent == AVT_ZONECENT)
        {
            newDS->TurnZoneNumbersOn();
        }
        else if (cent == AVT_NODECENT)
        {
            newDS->TurnNodeNumbersOn();
        }
        else 
        {
            newDS->TurnZoneNumbersOn();
            newDS->TurnNodeNumbersOn();
        }

        avtPipelineSpecification_p pspec = 
            new avtPipelineSpecification(newDS, queryAtts.GetPipeIndex());

        avtDataObject_p temp;
        CopyTo(temp, inData);
        condense->SetInput(temp);
        avtDataObject_p rv = condense->GetOutput();
        rv->Update(pspec);
        return rv;
    }
    else 
    {
        return avtSummationQuery::ApplyFilters(inData);
    }
}
