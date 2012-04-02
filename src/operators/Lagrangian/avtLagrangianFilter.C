/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//  File: avtLagrangianFilter.C
// ************************************************************************* //

#include <avtLagrangianFilter.h>

#include <avtStateRecorderIntegralCurve.h>

#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkAppendFilter.h>

#include <vtkVisItUtility.h>

// ****************************************************************************
//  Class: avtLagrangianIC
//
//  Purpose:
//      A plugin operator for Lagrangian.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
// ****************************************************************************

class avtLagrangianIC : public avtStateRecorderIntegralCurve
{
  public:
    avtLagrangianIC() { numSteps = 0; maxSteps = 0; }
    avtLagrangianIC(unsigned int mask, const avtIVPSolver *model, 
                    avtIntegralCurve::Direction dir, 
                    const double &t_start, 
                    const avtVector &p_start, 
                    const avtVector &v_start, 
                    long int ID) 
        : avtStateRecorderIntegralCurve(mask, model, dir, t_start, 
                                        p_start, v_start, ID)
    { numSteps = 0; maxSteps = 0;}
    
    virtual bool CheckForTermination(avtIVPStep &step, avtIVPField *field)
    {
        numSteps++;
        if (numSteps >= maxSteps)
            return true;
        return false;
    }
    
    int maxSteps;
  private:
    int numSteps;
};

// ****************************************************************************
//  Method: avtLagrangianFilter constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
// ****************************************************************************

avtLagrangianFilter::avtLagrangianFilter()
{
}


// ****************************************************************************
//  Method: avtLagrangianFilter destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
//  Modifications:
//
// ****************************************************************************

avtLagrangianFilter::~avtLagrangianFilter()
{
}


// ****************************************************************************
//  Method:  avtLagrangianFilter::Create
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
// ****************************************************************************

avtFilter *
avtLagrangianFilter::Create()
{
    return new avtLagrangianFilter();
}


// ****************************************************************************
//  Method:      avtLagrangianFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
// ****************************************************************************

void
avtLagrangianFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const LagrangianAttributes*)a;
}


// ****************************************************************************
//  Method: avtLagrangianFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtLagrangianFilter with the given
//      parameters would result in an equivalent avtLagrangianFilter.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
// ****************************************************************************

bool
avtLagrangianFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(LagrangianAttributes*)a);
}

// ****************************************************************************
// Method:  avtLagrangianFilter::CreateIntegralCurve
//
// Purpose: Create ICs.
//   
//  Programmer: Dave Pugmire
// Creation:    March 14, 2012
//
// ****************************************************************************

avtIntegralCurve *
avtLagrangianFilter::CreateIntegralCurve()
{
    avtLagrangianIC *ic = new avtLagrangianIC;
    ic->maxSteps = atts.GetNumSteps();
    return ic;
}

// ****************************************************************************
// Method:  avtLagrangianFilter::CreateIntegralCurve
//
// Purpose: Create ICs.
//   
//  Programmer: Dave Pugmire
// Creation:    March 14, 2012
//
// ****************************************************************************

avtIntegralCurve *
avtLagrangianFilter::CreateIntegralCurve(const avtIVPSolver *model, 
                                         avtIntegralCurve::Direction dir, 
                                         const double &t_start, 
                                         const avtVector &p_start, 
                                         const avtVector &v_start, 
                                         long int ID)

