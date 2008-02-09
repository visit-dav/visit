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
//                               avtMatvfExpression.C                            //
// ************************************************************************* //

#include <avtMatvfExpression.h>

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

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtMatvfExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMatvfExpression::avtMatvfExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMatvfExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMatvfExpression::~avtMatvfExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMatvfExpression::PreExecute
//
//  Purpose:
//      Reset the "issuedWarning" flag.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2004
//
// ****************************************************************************

void
avtMatvfExpression::PreExecute(void)
{
    issuedWarning = false;
    avtSingleInputExpressionFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtMatvfExpression::DeriveVariable
//
//  Purpose:
//      Assigns the material fraction to each point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Tue Mar 18 23:19:39 America/Los_Angeles 2003
//
//  Modifications:
//
//    Hank Childs, Tue Aug 12 10:21:49 PDT 2003
//    Wrote the code to actually find the material volume fractions (the
//    previous code was just a place-holder).
//
//    Jeremy Meredith, Mon Sep 29 12:13:04 PDT 2003
//    Added support for integer material indices.
//
//    Hank Childs, Wed Feb 18 09:26:33 PST 2004
//    Issue a warning if we can't match up an argument with a material.  Also
//    fixed indexing bug.
//
//    Kathleen Bonnell, Mon Jun 28 07:48:55 PDT 2004
//    Send currentTimeState to GetMaterial. 
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed memory leak caused by cut-n-paste error
//
// ****************************************************************************

vtkDataArray *
avtMatvfExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int    i, j;

    int ncells = in_ds->GetNumberOfCells();

    //
    // The 'currentDomainsIndex' is a data member of the base class that is
    // set to be the id of the current domain right before DeriveVariable is
    // called.  We need that index to make sure we are getting the right mat.
    //
    // The 'currentTimeState' is a data member of the base class that is
    // set to be the current timestep during ExamineSpecification. 
    // We need that timestep to make sure we are getting the right mat.
    //
    avtMaterial *mat = GetMetaData()->GetMaterial(currentDomainsIndex,
                                                  currentTimeState);
    if (mat == NULL)
    {
        debug1 << "Could not find a material object." << endl;
        vtkFloatArray *dummy = vtkFloatArray::New();
        dummy->SetNumberOfTuples(ncells);
        for (i = 0 ; i < ncells ; i++)
            dummy->SetTuple1(i, 0.);
        return dummy;
    }

    //
    // Note that we are setting up vf_for_orig_cells based on the number of
    // zones in the original dataset -- this may or may not be the number
    // of cells in the input, depending on whether or not we did MIR.
    //
    vtkFloatArray *vf_for_orig_cells = vtkFloatArray::New();
    int norigcells = mat->GetNZones();
    vf_for_orig_cells->SetNumberOfTuples(norigcells);

    //
    // Try to match up the materials in the avtMaterial object with the
    // materials requested by the users.
    //
    int nmats = mat->GetNMaterials();
    std::vector<bool>  useMat(nmats, false);
    std::vector<bool>  matchedMatName(matNames.size(), false);
    std::vector<bool>  matchedMatIndex(matIndices.size(), false);
    for (i = 0 ; i < nmats ; i++)
    {
        std::string currentMat = mat->GetMaterials()[i];
        for (j = 0 ; j < matNames.size() ; j++)
        {
            if (currentMat == matNames[j])
            {
                useMat[i] = true;
                matchedMatName[j] = true;
            }
        }
        for (j = 0 ; j < matIndices.size() ; j++)
        {
            char tmp[256];
            sprintf(tmp, "%d", matIndices[j]);

            std::string matname(tmp);
            if (currentMat == matname ||
                (currentMat.length() > matname.length() &&
                 currentMat.substr(0,matname.length() + 1) == (matname + " ")))
            {
                useMat[i] = true;
                matchedMatIndex[j] = true;
            }
        }
    }

    //
    // Make sure that we found every material requested.  If not, issue
    // a warning.
    //
    for (i = 0 ; i < matNames.size() ; i++)
    {
        if (!matchedMatName[i])
        {
            const std::vector<std::string> &all_mats = 
                                                mat->GetCompleteMaterialList();
            bool matched = false;
            for (j = 0 ; j < all_mats.size() ; j++)
            {
                if (matNames[i] == all_mats[j])
                {
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                if (!issuedWarning)
                {
                    char warningString[100000];
                    sprintf(warningString, "Could not match up \"%s\" with "
                              "any materials when doing the matvf expression."
                              "\nList of valid materials is: ", 
                              matNames[i].c_str());
                    char *tmp = warningString + strlen(warningString);
                    for (j = 0 ; j < all_mats.size() ; j++)
                    {
                        if (j < (all_mats.size()-1))
                            sprintf(tmp, "\"%s\", ", all_mats[j].c_str());
                        else
                            sprintf(tmp, "\"%s\".", all_mats[j].c_str());
                        tmp += strlen(tmp);
                    }
                    avtCallback::IssueWarning(warningString);
                    issuedWarning = true;
                }
            }
        }
    }
    for (i = 0 ; i < matIndices.size() ; i++)
    {
        char tmp[256];
        sprintf(tmp, "%d", matIndices[i]);

        std::string matname(tmp);
        if (!matchedMatIndex[i])
        {
            const std::vector<std::string> &all_mats = 
                                                mat->GetCompleteMaterialList();
            bool matched = false;
            for (j = 0 ; j < all_mats.size() ; j++)
            {
                if (matname == all_mats[j])
                {
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                if (!issuedWarning)
                {
                    char warningString[100000];
                    sprintf(warningString, "Could not match up \"%s\" with "
                              "any materials when doing the matvf expression."
                              "\nList of valid materials is: ", 
                              matname.c_str());
                    char *tmp = warningString + strlen(warningString);
                    for (j = 0 ; j < all_mats.size() ; j++)
                    {
                        if (j < (all_mats.size()-1))
                            sprintf(tmp, "\"%s\", ", all_mats[j].c_str());
                        else
                            sprintf(tmp, "\"%s\".", all_mats[j].c_str());
                        tmp += strlen(tmp);
                    }
                    avtCallback::IssueWarning(warningString);
                    issuedWarning = true;
                }
            }
        }
    }
    
    //
    // Walk through the material data structure and calculate the volume
    // fraction for each cell.
    //
    const int *matlist = mat->GetMatlist();
    const int *mixmat = mat->GetMixMat();
    const float *mixvf = mat->GetMixVF();
    const int *mix_next = mat->GetMixNext();
    for (i = 0 ; i < norigcells ; i++)
    {
        float vf = 0.;
        if (matlist[i] >= 0)
        {
            vf = (useMat[matlist[i]] ? 1. : 0.);
        }
        else
        {
            vf = 0.;
            int current = -matlist[i]-1;

            // iterations < 1000 just to prevent infinite loops if someone
            // set this structure up wrong.
            int iterations = 0;
            bool stillMoreMats = true;
            while (stillMoreMats && (iterations < 1000))
            {
                if (useMat[mixmat[current]])
                {
                    vf += mixvf[current];
                }
                if (mix_next[current] == 0)
                    stillMoreMats = false;
                else
                    current = mix_next[current]-1;
                iterations++;
            }
        }
        vf_for_orig_cells->SetTuple1(i, vf);
    }

    bool zonesMatchMaterialObject = GetInput()->GetInfo().GetValidity().
                                            GetZonesPreserved();
    vtkFloatArray *rv = NULL;
    if (zonesMatchMaterialObject)
    {
        //
        // We have the volume fractions for the original cells and we are
        // operating on the original cells -- we're done.
        //
        rv = vf_for_orig_cells;
        rv->Register(NULL); // Because vf_for_orig_cells will be deleted later.

        // Sanity check.
        if (norigcells != ncells)
            EXCEPTION0(ImproperUseException);
    }
    else
    {
        //
        // We have the volume fractions for the original cells, but the
        // original cells have been modified -- most likely by MIR.  Use
        // their original indexing to determine the volume fractions.
        //
        rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(ncells);

        vtkUnsignedIntArray *ids = (vtkUnsignedIntArray *)
                      in_ds->GetCellData()->GetArray("avtOriginalCellNumbers");
        if (ids == NULL)
        {
            EXCEPTION0(ImproperUseException);
        }
        int ncomps = ids->GetNumberOfComponents();
        unsigned int *ptr = ids->GetPointer(0);
        for (i = 0 ; i < ncells ; i++)
        {
             //
             // The id's are poorly arranged.  There may be one or two
             // components -- always with zones, sometimes with domains.
             // The zones are always the last --> ncomps-1 st component.
             //
             unsigned int id = ptr[ncomps*i + (ncomps-1)];
             rv->SetTuple1(i, vf_for_orig_cells->GetTuple1(id));
        }

    }

    // This was essentially a temporary for us.
    vf_for_orig_cells->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtMatvfExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  Parses the second
//      argument into a list of material names.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Tue Mar 18 23:20:06 America/Los_Angeles 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 29 12:13:04 PDT 2003
//    Added support for integer material indices.
//    Added support for integer ranges.
//
//    Hank Childs, Thu Jul 15 14:44:17 PDT 2004
//    Make sure the base pointer type for the dynamic cast is in the 
//    inheritance tree of what we are downcasting type. ('5201)
//
//    Jeremy Meredith, Wed Nov 24 12:26:21 PST 2004
//    Renamed EngineExprNode to avtExprNode due to a refactoring.
//    Also renamed Token to ExprToken for the same reason.
//    Changed the base type for an Arg's expression.
//
//    Jeremy Meredith, Mon Jun 13 11:42:38 PDT 2005
//    Changed the way constant expressions work.
//
// ****************************************************************************
void
avtMatvfExpression::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "avtMatvfExpression: No arguments given.");
    }
    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        debug5 << "avtMatvfExpression: No second argument." << endl;
        return;
    }

    // See if there are other arguments.
    if (nargs > 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "avtMatvfExpression only expects two "
                   "arguments.  To specify more than one material, use a "
                   "list (e.g. [1,4,5:9].");
    }

    // Pull off the second argument and see if it's a string or a list.
    ArgExpr *secondarg = (*arguments)[1];
    ExprParseTreeNode *secondTree = secondarg->GetExpr();
    string type = secondTree->GetTypeName();
    if ((type != "IntegerConst") && (type != "StringConst") && (type != "List"))
    {
        debug5 << "avtMatvfExpression: Second argument is not a constant or a list: " << type.c_str() << endl;
        EXCEPTION2(ExpressionException, outputVariableName, "avtMatvfExpression: Second argument is not a constant or a list.");
    }

    if (type == "IntegerConst" || type == "StringConst")
    {
        // It's a single constant.
        AddMaterial(dynamic_cast<ConstExpr*>(secondTree));
    }
    else
    {
        // It's a list.  Process all of them.
        ListExpr *list = dynamic_cast<ListExpr*>(secondTree);
        std::vector<ListElemExpr*> *elems = list->GetElems();
        for(int i=0;i<elems->size();i++)
        {
            if ((*elems)[i]->GetEnd())
            {
                // it's a range
                ExprNode *begExpr  = (*elems)[i]->GetBeg();
                ExprNode *endExpr  = (*elems)[i]->GetEnd();
                ExprNode *skipExpr = (*elems)[i]->GetSkip();
                
                if (begExpr->GetTypeName() != "IntegerConst" ||
                    endExpr->GetTypeName() != "IntegerConst" ||
                    (skipExpr && skipExpr->GetTypeName() != "IntegerConst"))
                {
                    EXCEPTION2(ExpressionException, outputVariableName, "avtMatvfExpression: "
                               "Range must contain integers.");
                }

                int beg  = dynamic_cast<IntegerConstExpr*>(begExpr)->GetValue();
                int end  = dynamic_cast<IntegerConstExpr*>(endExpr)->GetValue();
                int skip = !skipExpr ? 1 : 
                           dynamic_cast<IntegerConstExpr*>(skipExpr)->GetValue();

                if (skip <= 0 || beg > end)
                {
                    EXCEPTION2(ExpressionException, outputVariableName, "avtMatvfExpression: "
                               "Range must be of the form beg:end[:skip].");
                }

                for (int m = beg; m <= end ; m += skip)
                    matIndices.push_back(m);
            }
            else
            {
                ExprNode *item = (*elems)[i]->GetItem();
                string type = item->GetTypeName();
                if (type != "IntegerConst" && type != "StringConst")
                {
                    debug5 << "avtMatvfExpression: List element is not an "
                              "integer constant, a string constant, "
                              "or a list: " << type.c_str() << endl;
                    EXCEPTION2(ExpressionException, outputVariableName, "avtMatvfExpression: "
                               "List element is not an int/string constant "
                               "or a list.");
                }

                AddMaterial(dynamic_cast<ConstExpr*>(item));
            }
        }
    }
}

