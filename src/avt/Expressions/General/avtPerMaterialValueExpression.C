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
//                  avtPerMaterialValueExpression.C                          //
// ************************************************************************* //

#include <avtPerMaterialValueExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
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

#include <sstream>
#include <string>
#include <vector>

using std::ostringstream;

// ****************************************************************************
//  Method: avtPerMaterialValueExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   January 29, 2008
//
// ****************************************************************************

avtPerMaterialValueExpression::avtPerMaterialValueExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPerMaterialValueExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   January 29, 2008
//
// ****************************************************************************

avtPerMaterialValueExpression::~avtPerMaterialValueExpression()
{
    ;
}



// ****************************************************************************
//  Method: avtPerMaterialValueExpression::DeriveVariable
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
//    Cyrus Harrison, Tue Feb 12 13:38:13 PST 2008
//    Added support for datasets with ghost zones. 
//
//    Cyrus Harrison, Mon Mar 17 10:06:17 PDT 2008
//    Make sure to only request post ghost Material & Mixedvar objects 
//    if the dataset has ghost zones. 
//
//    Cyrus Harrison, Wed Apr  9 11:11:40 PDT 2008
//    Make sure to only do post ghost case if VisIt created the ghost zones.
//
//    Kathleen Bonnell, Tue Jun  3 08:09:52 PDT 2008
//    Remove unreferenced variable.
//
//    Cyrus Harrison,Thu Feb  9 10:26:48 PST 2012
//    Added logic to support presentGhostZoneTypes, which allows us to
//    differentiate between ghost zones for boundaries & nesting.
//
//    David Camp, Thu Jul 17 12:55:02 PDT 2014
//    Tried to optimize the code.
//    Fixed an issue with Threading, the function modified a class member 
//    varable. I create a local variable.
//    Fixed a memory leak if an exception happened.
//
// ****************************************************************************

vtkDataArray *
avtPerMaterialValueExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    // if the cell is clean, we simply need the orig value
    // if the cell is mixed, we need the correct value from the 
    // avtMixedVariable entry.
   
    // get input array
    vtkDataArray *val_array = in_ds->GetCellData()->GetArray(activeVariable);
    if (val_array == NULL) // check for error on input
    {
        ostringstream oss;
        oss << "could not find zonal variable named: " << activeVariable 
            << "." << endl
            << "The value_for_material expression requires a varaible with" 
            << " zonal centering." << endl;

        EXCEPTION2(ExpressionException, outputVariableName, oss.str().c_str());
    }

    // only ask for post ghost Material info if the dataset actually
    // has ghost zones and VisIt created them. 

    bool localDoPostGhost = false;
    if (doPostGhost)  // Did we ask for ghost data?
    {
        // check bitmask to make sure we actually have boundary ghost zones
        // (in some cases we may only have nesting ghosts zones, and post ghost
        // is not the proper path)
        avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
        if (datts.GetContainsGhostZones() == AVT_CREATED_GHOSTS)
        {
            localDoPostGhost = (datts.GetGhostZoneTypesPresent() & AVT_BOUNDARY_GHOST_ZONES) &&
                               in_ds->GetCellData()->GetArray("avtGhostZones");
        }
        // if localDoPostGhost is false then set doPostGhost so we don't try again.
        if (! localDoPostGhost)
            doPostGhost = localDoPostGhost;

        if (DebugStream::Level5())
        {
            debug5 << "avtMatvfExpression: GetGhostZoneTypesPresent() = "
                   << datts.GetGhostZoneTypesPresent() << endl;

            debug5 << "avtPerMaterialValueExpression: Using post ghost material "
                      " and mixedvar objects ?  " << localDoPostGhost << endl;
        }
    }

    // Request ghost adjusted values if required. 
    avtMaterial  *mat = GetMetaData()->GetMaterial(currentDomainsIndex,
                                                   currentTimeState,
                                                   localDoPostGhost);
    if (mat == NULL) // error
    {
       EXCEPTION2(ExpressionException, outputVariableName,
                   "could not obtaing valid material object.");
    }
    
    // get the mixed var
    // note: this will only exist in domains that have mixed vars,
    // so it can be null. 
    // Request ghost adjusted values if required. 
    avtMixedVariable *mvar = GetMetaData()->GetMixedVar(activeVariable,
                                                        currentDomainsIndex,
                                                        currentTimeState,
                                                        localDoPostGhost); 

    // get the material index from give material number or name
    int n_mats = mat->GetNMaterials();
    int mat_idx = -1;
    
    if (matNumber == -1 && matName != "")
    {
        // find material index from name
        for (int i = 0 ; i < n_mats ; ++i)
        {
            std::string curr_mat = mat->GetMaterials()[i];
            if (curr_mat == matName)
            {
                mat_idx = i;
                break;
            }
        }
    }
    else
    {
        char tmp[50];
        sprintf(tmp, "%d", matNumber);
        std::string matname(tmp);

        // match mat # to proper index 
        for (int i = 0 ; i < n_mats ; ++i)
        {
            std::string curr_mat = mat->GetMaterials()[i];
            if (curr_mat == matname ||
                (curr_mat.length() > matname.length() &&
                 curr_mat.substr(0,matname.length() + 1) == (matname + " ")))
            {
                    mat_idx = i;
                    break;
            }
        }
    }
    
    // make sure we actually found a valid material index
    if (mat_idx < 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName,                         
                   "an invalid material number or name was passed.");
    }
    
    // prepare result array
    int ncells = in_ds->GetNumberOfCells();
    vtkDataArray *res = val_array->NewInstance();
    res->SetNumberOfTuples(ncells);

    const int *mat_list = mat->GetMatlist();
    
    // get the proper value
    for (int i=0; i < ncells ;++i)
    {
        float val = 0.0;
        // check for clean case
        if (mat_list[i] == mat_idx)
        {
            // if clean use original value
            val = val_array->GetTuple1(i);
        }
        else if (mvar) 
        {
            std::vector<float> mv_vals;

            // mixed case, use mix var to get the proper value
            mvar->GetValuesForZone(i, mat, mv_vals);
            val = mv_vals[mat_idx];
        }
        
        // set val
        // (will be zero if desired material does not exist in this zone)
        res->SetTuple1(i, val);
    }
    
    return res;
}


