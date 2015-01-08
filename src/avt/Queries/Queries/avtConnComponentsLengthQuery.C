/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                          avtConnComponentsLengthQuery.C                     //
// ************************************************************************* //

#include <avtConnComponentsLengthQuery.h>

#include <avtCallback.h>
#include <avtConnComponentsExpression.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtEdgeLength.h>

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

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtConnComponentsLengthQuery constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Jun 15 13:09:43 PDT 2011
//
// ****************************************************************************

avtConnComponentsLengthQuery::avtConnComponentsLengthQuery()
{
    // create the length filter
    // (base class creates the connected components filter)
    lengthFilter = new avtEdgeLength;
    lengthFilter->SetOutputVariableName("avt_weight");
}


// ****************************************************************************
//  Method: avtConnComponentsLengthQuery  destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Jun 15 13:09:43 PDT 2011
//
// ****************************************************************************

avtConnComponentsLengthQuery::~avtConnComponentsLengthQuery()
{
    delete lengthFilter;
}


// ****************************************************************************
//  Method: avtConnComponentsLengthQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed to obtain the
//      number of components and initialize component area sums.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Jun 15 13:09:43 PDT 2011
//
// ****************************************************************************

void
avtConnComponentsLengthQuery::PreExecute(void)
{
    // let base class get the # of connected components
    avtConnComponentsQuery::PreExecute();

    // prepare component arrays
    lengthPerComp = std::vector<double>(nComps,0.0);
}


// ****************************************************************************
//  Method: avtConnComponentsLengthQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed to collect
//      info from all processors and finalize component area values.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Jun 15 13:09:43 PDT 2011
//
//  Modifications:
//    Kathleen Biagas, Wed Feb 26 12:00:54 PST 2014
//    Add Xml results.
//
// ****************************************************************************

void
avtConnComponentsLengthQuery::PostExecute(void)
{
    double *sum_res_dbl = new double[nComps];
    SumDoubleArrayAcrossAllProcessors(&lengthPerComp[0], sum_res_dbl, nComps);
    memcpy(&lengthPerComp[0],sum_res_dbl,nComps * sizeof(double));

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
        std::string format  =  "Component %d Length = (" 
                              + queryAtts.GetFloatFormat() +")\n";
        for(int i=0;i<nComps;i++)
        {
            SNPRINTF(buff,1024,
                     format.c_str(),
                     i,
                    lengthPerComp[i]);

            msg += buff;
        }

        SetResultMessage(msg);
        SetResultValues(lengthPerComp);
        MapNode result_node;
        result_node["connected_components_count"] = nComps;
        result_node["lengths"] = lengthPerComp;
        SetXmlResult(result_node.ToXML());
    }
}


// ****************************************************************************
//  Method: avtConnComponentsLengthQuery::Execute
//
//  Purpose:
//      Processes a single input domain to update the per component area sums.
//
//  Arguments:
//      ds       Input dataset
//      dom      Input domain number
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Jun 15 13:09:43 PDT 2011
//
// ****************************************************************************
void
avtConnComponentsLengthQuery::Execute(vtkDataSet *ds, const int dom)
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

    double len_val = 0.0;
    // loop over all cells
    for (int i = 0 ; i < ncells ; i++)
    {
        // get the component label
        int      comp_id = labels->GetValue(i);

        // get cell area
        len_val = (double) vol->GetTuple1(i);

        // add to total component area
        lengthPerComp[comp_id] += len_val;
    }
}


// ****************************************************************************
//  Method: avtConnComponentsLengthQuery::ApplyFilters
//
//  Purpose:
//      Constructs an artificial pipeline with the connected components and
//      area filters necessary to obtain per component area.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Jun 15 13:09:43 PDT 2011
//
// ****************************************************************************
avtDataObject_p
avtConnComponentsLengthQuery::ApplyFilters(avtDataObject_p inData)
{
    // Create an artificial pipeline.
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    // add the area filter to the pipeline
    lengthFilter->SetInput(dob);
    dob = lengthFilter->GetOutput();

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
//  Creation:   Wed Jun 15 13:09:43 PDT 2011
//
// ****************************************************************************
void
avtConnComponentsLengthQuery::VerifyInput()
{
    //  Make sure base class verify is still called.
    avtConnComponentsQuery::VerifyInput();

    //  Make sure we can calculate the length (Must be a line mesh)
    if(GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        EXCEPTION2(InvalidDimensionsException, 
                   "Connected Components Length Query",
                   "1D");
    }

}


