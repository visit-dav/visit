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
//                        avtTotalVolumeQuery.C                              //
// ************************************************************************* //

#include <avtTotalVolumeQuery.h>

#include <avtVMetricVolume.h>
#include <avtSourceFromAVTDataset.h>
#include <InvalidDimensionsException.h>


using     std::string;

// ****************************************************************************
//  Method: avtTotalVolumeQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalVolumeFilter, 
//          now deprecated.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:06:59 PDT 2003
//    Set units_append.
//
//    Kathleen Bonnell, Fri Sep 15 09:23:50 PDT 2006
//    Added bool constructor arg, passed to avtVMetricVolume.
//
// ****************************************************************************

avtTotalVolumeQuery::avtTotalVolumeQuery(bool useVerdictHex) : avtSummationQuery()
{
    string      varname = "volume";
    string      sum_type = "Volume";    
    string      units_append = "^3";    

    volume      = new avtVMetricVolume;
    volume->SetOutputVariableName(varname.c_str());
    volume->UseOnlyPositiveVolumes(true);
    volume->UseVerdictHex(useVerdictHex);

    SetVariableName(varname);
    SetSumType(sum_type);
    SetUnitsAppend(units_append);
    SumGhostValues(false);
}


// ****************************************************************************
//  Method: avtTotalVolumeQuery destructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalVolumeFilter, 
//          now deprecated.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//
// ****************************************************************************

avtTotalVolumeQuery::~avtTotalVolumeQuery()
{
    if (volume != NULL)
    {
        delete volume;
        volume = NULL;
    }
}


// ****************************************************************************
//  Method: avtTotalVolumeQuery::VerifyInput 
//
//  Purpose:  Throw exception for invalid input. 
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
avtTotalVolumeQuery::VerifyInput()
{
    //
    //  Since base class performs general input verification for all queries,
    //  make sure to call it's verify method first.
    //
    avtDataObjectQuery::VerifyInput();

    int tDim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if  (tDim != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "Volume query", "3D surface");
    }
    SetUnits(GetInput()->GetInfo().GetAttributes().GetXUnits());
}


// ****************************************************************************
//  Method: avtTotalVolumeQuery::ApplyFilters 
//
//  Purpose:  Apply any avt filters necessary for completion of this query.
//
//  Notes:  Taken mostly from Hank Childs' avtTotalVolumeFilter, 
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
//    Rework so that both time-varying and non use artificial pipeline. 
//    Only difference is the pipeline spec used. 
//
// ****************************************************************************

avtDataObject_p 
avtTotalVolumeQuery::ApplyFilters(avtDataObject_p inData)
{
    avtContract_p contract =
        inData->GetOriginatingSource()->GetGeneralContract();

    if (timeVarying) 
    { 
        avtDataRequest_p oldSpec = inData->GetOriginatingSource()->
            GetGeneralContract()->GetDataRequest();

        avtDataRequest_p newDS = new 
            avtDataRequest(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        contract = new avtContract(newDS, contract->GetPipelineIndex());
    }


    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();
    volume->SetInput(dob);
    avtDataObject_p objOut = volume->GetOutput();
    objOut->Update(contract);
    return objOut;
}