// ****************************************************************************
//  Method: avtPerMaterialValueExpression::ProcessArguments
//
//  Purpose:
//      Parses arguments for the avtPerMaterialValueExpression.
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
avtPerMaterialValueExpression::ProcessArguments(ArgsExpr *args,
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
                    "avtPerMaterialValueExpression: No arguments given.");
    }
    // Tell the first argument to create its filters.
    ArgExpr *first_arg= (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        debug5 << "avtPerMaterialValueExpression: Missing second argument." << endl;
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "avtPerMaterialValueExpression: Missing second argument.");
    }

    // Check for too many arguments.
    if (nargs > 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "avtPerMaterialValueExpression expects only two arguments.");
    }
    
    // get material id or name
    ArgExpr *sec_arg= (*arguments)[1];
    avtExprNode *sec_tree= dynamic_cast<avtExprNode*>(sec_arg->GetExpr());
    std::string type = sec_tree->GetTypeName();
    
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
        debug5 << "avtPerMaterialValueExpression: Second argument is not a "
               << "valid material id (integer) or name (string): " 
               << type.c_str() << endl;
        EXCEPTION2(ExpressionException, outputVariableName, 
          "avtPerMaterialValueExpression: Second argument is not a material "
          "number (integer) or name (string).");
    }
}


// ****************************************************************************
//  Method: avtPerMaterialValueExpression::ModifyContract
//
//  Purpose:
//      This routine allows the filter to change the data specification.
//      It is redefined for val4mat to request post ghost material and
//      mixed var info from avtGenericDatabase.
//
//  Programmer: Cyrus Harrison (copied from Hank Childs' matvf implementation)
//  Creation:   January 29, 2008
//
//  Modifications:
//
//    Cyrus Harrison, Tue Feb 12 13:38:13 PST 2008
//    Request post ghost material info, to allow val4mat to be used with  
//    ghost zones.
//
//    Cyrus Harrison, Wed Apr  9 11:04:33 PDT 2008
//    Request post ghost material info as long as we are not doing mir.
//
// ****************************************************************************

avtContract_p
avtPerMaterialValueExpression::ModifyContract(avtContract_p spec)
{
    // request post ghost material info, as long as we are not doing mir
    if( !spec->GetDataRequest()->MustDoMaterialInterfaceReconstruction() )
    {
        spec->GetDataRequest()->SetNeedPostGhostMaterialInfo(true);
        doPostGhost = true;
    }
    else
    {
        doPostGhost = false;
    }
    return spec;
}

