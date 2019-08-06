// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtConnComponentsCentroidQuery.C                    //
// ************************************************************************* //

#include <avtConnComponentsCentroidQuery.h>

#include <avtCallback.h>
#include <avtConnComponentsExpression.h>
#include <avtParallel.h>


#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>


#include <DebugStream.h>
#include <InvalidVariableException.h>

#include <string>
#include <vector>

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
//    Cyrus Harrison, Tue Mar 31 08:26:51 PDT 2009
//    Only set results on the root processor.
//
//    Kathleen Biagas, Wed Feb 26 12:03:51 PST 2014
//    Add Xml results.
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

    if(PAR_Rank() == 0)
    {
        std::string msg = "";
        char buff[2048];

        if(nComps == 1)
        {snprintf(buff,2048,"Found %d connected component\n",nComps);}
        else
        {snprintf(buff,2048,"Found %d connected components\n",nComps);}

        msg += buff;

        // pack values into a a single vector for query output
        std::vector<double> result_vec(nComps *3);

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

        std::string format  =  "Component %d [%d cells] Centroid = (" 
                            + queryAtts.GetFloatFormat()  +","
                            + queryAtts.GetFloatFormat()  +","
                            + queryAtts.GetFloatFormat()  +")\n";

        // prepare the output message
        for(int i=0;i<nComps;i++)
        {
            snprintf(buff,1024,
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

        // set xml result
        MapNode result_node;
        result_node["centroids"] = result_vec;
        result_node["connected_component_count"] = nComps;
        result_node["cell_counts"] = nCellsPerComp;
        SetXmlResult(result_node.ToXML());
    }
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



