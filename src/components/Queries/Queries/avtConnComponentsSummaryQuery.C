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
//                     avtConnComponentsSummaryQuery.C                       //
// ************************************************************************* //

#include <avtConnComponentsSummaryQuery.h>

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
//  Method: avtConnComponentsSummaryQuery constructor
//
//  Notes:  
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007
//
// ****************************************************************************

avtConnComponentsSummaryQuery::
avtConnComponentsSummaryQuery()
{
    // (base class creates the connected components filter)
    // create weight filters

    areaFilter = new avtVMetricArea;
    areaFilter->SetOutputVariableName("avt_area");

    revolvedVolumeFilter = new avtRevolvedVolume;
    revolvedVolumeFilter->SetOutputVariableName("avt_rvolume");

    volumeFilter = new avtVMetricVolume;
    volumeFilter->SetOutputVariableName("avt_volume");

    outputFileName = "cc_summary.okc";

}

// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery  destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007
//
// ****************************************************************************

avtConnComponentsSummaryQuery::
~avtConnComponentsSummaryQuery()
{
    delete areaFilter;
    delete revolvedVolumeFilter;
    delete volumeFilter;
}


// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery::PreExecute
//
//
//  Purpose:
//      This is called before all of the domains are executed to obtain the
//      number of components and initialize component sums.
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007
//
// ****************************************************************************

void
avtConnComponentsSummaryQuery::PreExecute(void)
{
    // let base class get the # of connected components
    avtConnComponentsQuery::PreExecute();

    // prepare component vectors

    // cell count
    nCellsPerComp.resize(nComps);

    // centroid vectors
    xCentroidPerComp.resize(nComps);
    yCentroidPerComp.resize(nComps);
    zCentroidPerComp.resize(nComps);

    // area & volume vectors
    if(findArea)
        areaPerComp.resize(nComps);
    if(findVolume)
        volPerComp.resize(nComps);

    // sum vectors
    sumPerComp.resize(nComps);
    wsumPerComp.resize(nComps);

    for(int i=0;i<nComps;i++)
    {
        nCellsPerComp[i] = 0;

        xCentroidPerComp[i] = 0;
        yCentroidPerComp[i] = 0;
        zCentroidPerComp[i] = 0;

        if(findArea)
            areaPerComp[i] = 0;
        if(findVolume)
            volPerComp[i] = 0;

        sumPerComp[i] = 0;
        wsumPerComp[i] = 0;
    }
}


// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed to collect
//      info from all processors and finalize component sums. 
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007
//
// ****************************************************************************