{
    int mask = avtStateRecorderIntegralCurve::SAMPLE_POSITION;

    if (atts.GetXAxisSample() == LagrangianAttributes::Time ||
        atts.GetYAxisSample() == LagrangianAttributes::Time)
    {
        mask |= avtStateRecorderIntegralCurve::SAMPLE_TIME;
    }
    if (atts.GetXAxisSample() == LagrangianAttributes::ArcLength ||
        atts.GetYAxisSample() == LagrangianAttributes::ArcLength)
    {
        mask |= avtStateRecorderIntegralCurve::SAMPLE_ARCLENGTH;
    }
    if (atts.GetXAxisSample() == LagrangianAttributes::Speed ||
        atts.GetYAxisSample() == LagrangianAttributes::Speed)
    {
        mask |= avtStateRecorderIntegralCurve::SAMPLE_VELOCITY;
    }
    if (atts.GetXAxisSample() == LagrangianAttributes::Vorticity ||
        atts.GetYAxisSample() == LagrangianAttributes::Vorticity)
    {
        mask |= avtStateRecorderIntegralCurve::SAMPLE_VORTICITY;
    }
    if (atts.GetXAxisSample() == LagrangianAttributes::Variable ||
        atts.GetYAxisSample() == LagrangianAttributes::Variable)
    {
        mask |= avtStateRecorderIntegralCurve::SAMPLE_SCALAR0;
    }
    
    avtLagrangianIC *ic = new avtLagrangianIC(mask, model, dir, t_start,
                                              p_start, v_start, ID);
    ic->maxSteps = atts.GetNumSteps();
    return ic;
}

// ****************************************************************************
// Method:  avtLagrangianFilter::GetInitialLocations
//
// Purpose: Create seed locations.
//   
//  Programmer: Dave Pugmire
// Creation:    March 14, 2012
//
// ****************************************************************************

std::vector<avtVector>
avtLagrangianFilter::GetInitialLocations()
{
    std::vector<avtVector> rv;
    avtVector v;
    v.x = atts.GetSeedPoint()[0];
    v.y = atts.GetSeedPoint()[1];
    v.z = atts.GetSeedPoint()[2];
    
    rv.push_back(v);
    return rv;
}


// ****************************************************************************
// Method:  avtLagrangianFilter::CreateIntegralCurveOutput
//
// Purpose: Create output.
//   
//  Programmer: Dave Pugmire
// Creation:    March 14, 2012
//
// ****************************************************************************

void
avtLagrangianFilter::CreateIntegralCurveOutput(std::vector<avtIntegralCurve *> &ics)
{
    
    avtLagrangianIC *ic = (avtLagrangianIC*)ics[0];
    int nSamps = ic->GetNumberOfSamples();
    
    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nSamps,VTK_FLOAT);
    vtkFloatArray    *vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nSamps);
    vals->SetName("curve");
        
    rg->GetPointData()->SetScalars(vals);
    vtkFloatArray *xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
    
    avtStateRecorderIntegralCurve::Sample samp;

    float xi, yi;
    for (int j = 0; j < nSamps; j++)
    {
        samp = ic->GetSample(j);

        if (atts.GetXAxisSample() == LagrangianAttributes::Step)
            xi = j;
        else if (atts.GetXAxisSample() == LagrangianAttributes::Time)
            xi = samp.time;
        else if (atts.GetXAxisSample() == LagrangianAttributes::ArcLength)
            xi = samp.arclength;

        if (atts.GetYAxisSample() == LagrangianAttributes::Step)
            yi = j;
        else if (atts.GetYAxisSample() == LagrangianAttributes::Time)
            yi = samp.time;
        else if (atts.GetYAxisSample() == LagrangianAttributes::ArcLength)
            yi = samp.arclength;
        else if (atts.GetYAxisSample() == LagrangianAttributes::Speed)
            yi = samp.velocity.length();
        else if (atts.GetYAxisSample() == LagrangianAttributes::Vorticity)
            yi = samp.vorticity;
        else if (atts.GetYAxisSample() == LagrangianAttributes::Variable)
            yi = samp.scalar0;
        
        xc->SetValue(j, xi);
        vals->SetValue(j, yi);
    }

    avtDataTree_p rv = new avtDataTree(rg, -1);
    
    SetOutputDataTree(rv);
}


// ****************************************************************************
//  Method: avtLagrangianFilter::ModifyContract
//
//  Purpose:
//      Creates a contract the removes the operator-created-expression.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
// ****************************************************************************

