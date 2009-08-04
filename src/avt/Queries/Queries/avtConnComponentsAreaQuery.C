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
//                          avtConnComponentsAreaQuery.C                     //
// ************************************************************************* //

#include <avtConnComponentsAreaQuery.h>

#include <avtCallback.h>
#include <avtConnComponentsExpression.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtVMetricArea.h>
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
//  Method: avtConnComponentsAreaQuery constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************

avtConnComponentsAreaQuery::avtConnComponentsAreaQuery()
{
    // create the area filter
    // (base class creates the connected components filter)
    areaFilter = new avtVMetricArea;
    areaFilter->SetOutputVariableName("avt_weight");
}


// ****************************************************************************
//  Method: avtConnComponentsAreaQuery  destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************

avtConnComponentsAreaQuery::~avtConnComponentsAreaQuery()
{
    delete areaFilter;
}


// ****************************************************************************
//  Method: avtConnComponentsAreaQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed to obtain the
//      number of components and initialize component area sums.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
// ****************************************************************************

void
avtConnComponentsAreaQuery::PreExecute(void)
{
    // let base class get the # of connected components
    avtConnComponentsQuery::PreExecute();

    // prepare component arrays 
    areaPerComp.resize(nComps);
    for(int i=0;i<nComps;i++)
    {
        areaPerComp[i] = 0;
    }
}


// ****************************************************************************
//  Method: avtConnComponentsAreaQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed to collect
//      info from all processors and finalize component area values.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
//  Modifications:
//    Cyrus Harrison, Tue Sep 18 09:41:09 PDT 2007
//    Added support for user settable floating point format string
//
//    Cyrus Harrison, Tue Mar 31 08:26:51 PDT 2009
//    Only set results on the root processor.
//
// ****************************************************************************

void
avtConnComponentsAreaQuery::PostExecute(void)
{
    double *sum_res_dbl = new double[nComps];
    SumDoubleArrayAcrossAllProcessors(&areaPerComp[0], sum_res_dbl, nComps);
    memcpy(&areaPerComp[0],sum_res_dbl,nComps * sizeof(double));

    delete [] sum_res_dbl;

    // create output message

    if(PAR_Rank() == 0)
    {
        std::string msg = "";
        char buff[2048];

        if(nComps == 1)
        {SNPRINTF(buff,2048,"Found %d connected component\n",nComps);}
        else
        {SNPRINTF(buff,2048,"Found %d connected components\n",nComps);}

        msg += buff;
        string format  =  "Component %d Area = (" 
                              + queryAtts.GetFloatFormat() +")\n";
        for(int i=0;i<nComps;i++)
        {
            SNPRINTF(buff,1024,
                     format.c_str(),
                     i,
                    areaPerComp[i]);

            msg += buff;
        }

        SetResultMessage(msg);
        SetResultValues(areaPerComp);
    }
}


// ****************************************************************************
//  Method: avtConnComponentsAreaQuery::Execute
//
//  Purpose:
//      Processes a single input domain to update the per component area sums.
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
avtConnComponentsAreaQuery::Execute(vtkDataSet *ds, const int dom)
{
    // get the number of cells to process
    int ncells = ds->GetNumberOfCells();

    // get the component labels and volume arrays
    vtkIntArray  *labels =(vtkIntArray*)ds->GetCellData()->GetArray("avt_ccl");
    vtkDataArray *vol= ds->GetCellData()->GetArray("avt_weight");

    // make sure the volume and component label arrays are valid
    if (vol == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_weight");
    }

    if (labels == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_ccl");
    }

    // loop over all cells
    for (int i = 0 ; i < ncells ; i++)
    {
        // get the cell  & and its component label
        vtkCell *cell    = ds->GetCell(i);
        int      comp_id = labels->GetValue(i);

        // get cell area
        double area_val = (double) vol->GetTuple1(i);

        // add to total component area
        areaPerComp[comp_id] += area_val;
    }
}


// ****************************************************************************
//  Method: avtConnComponentsAreaQuery::ApplyFilters
//
//  Purpose:
//      Constructs an artificial pipeline with the connected components and
//      area filters necessary to obtain per component area.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************
avtDataObject_p
avtConnComponentsAreaQuery::ApplyFilters(avtDataObject_p inData)
{
    // Create an artificial pipeline.
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    // add the area filter to the pipeline
    areaFilter->SetInput(dob);
    dob = areaFilter->GetOutput();

    // add the ccl filter to the pipeline
    cclFilter->SetInput(dob);
    dob = cclFilter->GetOutput();

    avtContract_p contract = 
        inData->GetOriginatingSource()->GetGeneralContract();

    cclFilter->GetOutput()->Update(contract);

    return cclFilter->GetOutput();
}

// ****************************************************************************
//  Method: avtConnComponentsVolumeQuery::VerifyInput
//
//  Purpose:  Throw an exception for invalid input. Ensures that the input
//            is 2D.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007
//
// ****************************************************************************
void
avtConnComponentsAreaQuery::VerifyInput()
{
    //  Make sure base class verify is still called.
    avtConnComponentsQuery::VerifyInput();

    //  Make sure we can calculate the area (Must be a 2D mesh)
    if(GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 2)
    {
        EXCEPTION2(InvalidDimensionsException, 
                   "Connected Components Area Query",
                   "2D");
    }

}