void
avtConnComponentsSummaryQuery::PostExecute(void)
{
    // sum vals across all processors

    // temp arrays for sum results
    int    *sum_res_int = new int[nComps];
    double *sum_res_dbl = new double[nComps];
    

    // get # of cells per component (from all processors)
    SumIntArrayAcrossAllProcessors(&nCellsPerComp[0], sum_res_int, nComps);
    memcpy(&nCellsPerComp[0],sum_res_int,nComps * sizeof(int));

    // get centroid values (from all processors)


    // get x centroid sum
    SumDoubleArrayAcrossAllProcessors(&xCentroidPerComp[0],
                                      sum_res_dbl,
                                      nComps);
    memcpy(&xCentroidPerComp[0],sum_res_dbl,nComps * sizeof(double));

    // get y centroid sum
    SumDoubleArrayAcrossAllProcessors(&yCentroidPerComp[0],
                                      sum_res_dbl,
                                      nComps);
    memcpy(&yCentroidPerComp[0],sum_res_dbl,nComps * sizeof(double));

    // get z centroid sum
    SumDoubleArrayAcrossAllProcessors(&zCentroidPerComp[0],
                                      sum_res_dbl,
                                      nComps);
    memcpy(&zCentroidPerComp[0],sum_res_dbl,nComps * sizeof(double));

    if(findArea)
    {
        // get area sum 
        SumDoubleArrayAcrossAllProcessors(&areaPerComp[0], sum_res_dbl, nComps);
        memcpy(&areaPerComp[0],sum_res_dbl,nComps * sizeof(double));
    }

    if(findVolume)
    {
        // get volume sum 
        SumDoubleArrayAcrossAllProcessors(&volPerComp[0], sum_res_dbl, nComps);
        memcpy(&volPerComp[0],sum_res_dbl,nComps * sizeof(double));
    }


    // get sum res
    SumDoubleArrayAcrossAllProcessors(&sumPerComp[0], sum_res_dbl, nComps);
    memcpy(&sumPerComp[0],sum_res_dbl,nComps * sizeof(double));

    // get weighted sum res
    SumDoubleArrayAcrossAllProcessors(&wsumPerComp[0], sum_res_dbl, nComps);
    memcpy(&wsumPerComp[0],sum_res_dbl,nComps * sizeof(double));


    delete [] sum_res_int;
    delete [] sum_res_dbl;

    // get final centroid values
    for(int i=0;i<nComps;i++)
    {
        // get number of cells for current component
        double n_comp_cells =  (double)nCellsPerComp[i];
        // calculate centriod values for the current component
        xCentroidPerComp[i] /= n_comp_cells;
        yCentroidPerComp[i] /= n_comp_cells;
        zCentroidPerComp[i] /= n_comp_cells;
    }

    // make sure we got a valid output file name (not simply the var name)
    if(outputFileName == variableName)
        outputFileName = "cc_summary.okc";

    // create output message


    std::string msg = "";
    char buff[2048];

    if(nComps == 1)
    {SNPRINTF(buff,2048,"Found %d connected component.\n",nComps);}
    else
    {SNPRINTF(buff,2048,"Found %d connected components.\n",nComps);}

    msg += buff;

    msg += "Component summary information saved to " + outputFileName;
    msg += ", which can be imported into VisIt";

    // set output message
    SetResultMessage(msg);

    if(PAR_Rank() == 0)
    {
        // save results to output okc file
        SaveComponentResults(outputFileName);
    }
 
    vector<double> results;
    PrepareComponentResults(results);

    // set output values
    SetResultValues(results);
}


// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery::Execute
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
avtConnComponentsSummaryQuery::Execute(vtkDataSet *ds, const int dom)
{
    const char *var = variableName.c_str();
    bool own_values_array = false;
    // get the number of cells to process
    int ncells = ds->GetNumberOfCells();
    // get component label array 
    vtkIntArray  *labels =(vtkIntArray*)ds->GetCellData()->GetArray("avt_ccl");
    // get component value array 
    vtkDataArray *values  =ds->GetCellData()->GetArray(var);

    // attemp to get area, volume and revolved volume arrays
    vtkDataArray *areas =ds->GetCellData()->GetArray("avt_area");
    vtkDataArray *volumes =ds->GetCellData()->GetArray("avt_volume");
    vtkDataArray *rvolumes =ds->GetCellData()->GetArray("avt_rvolume");


    // make sure the cell arrays are valid
    if (labels == NULL)
    {
        EXCEPTION1(InvalidVariableException, "avt_ccl");
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


    double pt_val[3];

    // loop over all cells
    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = ds->GetCell(i);
        // get component label
        int comp_id = labels->GetValue(i);

        // increment # of cells per component
        nCellsPerComp[comp_id]++;

        // get the cell center
         vtkVisItUtility::GetCellCenter(cell, pt_val);

        // add centroid value
        xCentroidPerComp[comp_id]+= pt_val[0];
        yCentroidPerComp[comp_id]+= pt_val[1];
        zCentroidPerComp[comp_id]+= pt_val[2];

        // update sum value
        double val    = (double) values->GetTuple1(i);
        sumPerComp[comp_id] += val;

        double weight = 0;

        if(findArea)
        {
            // get area value
            weight = areas->GetTuple1(i);
            // update area
            areaPerComp[comp_id] += weight;
        }
        
        if(findVolume)
        {
            // check for volume result first
            if(volumes)
            {
                weight = volumes->GetTuple1(i);
                // update volume 
                volPerComp[comp_id]+= weight;
            }
            // use revolved volume if volume result does not exist
            else if(rvolumes)
            {
                weight = rvolumes->GetTuple1(i);
                // update volume 
                volPerComp[comp_id]+= weight;
            }
        }

        // update weighted sum value 
        wsumPerComp[comp_id] += (val * weight);
    }

    if(own_values_array)
        values->Delete();
}

// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
// ****************************************************************************
avtDataObject_p
avtConnComponentsSummaryQuery::ApplyFilters(avtDataObject_p inData)
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
            debug5 << "ConnComponentsSummary query using "
                   << "Area for weighted sum" << endl;

            areaFilter->SetInput(dob);
            dob = areaFilter->GetOutput();
            findArea = true;
            findVolume = false;
        }
        else
        {
            debug5 << "ConnComponentsSummary query using "
                   << "RevolvedVolume for weighted sum" << endl;
        
            // add area and revolved volume filter
            revolvedVolumeFilter->SetInput(dob); 
            dob = revolvedVolumeFilter->GetOutput();
            areaFilter->SetInput(dob); 
            dob = areaFilter->GetOutput();

            findArea   = true;
            findVolume = true;
        }
    }
    else
    {
        debug5 << "ConnComponentsSummary query using "
               << "Volume for weighted sum" << endl;

        volumeFilter->SetInput(dob);
        dob = volumeFilter->GetOutput();
        findArea = false;
        findVolume = true;
    }

    cclFilter->SetInput(dob);
    dob = cclFilter->GetOutput();

    avtContract_p contract = 
        inData->GetOriginatingSource()->GetGeneralContract();

    cclFilter->GetOutput()->Update(contract);

    return cclFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery::VerifyInput
//
//  Purpose: Gets the active variable name.
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007
//
// ****************************************************************************

void
avtConnComponentsSummaryQuery::VerifyInput(void)
{
    avtConnComponentsQuery::VerifyInput();

    avtDataRequest_p dataRequest = GetInput()->GetOriginatingSource()
                                     ->GetFullDataRequest();

    // get the variable name
    avtDataAttributes &dataAtts = GetInput()->GetInfo().GetAttributes();
    variableName = dataRequest->GetVariable();
}

// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery::SaveComponentResults
//
//  Purpose: Save component results to an Xmdv file.
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007
//
// ****************************************************************************

