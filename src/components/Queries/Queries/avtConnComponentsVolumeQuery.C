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
//                        avtConnComponentsVolumeQuery.C                     //
// ************************************************************************* //

#include <avtConnComponentsVolumeQuery.h>

#include <avtCallback.h>
#include <avtConnComponentsExpression.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtRevolvedVolume.h>
#include <avtVMetricVolume.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidDimensionsException.h>
#include <snprintf.h>

// ****************************************************************************
//  Method: avtConnComponentsVolumeQuery constructor
//
//  Notes:  
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
// ****************************************************************************

avtConnComponentsVolumeQuery::avtConnComponentsVolumeQuery()
{
    revolvedVolumeFilter = new avtRevolvedVolume;
    revolvedVolumeFilter->SetOutputVariableName("avt_weight");

    volumeFilter = new avtVMetricVolume;
    volumeFilter->SetOutputVariableName("avt_weight");
}


// ****************************************************************************
//  Method: avtConnComponentsVolumeQuery  destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
// ****************************************************************************

avtConnComponentsVolumeQuery::~avtConnComponentsVolumeQuery()
{
    delete revolvedVolumeFilter;
    delete volumeFilter;
}


// ****************************************************************************
//  Method: avtConnComponentsVolumeQuery::PreExecute
//
//  Notes:  Adapted from avtSummationQuery
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
// ****************************************************************************

void
avtConnComponentsVolumeQuery::PreExecute(void)
{
    // let base class get the # of connected components
    avtConnComponentsQuery::PreExecute();

    
    // prepare component arrays 
    volPerComp.resize(nComps);
    for(int i=0;i<nComps;i++)
    {
        volPerComp[i] = 0;
    }
}


// ****************************************************************************
//  Method: avtConnComponentsVolumeQuery::PostExecute
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
// ****************************************************************************

void
avtConnComponentsVolumeQuery::PostExecute(void)
{

    double *sum_res_dbl = new double[nComps];
    SumDoubleArrayAcrossAllProcessors(&volPerComp[0], sum_res_dbl, nComps);
    memcpy(&volPerComp[0],sum_res_dbl,nComps * sizeof(double));

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
                 "Component %d Volume = (%f)\n",
                 i,
                 volPerComp[i]);

        msg += buff;
    }
    
    // set output message
    SetResultMessage(msg);
    // set output values
    SetResultValues(volPerComp);
}


// ****************************************************************************
//  Method: avtConnComponentsVolumeQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Notes:  
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007
//
//
// ****************************************************************************

void
avtConnComponentsVolumeQuery::Execute(vtkDataSet *ds, const int dom)
{
    int ncells = ds->GetNumberOfCells();
    // get cell labels and the weight filter
    vtkIntArray *labels =(vtkIntArray*)ds->GetCellData()->GetArray("avt_ccl");    
    vtkDataArray *vol= ds->GetCellData()->GetArray("avt_weight");

    // make sure our data arrays are valid
    if (vol == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_weight");
    }

    if (labels == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_ccl");
    }


    for (int i = 0 ; i < ncells ; i++)
    {
        // get the cell  & and its component label
        vtkCell *cell = ds->GetCell(i);
        int comp_id = labels->GetValue(i);

        // get cell volume
        double vol_val = (double) vol->GetTuple1(i);

        // add to total component volume
        volPerComp[comp_id] += vol_val;
    }
}



// ****************************************************************************
//  Method: avtConnComponentsVolumeQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
// ****************************************************************************
avtDataObject_p
avtConnComponentsVolumeQuery::ApplyFilters(avtDataObject_p inData)
{
    // Create an artificial pipeline.
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();


    // add either areaFilter, or volumeFilter based on input dimension
    int topo = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if (topo == 2)
    {
        if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() != AVT_XY)
        {
            debug5 << "ConnComponentsVolume query using RevolvedVolume" 
                   << endl;
            revolvedVolumeFilter->SetInput(dob);
            dob = revolvedVolumeFilter->GetOutput();
        }
    }
    else
    {
        debug5 << "ConnComponentsVolume query using Volume" << endl;
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
//  Method: avtConnComponentsVolumeQuery::VerifyInput
//
//  Purpose:  Throw an exception for invalid input.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************
void 
avtConnComponentsVolumeQuery::VerifyInput()
{
    //
    //  Make sure base class verify is still called 
    //

    avtConnComponentsQuery::VerifyInput();

    //
    //  Make sure we can calculate volume 
    //  (Must be 3D, or a 2D revolved type)
    //

    int topo = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if (topo < 2)
    {
        EXCEPTION2(InvalidDimensionsException, 
                   "Connected Component Volume Query",
                   "3D or Revolved Volume");
    }
    else if (topo == 2)
    {
        if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() == AVT_XY)
        {
            EXCEPTION2(InvalidDimensionsException, 
                       "Connected Component Volume Query",
                       "3D or Revolved Volume");
        }
    }
}


