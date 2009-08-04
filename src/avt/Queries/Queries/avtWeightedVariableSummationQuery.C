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
//                     avtWeightedVariableSummationQuery.C                   //
// ************************************************************************* //

#include <avtWeightedVariableSummationQuery.h>

#include <avtBinaryMultiplyExpression.h>
#include <avtRevolvedVolume.h>
#include <avtSourceFromAVTDataset.h>
#include <avtOriginatingSource.h>
#include <avtVMetricArea.h>
#include <avtVMetricVolume.h>

#include <DebugStream.h>


using     std::string;


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 10 14:05:07 PDT 2005
//    Force only positive volumes, but allow negative values from var.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added revolvedVolume. 
//
// ****************************************************************************

avtWeightedVariableSummationQuery::avtWeightedVariableSummationQuery() 
    : avtSummationQuery()
{
    area = new avtVMetricArea;
    area->SetOutputVariableName("avt_weights");

    multiply = new avtBinaryMultiplyExpression;
    multiply->SetOutputVariableName("avt_sum");

    volume = new avtVMetricVolume;
    volume->SetOutputVariableName("avt_weights");
    volume->UseOnlyPositiveVolumes(true);

    revolvedVolume = new avtRevolvedVolume;
    revolvedVolume->SetOutputVariableName("avt_weights");

    string vname = "avt_sum";
    SetVariableName(vname);
    SumGhostValues(false);
    SumOnlyPositiveValues(false);
}


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added revolvedVolume. 
//
// ****************************************************************************

avtWeightedVariableSummationQuery::~avtWeightedVariableSummationQuery()
{
    delete area;
    delete multiply;
    delete volume;
    delete revolvedVolume;
}


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Mar 31 16:20:39 PST 2004
//    Added logic for time-varying case.
//
//    Hank Childs, Thu Apr  8 08:24:12 PDT 2004
//    Allow time varying data to access database, so it can consider more than
//    one timestep.
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet. 
//
//    Kathleen Bonnell, Fri Jan  7 15:15:32 PST 2005 
//    Rework so that both time-varying and non use artificial pipeline. 
//    Only difference is the pipeline spec used. 
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added revolvedVolume, use it when 2D data and meshcoord type is RZ or ZR.
//
//    Hank Childs, Thu May 11 13:31:55 PDT 2006
//    Add support for filters to inherit from this class and create new
//    variables based on the mesh.
//
//    Kathleen Bonnell, Wed Apr  2 10:20:27 PDT 2008 
//    Retrieve the varname from the dataAtts instead of DataRequest, as
//    DataRequest may have the wrong value based on other pipelines sharing
//    the same source. 
//
//    Kathleen Bonnell, Tue Jul 29 9:03:15 PDT 2008 
//    For better error messages, check if there is an active variable in the
//    data attributes, and if not then retrieve from data request.
//
//    Kathleen Bonnell, Tue Sep 23 08:53:03 PDT 2008 
//    Move setting of secondary var "avt_weights" till after the contract
//    has been set for the time-varying case. 
//
// ****************************************************************************

avtDataObject_p
avtWeightedVariableSummationQuery::ApplyFilters(avtDataObject_p inData)
{
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    //
    // Set up our base class so it is ready to sum.
    //
    string varname;
    if (GetInput()->GetInfo().GetAttributes().ValidActiveVariable())
        varname = GetInput()->GetInfo().GetAttributes().GetVariableName();
    else 
        varname = GetInput()->GetOriginatingSource()->GetFullDataRequest()->
                  GetVariable();
    varname = GetVarname(varname);
    SetSumType(varname);

    int topo = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if (topo == 2)
    {
        if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() == AVT_XY)
        {
            debug5 << "WeightedVariableSum using Area" << endl;
            area->SetInput(dob);
            dob = area->GetOutput();
        }
        else 
        {
            debug5 << "WeightedVariableSum using RevolvedVolume" << endl;
            revolvedVolume->SetInput(dob);
            dob = revolvedVolume->GetOutput();
        }
    }
    else
    {
        debug5 << "WeightedVariableSum using Volume" << endl;
        volume->SetInput(dob);
        dob = volume->GetOutput();
    }

    //
    // Let the derived type create a new variable if necessary.
    //
    dob = CreateVariable(dob);

    multiply->SetInput(dob);
    multiply->ClearInputVariableNames();
    multiply->AddInputVariableName("avt_weights");
    multiply->AddInputVariableName(varname.c_str());

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract = 
        inData->GetOriginatingSource()->GetGeneralContract();

    if (timeVarying) 
    { 
        avtDataRequest_p dataRequest = GetInput()->GetOriginatingSource()
                                           ->GetFullDataRequest();
        avtDataRequest_p newDS = new 
            avtDataRequest(dataRequest, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        contract = new avtContract(newDS, contract->GetPipelineIndex());
    }

    if (CalculateAverage())
    {
        denomVariableName = "avt_weights";
        // State that we want avt_weights as an output, so it doesn't get
        // thrown out after the multiply.
        contract->GetDataRequest()->AddSecondaryVariable("avt_weights");
    }

    multiply->GetOutput()->Update(contract);
    return multiply->GetOutput();
}


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery::VerifyInput
//
//  Purpose:
//      Now that we have an input, we can determine what the variable units
//      are and tell the base class about it.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 28, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005 
//    Remove TRY-CATCH block in favor of testing for ValidActiveVariable. 
//
//    Hank Childs, Thu May 11 14:13:30 PDT 2006
//    Do not set the units if the variable doesn't match the variable of
//    interest.
//
// ****************************************************************************

void
avtWeightedVariableSummationQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtSummationQuery::VerifyInput();

    if (GetInput()->GetInfo().GetAttributes().ValidActiveVariable())
    {
        std::string varname1 = 
                        GetInput()->GetInfo().GetAttributes().GetVariableName();
        std::string varname2 = GetVarname(varname1);
        if (varname1 == varname2)
        {
            //
            // Set the base class units to be used in output.
            //
            SetUnits(GetInput()->GetInfo().GetAttributes().GetVariableUnits());
        }
    }
}


