/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                       avtConnComponentsCentroidQuery.C                    //
// ************************************************************************* //

#include <avtConnComponentsCentroidQuery.h>

#include <avtCallback.h>
#include <avtConnComponentsExpression.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>


#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>


#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <snprintf.h>


// ****************************************************************************
//  Method: avtConnComponentsCentroidQuery constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
// ****************************************************************************

avtConnComponentsCentroidQuery::avtConnComponentsCentroidQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtConnComponentsCentroidQuery  destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
// ****************************************************************************

avtConnComponentsCentroidQuery::~avtConnComponentsCentroidQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtConnComponentsCentroidQuery::PreExecute
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
avtConnComponentsCentroidQuery::PreExecute(void)
{
    // let base class get the # of connected components
    avtConnComponentsQuery::PreExecute();

    // prepare cell count vector
    nCellsPerComp.resize(nComps);
    // prepare component centroid vectors
    xCentroidPerComp.resize(nComps);
    yCentroidPerComp.resize(nComps);
    zCentroidPerComp.resize(nComps);

    for(int i=0;i<nComps;i++)
    {
        nCellsPerComp[i]    = 0;
        xCentroidPerComp[i] = 0;
        yCentroidPerComp[i] = 0;
        zCentroidPerComp[i] = 0;
    }

}


// ****************************************************************************
//  Method: avtConnComponentsCentroidQuery::PostExecute
//
//  Notes:  Taken mostly from Hank Childs' avtSummationFilter.
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 7, 2007
//
//  Modifications:
//    Cyrus Harrison, Tue Sep 18 09:41:09 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

void
avtConnComponentsCentroidQuery::PostExecute(void)
{
    // get # of cells per component (from all processors)
    int    *sum_res_int = new int[nComps];
    SumIntArrayAcrossAllProcessors(&nCellsPerComp[0], sum_res_int, nComps);
    memcpy(&nCellsPerComp[0],sum_res_int,nComps * sizeof(int));

    delete [] sum_res_int;

    // get centroid values (from all processors)
    double *sum_res_dbl = new double[nComps];

    SumDoubleArrayAcrossAllProcessors(&xCentroidPerComp[0],
                                      sum_res_dbl,
                                      nComps);

    memcpy(&xCentroidPerComp[0],sum_res_dbl,nComps * sizeof(double));

    SumDoubleArrayAcrossAllProcessors(&yCentroidPerComp[0],
                                      sum_res_dbl,
                                      nComps);

    memcpy(&yCentroidPerComp[0],sum_res_dbl,nComps * sizeof(double));

    SumDoubleArrayAcrossAllProcessors(&zCentroidPerComp[0],
                                      sum_res_dbl,
                                      nComps);

    memcpy(&zCentroidPerComp[0],sum_res_dbl,nComps * sizeof(double));

    delete [] sum_res_dbl;

    // create output message

    std::string msg = "";
    char buff[2048];

    if(nComps == 1)
    {SNPRINTF(buff,2048,"Found %d connected component\n",nComps);}
    else
    {SNPRINTF(buff,2048,"Found %d connected components\n",nComps);}

    msg += buff;

    // pack values into a a single vector for query output
    vector<double> result_vec(nComps *3);

    for(int i=0;i<nComps;i++)
    {
        // get number of cells for current component
        double n_comp_cells =  (double)nCellsPerComp[i];
        // calculate centriod values for the current component
        xCentroidPerComp[i] /= n_comp_cells;
        yCentroidPerComp[i] /= n_comp_cells;
        zCentroidPerComp[i] /= n_comp_cells;

        // pack into result vector
        result_vec[i*3 + 0] = xCentroidPerComp[i];
        result_vec[i*3 + 1] = yCentroidPerComp[i];
        result_vec[i*3 + 2] = zCentroidPerComp[i];
    }

    
    string format  =  "Component %d [%d cells] Centroid = (" 
                        + queryAtts.GetFloatFormat()  +","
                        + queryAtts.GetFloatFormat()  +","
                        + queryAtts.GetFloatFormat()  +")\n";
    
    // prepare the output message
    for(int i=0;i<nComps;i++)
    {
        SNPRINTF(buff,1024,
                 format.c_str(),
                 i,
                 nCellsPerComp[i],
                 xCentroidPerComp[i],
                 yCentroidPerComp[i],
                 zCentroidPerComp[i]);

        msg += buff;
    }

    // set result message
    SetResultMessage(msg);

    // set result values
    SetResultValues(result_vec);
}


// ****************************************************************************
//  Method: avtConnComponentsCentroidQuery::Execute
//
//  Purpose:
//      Processes a single domain. Added each cell's centroid value to the 
//      proper component sums.
//
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007
//
// ****************************************************************************

void
avtConnComponentsCentroidQuery::Execute(vtkDataSet *ds, const int dom)
{
    // get the number of cells to process
    int ncells = ds->GetNumberOfCells();
    // get component labels
    vtkIntArray *labels = (vtkIntArray*)ds->GetCellData()->GetArray("avt_ccl");
    if (labels == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_ccl");
    }

    double pt_val[3];

    for (int i = 0 ; i < ncells ; i++)
    {
        // get the cell  & and its component label
        vtkCell *cell = ds->GetCell(i);
        int comp_id = labels->GetValue(i);

        // increment # of cells per component
        nCellsPerComp[comp_id]++;

        // get the cell center
         vtkVisItUtility::GetCellCenter(cell, pt_val);

        // add centroid value
        xCentroidPerComp[comp_id]+= pt_val[0];
        yCentroidPerComp[comp_id]+= pt_val[1];
        zCentroidPerComp[comp_id]+= pt_val[2];

    }
}



