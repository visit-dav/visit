// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

#include <string>

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
//  Modifications:
//    Cyrus Harrison, Tue Sep 18 09:41:09 PDT 2007
//    Added support for user settable floating point format string
//
//    Cyrus Harrison, Tue Mar 31 08:26:51 PDT 2009
//    Only set results on the root processor.
//
//    Kathleen Biagas, Wed Feb 26 11:54:46 PST 2014
//    Add Xml results.
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
    if(PAR_Rank() == 0)
    {
        std::string msg = "";
        char buff[2048];

        if(nComps == 1)
        {snprintf(buff,2048,"Found %d connected component\n",nComps);}
        else
        {snprintf(buff,2048,"Found %d connected components\n",nComps);}

        msg += buff;

        std::string format  =  "Component %d Volume = ("
                                  + queryAtts.GetFloatFormat() +")\n";

        for(int i=0;i<nComps;i++)
        {
            snprintf(buff,1024,
                     format.c_str(),
                     i,
                    volPerComp[i]);

            msg += buff;
        }

        // set output message
        SetResultMessage(msg);
        // set output values
        SetResultValues(volPerComp);
        // set Xml result
        MapNode result_node;
        result_node["connected_component_count"] = nComps;
        result_node["volumes"] = volPerComp;
        SetXmlResult(result_node.ToXML());
    }
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

    avtContract_p contract = 
        inData->GetOriginatingSource()->GetGeneralContract();

    cclFilter->GetOutput()->Update(contract);

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