void
avtConnComponentsSummaryQuery::SaveComponentResults(string fname)
{
    // look index
    int i;

    // open file

    ofstream outs;
    outs.open(fname.c_str());

    char msg[1024];

    if(!outs.is_open())
    {
        sprintf(msg, "Unable to save component summary output to %s",fname.c_str());
        SetResultMessage(string(msg));
        return;
    }

    // write Xmdv header
    
    //
    // write # of cols and # of rows 
    //

    int nrows = nComps;
    int ncols = 7;

    // inc # of columns if we are including area and/or volume
    if(findArea)
        ncols++;
    if(findVolume)
        ncols++;
    
    outs << ncols << " " << nrows << " 1" << endl;

    // write column names

    outs << "comp_x" << endl;
    outs << "comp_y" << endl;
    outs << "comp_z" << endl;

    outs << "comp_label" << endl;
    outs << "comp_num_cells" << endl;

    if(findArea)
        outs << "comp_area" << endl;
   
    if(findVolume)
        outs << "comp_volume" << endl;
   
    outs << "comp_sum" << endl;
    outs << "comp_weighted_sum" << endl;
   
    // find component ranges
    int ncells_min    =  INT_MAX;
    int ncells_max    =  0;
    
    double cent_x_min =  DBL_MAX;
    double cent_x_max = -DBL_MAX;
   
    double cent_y_min =  DBL_MAX;
    double cent_y_max = -DBL_MAX;

    double cent_z_min =  DBL_MAX;
    double cent_z_max = -DBL_MAX;

    double area_min   =  DBL_MAX;
    double area_max   = -DBL_MAX;

    double volume_min =  DBL_MAX;
    double volume_max = -DBL_MAX;

    double sum_min    =  DBL_MAX;
    double sum_max    = -DBL_MAX;

    double wsum_min   =  DBL_MAX;
    double wsum_max   = -DBL_MAX;    

    for(i = 0; i < nComps;i++)
    {
        //  update # of cells range
        if(ncells_min > nCellsPerComp[i])
            ncells_min = nCellsPerComp[i];

        if(ncells_max < nCellsPerComp[i])
            ncells_max = nCellsPerComp[i];

        //  update x centroid range
        if(cent_x_min > xCentroidPerComp[i])
            cent_x_min = xCentroidPerComp[i];

        if(cent_x_max < xCentroidPerComp[i])
            cent_x_max = xCentroidPerComp[i];

        //  update y centroid range
        if(cent_y_min > yCentroidPerComp[i])
            cent_y_min = yCentroidPerComp[i];

        if(cent_y_max < yCentroidPerComp[i])
            cent_y_max = yCentroidPerComp[i];

        //  update z centroid range
        if(cent_z_min > yCentroidPerComp[i])
            cent_z_min = yCentroidPerComp[i];

        if(cent_z_max < zCentroidPerComp[i])
            cent_z_max = zCentroidPerComp[i];
        
        if(findArea)
        {
            // update area range
            if(area_min > areaPerComp[i])
                area_min = areaPerComp[i];

            if(area_max < areaPerComp[i])
                area_max = areaPerComp[i];
        }

        if(findVolume)
        {
            // update area range
            if(volume_min > volPerComp[i])
                volume_min = volPerComp[i];
                    
            if(volume_max < volPerComp[i])
                volume_max = volPerComp[i];
        }

        // update var sum
        if(sum_min > sumPerComp[i])
            sum_min = sumPerComp[i];

        if(sum_max < sumPerComp[i])
            sum_max = sumPerComp[i];

        // update weighted var sum
        if(wsum_min > wsumPerComp[i])
            wsum_min = wsumPerComp[i];

        if(wsum_max < wsumPerComp[i])
            wsum_max = wsumPerComp[i];

    }
    // write column ranges

    // centroid coord ranges
    outs << cent_x_min << "\t" << cent_x_max << "\t10"<< endl;
    outs << cent_y_min << "\t" << cent_y_max << "\t10"<< endl;
    outs << cent_z_min << "\t" << cent_z_max << "\t10"<< endl;

    // label range
    outs << 0 << " " << nrows -1 << "\t10"<< endl;
    // # of cells range
    outs << ncells_min << "\t" << ncells_max<< "\t10"<< endl;
    
    // area range
    if(findArea)
        outs << area_min << "\t" << area_max << "\t10"<< endl;

    // area range
    if(findVolume)
        outs << volume_min << "\t" << volume_max  << "\t10"<< endl;        
    
    // var sum range
    outs << sum_min << "\t" << sum_max << "\t10"<< endl;

    // weighted var sum range
    outs << wsum_min << "\t" << wsum_max << "\t10"<< endl;

    // write each component as a line
    for( i = 0; i < nrows; i++)
    {
        // centroid
        outs << xCentroidPerComp[i] << "\t";
        outs << yCentroidPerComp[i] << "\t";
        outs << zCentroidPerComp[i] << "\t";

        // label
        outs << i << "\t";

        // # of cells
        outs << nCellsPerComp[i] << "\t";    

        // area 
        if(findArea)
            outs << areaPerComp[i] << "\t";    

        // volume 
        if(findVolume)
            outs << volPerComp[i] << "\t";

        // var sum
        outs << sumPerComp[i] << "\t";         

        // weighted var sum
        outs << wsumPerComp[i] << "\t"; 
        outs << endl;        
    }


    // close file
    outs.close();
}

// ****************************************************************************
//  Method: avtConnComponentsSummaryQuery::PackResults
//
//  Purpose: Packs component summary results for return with Set
//
//  Programmer: Cyrus Harrison
//  Creation:   March 2, 2007
//
// ****************************************************************************

void
avtConnComponentsSummaryQuery::PrepareComponentResults(vector<double> &results)
{
    //
    // resize ouput vector to hold all results
    // since we are not sending header info, we provide all fields
    // even if they are not present (such as area, or volume)
    // They are set to zero if vals do not exist
    //

    results.resize(nComps *9);

    // loop index
    int i;
    // result cursor
    int idx = 0;

    for( i = 0 ;i < nComps; i++)
    {
        // centroid results
        results[idx++] = xCentroidPerComp[i];
        results[idx++] = yCentroidPerComp[i];
        results[idx++] = zCentroidPerComp[i];

        // label
        results[idx++] = i;

        // number of cells
        results[idx++] = nCellsPerComp[i];

        // area
        if(findArea)
            results[idx++] = areaPerComp[i];
        else
            results[idx++] = 0;

        // volume
        if(findVolume)
            results[idx++] = volPerComp[i];
        else
            results[idx++] = 0;

        // var sum
        results[idx++] = sumPerComp[i];

        // var weighted sum
        results[idx++] = wsumPerComp[i];
    }
}