avtContract_p
avtLagrangianFilter::ModifyContract(avtContract_p in_contract)
{
    avtContract_p rv;
    const char *pipelineVariable = in_contract->GetDataRequest()->GetVariable();
    if (strncmp(pipelineVariable, "operators/Lagrangian", strlen("operators/Lagrangian")) == 0)
    {
        outVarName = pipelineVariable;
        const char *justTheVar = pipelineVariable + strlen("operators/Lagrangian");
        avtDataRequest_p dr = new avtDataRequest(in_contract->GetDataRequest(), justTheVar);
        rv = new avtContract(in_contract, dr);
    }
    else
    {
        rv = new avtContract(in_contract);
    }

    if (atts.GetYAxisSample() == LagrangianAttributes::Variable)
    {
        avtDataRequest_p in_dr = rv->GetDataRequest();
        avtDataRequest_p out_dr = new avtDataRequest(in_dr);
        out_dr->AddSecondaryVariable(atts.GetVariable().c_str());
        avtContract_p out_contract;
        out_contract = new avtContract(rv, out_dr);
        return avtPICSFilter::ModifyContract(out_contract);
    }
    else
        return avtPICSFilter::ModifyContract(rv);
}


// ****************************************************************************
//  Method: avtLagrangianFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Tells output that we have a new variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 7 14:22:35 PST 2012
//
// ****************************************************************************

void
avtLagrangianFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetAttributes().GetThisProcsOriginalSpatialExtents()->Clear();
    GetOutput()->GetInfo().GetAttributes().GetOriginalSpatialExtents()->Clear();
    GetOutput()->GetInfo().GetAttributes().GetDesiredSpatialExtents()->Clear();

    if (outVarName != "")
    {
        avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
        if (! atts.ValidVariable(outVarName))
        {
            atts.AddVariable(outVarName.c_str());
            atts.SetActiveVariable(outVarName.c_str());
            atts.SetVariableDimension(1);
            atts.SetVariableType(AVT_SCALAR_VAR);
            atts.SetCentering(AVT_NODECENT);
        }
    }

    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetAttributes().SetSpatialDimension(2);
    
    std::string xLabel, yLabel;
    if (atts.GetXAxisSample() == LagrangianAttributes::Step)
        xLabel = "Integration Step";
    else if (atts.GetXAxisSample() == LagrangianAttributes::Time)
        xLabel = "Time";
    else if (atts.GetXAxisSample() == LagrangianAttributes::ArcLength)
        xLabel = "Arc length";
    else if (atts.GetXAxisSample() == LagrangianAttributes::Speed)
        xLabel = "Speed";
    else if (atts.GetXAxisSample() == LagrangianAttributes::Vorticity)
        xLabel = "Vorticity";
    else if (atts.GetXAxisSample() == LagrangianAttributes::Variable)
        xLabel = atts.GetVariable();

    if (atts.GetYAxisSample() == LagrangianAttributes::Step)
        yLabel = "Integration Step";
    else if (atts.GetYAxisSample() == LagrangianAttributes::Time)
        yLabel = "Time";
    else if (atts.GetYAxisSample() == LagrangianAttributes::ArcLength)
        yLabel = "Arc length";
    else if (atts.GetYAxisSample() == LagrangianAttributes::Speed)
        yLabel = "Speed";
    else if (atts.GetYAxisSample() == LagrangianAttributes::Vorticity)
        yLabel = "Vorticity";
    else if (atts.GetYAxisSample() == LagrangianAttributes::Variable)
        yLabel = atts.GetVariable();
    
    GetOutput()->GetInfo().GetAttributes().SetXLabel(xLabel.c_str());
    GetOutput()->GetInfo().GetAttributes().SetXUnits("");
    GetOutput()->GetInfo().GetAttributes().SetYLabel(yLabel.c_str());
    GetOutput()->GetInfo().GetAttributes().SetYUnits("");
}

// ****************************************************************************
// Method:  avtLagrangianFilter::GetFieldForDomain
//
// Purpose: Set sampling variable.
//   
//  Programmer: Dave Pugmire
// Creation:    March 14, 2012
//
// ****************************************************************************

avtIVPField*
avtLagrangianFilter::GetFieldForDomain(const DomainType& dom, vtkDataSet* ds)
{
    avtIVPField* field = avtPICSFilter::GetFieldForDomain( dom, ds );

    if (atts.GetYAxisSample() == LagrangianAttributes::Variable)
        field->SetScalarVariable(0, atts.GetVariable().c_str());
    
    return field;
}

