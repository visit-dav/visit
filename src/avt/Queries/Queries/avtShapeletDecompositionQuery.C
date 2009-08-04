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
//                     avtShapeletDecompositionQuery.C                       //
// ************************************************************************* //

#include <avtShapeletDecompositionQuery.h>

#include <avtCallback.h>
#include <avtShapeletBasisSet.h>
#include <avtShapeletDecompose.h>
#include <avtShapeletReconstruct.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>

#include <vtkCellData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidDimensionsException.h>
#include <snprintf.h>

#include <MapNode.h>

// ****************************************************************************
//  Method: avtShapeletDecompositionQuery constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   December 14, 2007
//
// ****************************************************************************

avtShapeletDecompositionQuery::avtShapeletDecompositionQuery()
:decompResult(NULL)
{
    beta = 1.0;
    nmax = 1;
    decompOutputFileName = "";
    recompOutputFileName = "";
}


// ****************************************************************************
//  Method: avtShapeletDecompositionQuery destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   December 14, 2007
//
// ****************************************************************************

avtShapeletDecompositionQuery::~avtShapeletDecompositionQuery()
{;}


// ****************************************************************************
//  Method: avtShapeletDecompositionQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed. 
//      (Not yet needed)
//
//  Programmer: Cyrus Harrison
//  Creation:   December 17, 2007
//
// ****************************************************************************

void
avtShapeletDecompositionQuery::PreExecute(void)
{
    if(decompResult)
        delete decompResult;
}


// ****************************************************************************
//  Method: avtShapeletDecompositionQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed to prepare the
//      query result.
//
//  Programmer: Cyrus Harrison
//  Creation:   December 14, 2007
//
//  Modifications:
//    Cyrus Harrison, Wed Jun 11 15:29:15 PDT 2008
//    Added original data extents to the MapNode for GetQueryOutputObject().
//
// ****************************************************************************

void
avtShapeletDecompositionQuery::PostExecute(void)
{
    std::string msg = "";
    char buff[256];
    string float_format = queryAtts.GetFloatFormat();
    if(decompResult)
    {
        msg = "Shapelet decomposition using beta(";
        SNPRINTF(buff,256,float_format.c_str(),beta);
        msg +=buff;
        msg +=") and nmax(";
        SNPRINTF(buff,256,"%d",nmax);
        msg +=buff;
        msg +=") yielded a reconstruction error of ";
        SNPRINTF(buff,256,float_format.c_str(),recompError);
        msg +=buff;
        msg +=".\nYou can access the shapelet decomposition result in VisIt's "
              " cli via \"GetQueryOutputObject()\".\n";
        
        // set output message
        SetResultMessage(msg);
        // set output as coeffs
        SetResultValues(decompResult->Coefficients());
    
        // return more info using QueryXmlResult
        MapNode result_node;
        result_node["beta"]    = decompResult->Beta();
        result_node["nmax"]    = decompResult->NMax();
        result_node["width"]   = decompResult->Width();
        result_node["height"]  = decompResult->Height();
        result_node["extents"] = decompResult->Extents();
        result_node["coeffs"]  = decompResult->Coefficients();
        result_node["error"]   = recompError;
        SetXmlResult(result_node.ToXML());
        
        if(decompOutputFileName!="")
        {
            ofstream ofs;
            ofs.open(decompOutputFileName.c_str());
            ofs << result_node.ToXML();
            ofs.close();
        }
    }
    else // error!
    {
        SetResultMessage("Error constructing Shapelet Decomposition!");
        doubleVector empty;
        SetResultValues(empty);
        SetXmlResult("");
    }
}


// ****************************************************************************
//  Method: avtShapeletDecompositionQuery::Execute
//
//  Purpose:
//      Processes a single domain. 
//
//  Programmer: Cyrus Harrison
//  Creation:   December 18, 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed Jan  2 08:15:07 PST 2008
//    Use vtkVisItUtility to write dataset, to fix compile problem on Windows.
//
// ****************************************************************************

void
avtShapeletDecompositionQuery::Execute(vtkDataSet *ds, const int dom)
{
    // clear any prev result
    if(decompResult)
    {
        delete decompResult;
        decompResult = NULL;
    }
    
    // make sure set is rectilinear
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        EXCEPTION1(VisItException, 
                   "The Shapelet Decomposition Query only operates on 2D"
                   " rectilinear grids.");
    }
    
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
    int dims[3];
    rgrid->GetDimensions(dims);
    
    if (dims[2] > 1)
    {
        EXCEPTION2(InvalidDimensionsException, "Shapelet Decomposition",
                                               "2-dimensional");
    }
    
    // make sure this is a zonal varaible
    string var = queryAtts.GetVariables()[0];
    
    
    vtkDataArray *vals = rgrid->GetCellData()->GetArray(var.c_str());
    if(!vals)
    {   
        EXCEPTION1(InvalidVariableException, var.c_str());
    }
    
    
    // get width & height
    int width  = dims[0]-1;
    int height = dims[1]-1;
   
    recompError = 1.0; // set error to 1.0 (max)
    // create basis set
    avtShapeletBasisSet basis_set(beta,nmax,width,height);
   
    avtShapeletDecompose   decomp;
    avtShapeletReconstruct recomp;
    // do decomp  

    decompResult = decomp.Execute(rgrid,var,&basis_set);
    
    if(decompResult)
    {
        // do recomp
        vtkRectilinearGrid *recomp_res = recomp.Execute(decompResult,var,&basis_set);
        // calc error
       
        recompError = ComputeError(rgrid,recomp_res,var);
        
        if(recompOutputFileName != "")
        {
            // save output 
            vtkVisItUtility::WriteDataSet(recomp_res, 
                                          recompOutputFileName.c_str());
        }
        // delete recomp image
        recomp_res->Delete();
    }
    
}

// ****************************************************************************
//  Method: avtShapeletDecompositionQuery::ComputeError
//
//  Purpose:
//      Computes Shapelet reconstruction error.
//
//  Programmer: Cyrus Harrison
//  Creation:   December 18, 2007
//
// ****************************************************************************

double
avtShapeletDecompositionQuery::ComputeError(vtkRectilinearGrid *a,
                                            vtkRectilinearGrid *b,
                                            const string &var_name)
{
    vtkDataArray *a_arr = a->GetCellData()->GetArray(var_name.c_str());
    vtkDataArray *b_arr = b->GetCellData()->GetArray(var_name.c_str());
    
    if(!a_arr || !b_arr)
        return 1.0;
    int a_len = a_arr->GetNumberOfTuples();
    int b_len = b_arr->GetNumberOfTuples();
    
    if(!a_arr || !b_arr)
        return 1.0;
    double a_sum = 0.0;
    for(int i=0;i<a_len;i++)
        a_sum += fabs(a_arr->GetTuple1(i));

    // error (percentage) == abs(a_val - b_val) / a_sum
    double res_error = 0.0;
    for(int i=0;i<a_len;i++)
    {
        res_error += fabs(a_arr->GetTuple1(i) - b_arr->GetTuple1(i));
    }   
    res_error = res_error / a_sum;
    return res_error;
}


