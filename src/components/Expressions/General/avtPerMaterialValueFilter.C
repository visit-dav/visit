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
//                      avtPerMaterialValueFilter.C                          //
// ************************************************************************* //

#include <avtPerMaterialValueFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedIntArray.h>

#include <avtExprNode.h>
#include <ExprToken.h>

#include <avtCallback.h>
#include <avtMaterial.h>
#include <avtMetaData.h>
#include <avtMixedVariable.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>
#include <visitstream.h>
using std::ostringstream;

// ****************************************************************************
//  Method: avtPerMaterialValueFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   January 29, 2008
//
// ****************************************************************************

avtPerMaterialValueFilter::avtPerMaterialValueFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtPerMaterialValueFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   January 29, 2008
//
// ****************************************************************************

avtPerMaterialValueFilter::~avtPerMaterialValueFilter()
{
    ;
}



// ****************************************************************************
//  Method: avtPerMaterialValueFilter::DeriveVariable
//
//  Purpose:
//      Extracts the proper zonal value for the given material.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Cyrus Harrison
//  Creation:     January 29, 2008
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtPerMaterialValueFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i, j;

    int ncells = in_ds->GetNumberOfCells();
    
    // if the cell is clean, we simply need the orig value
    
    // if the cell is mixed, we need the correct value from the 
    // avtMixedVariable entry.
   
    // get input array
    vtkFloatArray *val_array = NULL;
    val_array = (vtkFloatArray *)in_ds->GetCellData()->GetArray(activeVariable);
    
    if(val_array == NULL) // check for error on input
    {
        ostringstream oss;
        oss << "could not find zonal variable named: " << activeVariable 
            << "." << endl
            << "The value_for_material expression requires a varaible with" 
            << " zonal centering." <<endl;
            
        EXCEPTION2(ExpressionException, outputVariableName,oss.str().c_str());
    
    }


    // prepare result array
    vtkFloatArray *res = vtkFloatArray::New();
    res->SetNumberOfTuples(ncells);
    
    avtMaterial      *mat = GetMetaData()->GetMaterial(currentDomainsIndex,
                                                       currentTimeState);
    if(mat == NULL ) // error
    {
       EXCEPTION2(ExpressionException, outputVariableName,                         
                   "could not obtaing valid material object.");
    }
    
    // get the mixed var
    // note: this will only exists in domains that have mixed vars,
    // so it can be null
    avtMixedVariable *mvar = GetMetaData()->GetMixedVar(activeVariable,
                                                        currentDomainsIndex,
                                                        currentTimeState); 
    
    // get the material index from give material number or name
    int n_mats = mat->GetNMaterials();
    int mat_idx = -1;
    
    if(matNumber == -1 && matName != "")
    {
        // find material index from name
        for (i = 0 ; i < n_mats  && mat_idx == -1; i++)
        {
            std::string curr_mat = mat->GetMaterials()[i];
            if(curr_mat == matName)
                mat_idx = i;
        }
    }
    else
    {
        // match mat # to proper index 
        for (i = 0 ; i < n_mats  && mat_idx == -1; i++)
        {
            std::string curr_mat = mat->GetMaterials()[i];
            char tmp[256];
            sprintf(tmp, "%d", matNumber);

            std::string matname(tmp);
            if (curr_mat == matname ||
                (curr_mat.length() > matname.length() &&
                 curr_mat.substr(0,matname.length() + 1) == (matname + " ")))
                {
                    mat_idx = i;
                }
        }
    }
    
    // make sure we actually found a valid material index
    if(mat_idx < 0  || mat_idx > n_mats)
    {
        EXCEPTION2(ExpressionException, outputVariableName,                         
                   "an invalid material number or name was passed.");
    }
    
    std::vector<float> mv_vals;
    const int *mat_list = mat->GetMatlist();
    
    // get the proper value
    for(i=0;i<ncells;i++)
    {
        float val = 0.0;
        // check for clean case
        if( mat_list[i]  == mat_idx)
        {
            // if clean use original value
            val = val_array->GetTuple1(i);
        }
        else if(mvar != NULL) 
        {
            // mixed case, use mix var to get the proper value
            mvar->GetValuesForZone(i,mat,mv_vals);
            val = mv_vals[mat_idx];
        }
        
        // set val
        // (will be zero if desired material does not exist in this zone)
        res->SetTuple1(i,val);
    }
    
    return res;
}


// ****************************************************************************
//  Method: avtPerMaterialValueFilter::ProcessArguments
//
//  Purpose:
//      Parses arguments for the avtPerMaterialValueFilter.
//      Expected arguments are:
//        variable name and material number or name.
//
//  Programmer:   Cyrus Harrison
//  Creation:     January 29, 2008
//
//  Modifications:
//
// ****************************************************************************
void
avtPerMaterialValueFilter::ProcessArguments(ArgsExpr *args,
                                            ExprPipelineState *state)
{
    // clear mat info
    matNumber = -1;
    matName   = "";

    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
        
    // get var name
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                    "avtPerMaterialValueFilter: No arguments given.");
    }
    // Tell the first argument to create its filters.
    ArgExpr *first_arg= (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        debug5 << "avtPerMaterialValueFilter: Missing second argument." << endl;
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "avtPerMaterialValueFilter: Missing second argument.");
    }

    // Check for too many arguments.
    if (nargs > 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "avtPerMaterialValueFilter expects only two arguments.");
    }
    
    // get material id or name
    ArgExpr *sec_arg= (*arguments)[1];
    avtExprNode *sec_tree= dynamic_cast<avtExprNode*>(sec_arg->GetExpr());
    string type = sec_tree->GetTypeName();
    
    if (type == "IntegerConst")
    {
        matNumber = dynamic_cast<IntegerConstExpr*>(sec_tree)->GetValue();
    }
    else if (type == "StringConst")
    {
        matName = dynamic_cast<StringConstExpr*>(sec_tree)->GetValue();
    }
    else
    {
        debug5 << "avtPerMaterialValueFilter: Second argument is not a valid "
                  "material id (integer) or name (string): " 
                << type.c_str() << endl;
        EXCEPTION2(ExpressionException, outputVariableName, 
          "avtPerMaterialValueFilter: Second argument is not a material "
          "number (integer) or name (string).");
    }

}


// ****************************************************************************
//  Method: avtPerMaterialValueFilter::PerformRestriction
//
//  Purpose:
//      This routine allows the filter to change the data specification.
//      It is only re-defined for matvf to get around a shortcoming in the
//      generic database.  The problem is that if ghost zones are communicated,
//      the new avtMaterial object (if it exists at all), is not stored 
//      anywhere for this filter to get.  Instead, this filter will get the
//      original avtMaterial object and quickly have an internal error.
//
//      This could be solved in the generic database, but require a fairly
//      large re-organization.  For more information, look at '3939.
//
//  Programmer: Cyrus Harrison (copied from Hank Childs' matvf implementation)
//  Creation:   January 29, 2008
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtPerMaterialValueFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    spec->GetDataSpecification()->SetMaintainOriginalConnectivity(true);
    return spec;
}