// ****************************************************************************
//  Method:  avtMatvfExpression::AddMaterial
//
//  Purpose:
//    Add a material by name or index to the list
//
//  Arguments:
//    c          The expression to turn into a material
//
//  Programmer:  Sean Ahern
//  Creation:    March 18, 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 29 12:13:04 PDT 2003
//    Added support for integer material indices.
//
//    Jeremy Meredith, Mon Jun 13 11:42:38 PDT 2005
//    Changed the way constant expressions work.
//
// ****************************************************************************
void
avtMatvfExpression::AddMaterial(ConstExpr *c)
{
    if (c->GetConstantType() == ConstExpr::String)
    {
        string matname = dynamic_cast<StringConstExpr*>(c)->GetValue();
        matNames.push_back(matname);
    }
    else // c->GetConstantType() == ConstExpr::Integer
    {
        int matindex = dynamic_cast<IntegerConstExpr*>(c)->GetValue();
        matIndices.push_back(matindex);
    }
}


// ****************************************************************************
//  Method: avtMatvfExpression::PerformRestriction
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
//  Programmer: Hank Childs
//  Creation:   October 24, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Aug 11 08:03:38 PDT 2004
//    Account for changes in the data specification.
//
// ****************************************************************************

avtPipelineSpecification_p
avtMatvfExpression::PerformRestriction(avtPipelineSpecification_p spec)
{
    spec->GetDataSpecification()->SetMaintainOriginalConnectivity(true);
    return spec;
}


