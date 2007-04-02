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
//                  avtConnComponentsWeightedVariableQuery.C                 //
// ************************************************************************* //

#include <avtConnComponentsWeightedVariableQuery.h>

#include <avtCallback.h>
#include <avtConnComponentsExpression.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtRevolvedVolume.h>
#include <avtVMetricArea.h>
#include <avtVMetricVolume.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkVisItUtility.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidDimensionsException.h>
#include <snprintf.h>



// ****************************************************************************
//  Method: avtConnComponentsWeightedVariableQuery constructor
//
//  Notes:  
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************

avtConnComponentsWeightedVariableQuery::
avtConnComponentsWeightedVariableQuery()
{
    // (base class creates the connected components filter)
    // create weight filters

    areaFilter = new avtVMetricArea;
    areaFilter->SetOutputVariableName("avt_weight");

    revolvedVolumeFilter = new avtRevolvedVolume;
    revolvedVolumeFilter->SetOutputVariableName("avt_weight");

    volumeFilter = new avtVMetricVolume;
    volumeFilter->SetOutputVariableName("avt_weight");

}

// ****************************************************************************
//  Method: avtConnComponentsWeightedVariableQuery  destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************

avtConnComponentsWeightedVariableQuery::
~avtConnComponentsWeightedVariableQuery()
{
    delete areaFilter;
    delete revolvedVolumeFilter;
    delete volumeFilter;
}


// ****************************************************************************
//  Method: avtConnComponentsWeightedVariableQuery::PreExecute
//
//
//  Purpose:
//      This is called before all of the domains are executed to obtain the
//      number of components and initialize component sums.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************

void
avtConnComponentsWeightedVariableQuery::PreExecute(void)
{
    // let base class get the # of connected components
    avtConnComponentsQuery::PreExecute();

    // prepare component arrays 
    sumPerComp.resize(nComps);
    for(int i=0;i<nComps;i++)
    {
        sumPerComp[i] = 0;
    }
}


// ****************************************************************************
//  Method: avtConnComponentsWeightedVariableQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed to collect
//      info from all processors and finalize component sums. 
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************

void
avtConnComponentsWeightedVariableQuery::PostExecute(void)
{
    // sum vals across all processors

    double *sum_res_dbl = new double[nComps];
    SumDoubleArrayAcrossAllProcessors(&sumPerComp[0], sum_res_dbl, nComps);
    memcpy(&sumPerComp[0],sum_res_dbl,nComps * sizeof(double));

    delete [] sum_res_dbl;

    // create output message

    std::string msg = "";
    char buff[2048];

    if(nComps == 1)
    {SNPRINTF(buff,2048,"Found %d connected component\n",nComps);}
    else
    {SNPRINTF(buff,2048,"Found %d connected components\n",nComps);}

    msg += buff;

    for(int i=0;i<nComps;i++)
    {
        SNPRINTF(buff,1024,
                 "Component %d Weighted Sum = (%f)\n",
                 i,
                 sumPerComp[i]);

        msg += buff;
    }

    // set output message
    SetResultMessage(msg);
    // set output values
    SetResultValues(sumPerComp);
}


// ****************************************************************************
//  Method: avtConnComponentsWeightedVariableQuery::Execute
//
//  Purpose:
//      Processes a single input domain to update the per component sums.
//
//  Arguments:
//      ds       Input dataset         
//      dom      Input domain number   
// 
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************

void
avtConnComponentsWeightedVariableQuery::Execute(vtkDataSet *ds, const int dom)
{
    const char *var = variableName.c_str();
    bool own_values_array = false;
    // get the number of cells to process
    int ncells = ds->GetNumberOfCells();
    // get the component label, weight, and value arrays
    vtkIntArray  *labels =(vtkIntArray*)ds->GetCellData()->GetArray("avt_ccl");
    vtkDataArray *values  =ds->GetCellData()->GetArray(var);
    vtkDataArray *weights =ds->GetCellData()->GetArray("avt_weight");

    // make sure the cell arrays are valid
    if (labels == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_ccl");
    }

    if (weights  == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_weight");
    }

    if (values  == NULL)
    {
        // if we have point data convert it to cell data
        values = ds->GetPointData()->GetArray(var);
        if( values != NULL)
        {
            vtkDataSet *new_ds = (vtkDataSet*) ds->NewInstance();
            new_ds->CopyStructure(ds);
            new_ds->GetPointData()->AddArray(
                                      ds->GetPointData()->GetArray(var));
            vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
            pd2cd->SetInput(new_ds);
            pd2cd->Update();
            values = pd2cd->GetOutput()->GetCellData()->GetArray(var);
            values->Register(NULL);
            own_values_array =true;
            new_ds->Delete();
            pd2cd->Delete();

        }
        else
        {EXCEPTION1(InvalidVariableException, var);}
    }

    // loop over all cells
    for (int i = 0 ; i < ncells ; i++)
    {
        // get component label, cell value and weight 
        int comp_id = labels->GetValue(i);
        double val    = (double) values->GetTuple1(i);
        double weight = (double) weights->GetTuple1(i);

        // add to total component sum
        sumPerComp[comp_id] += (val * weight);
    }

    if(own_values_array)
        values->Delete();
}

// ****************************************************************************
//  Method: avtConnComponentsWeightedVariableQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
// ****************************************************************************
avtDataObject_p
avtConnComponentsWeightedVariableQuery::ApplyFilters(avtDataObject_p inData)
{
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    // add either areaFilter, or volumeFilter based on input dimension
    int topo = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if (topo == 2)
    {
        if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType()== AVT_XY)
        {
            debug5 << "ConnComponentsWeightedVariable query using "
                   << "Area" << endl;

            areaFilter->SetInput(dob);
            dob = areaFilter->GetOutput();
        }
        else
        {
            debug5 << "ConnComponentsWeightedVariable query using "
                   << "RevolvedVolume" << endl;

            revolvedVolumeFilter->SetInput(dob); 
            dob = revolvedVolumeFilter->GetOutput();
        }
    }
    else
    {
        debug5 << "ConnComponentsWeightedVariable query using "
               << "Volume" << endl;

        volumeFilter->SetInput(dob);
        dob = volumeFilter->GetOutput();
    }

    cclFilter->SetInput(dob);
    dob = cclFilter->GetOutput();

    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    cclFilter->GetOutput()->Update(pspec);

    return cclFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtConnComponentsWeightedVariableQuery::VerifyInput
//
//  Purpose:
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
//
// ****************************************************************************

void
avtConnComponentsWeightedVariableQuery::VerifyInput(void)
{
    avtConnComponentsQuery::VerifyInput();

    avtDataSpecification_p dspec = GetInput()->GetTerminatingSource()
                                     ->GetFullDataSpecification();

    // get the variable name
    avtDataAttributes &dataAtts = GetInput()->GetInfo().GetAttributes();
    variableName = dspec->GetVariable();
}

