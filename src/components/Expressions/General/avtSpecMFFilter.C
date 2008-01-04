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
//                               avtSpecMFFilter.C                           //
// ************************************************************************* //

#include <avtSpecMFFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedIntArray.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <avtCallback.h>
#include <avtMaterial.h>
#include <avtMetaData.h>
#include <avtSpecies.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtSpecMFFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  8, 2004
//
// ****************************************************************************

avtSpecMFFilter::avtSpecMFFilter()
{
    weightByVF = false;
}


// ****************************************************************************
//  Method: avtSpecMFFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  8, 2004
//
// ****************************************************************************

avtSpecMFFilter::~avtSpecMFFilter()
{
}


// ****************************************************************************
//  Method: avtSpecMFFilter::PreExecute
//
//  Purpose:
//      Reset the "issuedWarning" flag.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  8, 2004
//
// ****************************************************************************

void
avtSpecMFFilter::PreExecute(void)
{
    issuedWarning = false;
    avtSingleInputExpressionFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtSpecMFFilter::DeriveVariable
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
//  Programmer:   Jeremy Meredith
//  Creation:     June  8, 2004
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 28 07:48:55 PDT 2004
//    Send currentTimeState to GetMaterial. 
//
// ****************************************************************************

vtkDataArray *
avtSpecMFFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i, j;

    int ncells = in_ds->GetNumberOfCells();

    //
    // The 'currentDomainsIndex' is a data member of the base class that is
    // set to be the id of the current domain right before DeriveVariable is
    // called.  We need that index to make sure we are getting the right
    // materials and species.
    //
    // The 'currentTimeState' is a data member of the base class that is
    // set to be the current timestep during ExamineSpecification. 
    // We need that timestep to make sure we are getting the right 
    // materials and species.
    //
    avtMaterial *mat = GetMetaData()->GetMaterial(currentDomainsIndex,
                                                  currentTimeState);
    avtSpecies  *spec= GetMetaData()->GetSpecies(currentDomainsIndex,
                                                  currentTimeState);
    if (mat == NULL || spec == NULL)
    {
        if (!mat && !spec)
            debug1 << "Could not find a material or species object." << endl;
        else if (!mat)
            debug1 << "Could not find a material object." << endl;
        else
            debug1 << "Could not find a species object." << endl;
        vtkFloatArray *dummy = vtkFloatArray::New();
        dummy->SetNumberOfTuples(ncells);
        for (i = 0 ; i < ncells ; i++)
            dummy->SetTuple1(i, 0.);
        return dummy;
    }

    //
    // Note that we are setting up mf_for_orig_cells based on the number of
    // zones in the original dataset -- this may or may not be the number
    // of cells in the input, depending on whether or not we did MIR.
    //
    vtkFloatArray *mf_for_orig_cells = vtkFloatArray::New();
    mf_for_orig_cells = vtkFloatArray::New();
    int norigcells = mat->GetNZones();
    mf_for_orig_cells->SetNumberOfTuples(norigcells);

    //
    // Try to match up the materials in the avtMaterial object with the
    // materials requested by the users.
    //
    int selectedMat = -1;
    int nmats = mat->GetNMaterials();
    for (i = 0 ; i < nmats ; i++)
    {
        std::string currentMat = mat->GetMaterials()[i];
        if (matNames.size() > 0)
        {
            if (currentMat == matNames[0])
                selectedMat = i;
        }
        else // if (matIndices.size() > 0)
        {
            char tmp[256];
            sprintf(tmp, "%d", matIndices[0]);

            std::string matname(tmp);
            if (currentMat == matname ||
                (currentMat.length() > matname.length() &&
                 currentMat.substr(0,matname.length() + 1) == (matname + " ")))
            {
                selectedMat = i;
            }
        }
    }

    //
    // Make sure we got a real index for our material
    //
    if (selectedMat == -1)
    {
        const std::vector<std::string> &all_mats = 
                                                mat->GetCompleteMaterialList();
        char warningString[100000];
        if (matNames.size() > 0)
            sprintf(warningString, "Could not match up \"%s\" with "
                    "any materials when doing the specmf expression."
                    "\nList of valid materials is: ", 
                    matNames[0].c_str());
        else
            sprintf(warningString, "Could not match up \"%d\" with "
                    "any materials when doing the specmf expression."
                    "\nList of valid materials is: ", 
                    matIndices[0]);
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

        // Return a null array
        vtkFloatArray *dummy = vtkFloatArray::New();
        dummy->SetNumberOfTuples(ncells);
        for (i = 0 ; i < ncells ; i++)
            dummy->SetTuple1(i, 0.);
        return dummy;
    }
    

    //
    // Try to match up the species in the avtMaterial object with the
    // species requested by the users.
    //
    int nspecs = spec->GetNSpecies()[selectedMat];
    std::vector<bool>  useSpec(nspecs, false);
    std::vector<bool>  matchedSpecName(specNames.size(), false);
    std::vector<bool>  matchedSpecIndex(specIndices.size(), false);
    for (i = 0 ; i < nspecs ; i++)
    {
        std::string currentSpec = spec->GetSpecies()[selectedMat][i];
        for (j = 0 ; j < specNames.size() ; j++)
        {
            if (currentSpec == specNames[j])
            {
                useSpec[i] = true;
                matchedSpecName[j] = true;
            }
        }
        for (j = 0 ; j < specIndices.size() ; j++)
        {
            char tmp[256];
            sprintf(tmp, "%d", specIndices[j]);

            std::string specname(tmp);
            if (currentSpec == specname ||
                (currentSpec.length() > specname.length() &&
                 currentSpec.substr(0,specname.length() + 1) == (specname + " ")))
            {
                useSpec[i] = true;
                matchedSpecIndex[j] = true;
            }
        }
    }

    //
    // Make sure that we found every species requested.  If not, issue
    // a warning.
    //
    for (i = 0 ; i < specNames.size() ; i++)
    {
        if (!matchedSpecName[i])
        {
            const std::vector<std::string> &all_specs = 
                                               spec->GetSpecies()[selectedMat];
            bool matched = false;
            for (j = 0 ; j < all_specs.size() ; j++)
            {
                if (specNames[i] == all_specs[j])
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
                    sprintf(warningString, "Could not match up \"%s\" with any"
                              " species when doing the specmf expression.\n"
                              "List of valid species for the given material is: ", 
                              specNames[i].c_str());
                    char *tmp = warningString + strlen(warningString);
                    for (j = 0 ; j < all_specs.size() ; j++)
                    {
                        if (j < (all_specs.size()-1))
                            sprintf(tmp, "\"%s\", ", all_specs[j].c_str());
                        else
                            sprintf(tmp, "\"%s\".", all_specs[j].c_str());
                        tmp += strlen(tmp);
                    }
                    avtCallback::IssueWarning(warningString);
                    issuedWarning = true;
                }
            }
        }
    }
    for (i = 0 ; i < specIndices.size() ; i++)
    {
        char tmp[256];
        sprintf(tmp, "%d", specIndices[i]);

        std::string specname(tmp);
        if (!matchedSpecIndex[i])
        {
            const std::vector<std::string> &all_specs = 
                                               spec->GetSpecies()[selectedMat];
            bool matched = false;
            for (j = 0 ; j < all_specs.size() ; j++)
            {
                if (specname == all_specs[j])
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
                    sprintf(warningString, "Could not match up \"%s\" with any"
                              " species when doing the specmf expression.\n"
                              "List of valid species for the given material is: ", 
                              specname.c_str());
                    char *tmp = warningString + strlen(warningString);
                    for (j = 0 ; j < all_specs.size() ; j++)
                    {
                        if (j < (all_specs.size()-1))
                            sprintf(tmp, "\"%s\", ", all_specs[j].c_str());
                        else
                            sprintf(tmp, "\"%s\".", all_specs[j].c_str());
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
    const int *speclist = spec->GetSpeclist();
    const int *mixspeclist = spec->GetMixSpeclist();
    const float *specmf = spec->GetSpecMF();
    for (i = 0 ; i < norigcells ; i++)
    {
        float weight = 1.0;

        int specIndex = -1;
        if (matlist[i] >= 0)
        {
            if (matlist[i]==selectedMat)
                specIndex = speclist[i];
        }
        else
        {
            int mixix = -matlist[i]-1;

            while (mixix >= 0 && specIndex==-1)
            {
                if (mixmat[mixix] == selectedMat)
                {
                    specIndex = mixspeclist[mixix];
                    if (weightByVF)
                        weight = mixvf[mixix];
                }
                mixix = mix_next[mixix] - 1;
            }
        }

        float mf = 0;
        if (specIndex < 0)
        {
            // no species requested from this material
            mf = 0;
        }
        else if (specIndex == 0)
        {
            // this indicates only one species
            mf = 1;
        }
        else
        {
            for (j=0; j<nspecs; j++)
            {
                if (useSpec[j])
                    mf += specmf[specIndex + j - 1];
            }
        }

        mf_for_orig_cells->SetTuple1(i, mf * weight);
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
        rv = mf_for_orig_cells;
        rv->Register(NULL); // Because mf_for_orig_cells will be deleted later.

        // Sanity check.
        if (norigcells != ncells)
            EXCEPTION0(ImproperUseException);
    }
    else
    {
        //
        // We have the mass fractions for the original cells, but the
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
             rv->SetTuple1(i, mf_for_orig_cells->GetTuple1(id));
        }

    }

    // This was essentially a temporary for us.
    mf_for_orig_cells->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtSpecMFFilter::ProcessArguments
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
//  Programmer:   Jeremy Meredith
//  Creation:     June  8, 2004
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
//    Changed base type for an Arg's expression.
//
//    Jeremy Meredith, Mon Jun 13 11:42:38 PDT 2005
//    Changed the way constant expressions work.
//
//    Brad Whitlock, Tue May 8 10:40:11 PDT 2007
//    Fixed copy+paste error.
//
// ****************************************************************************
void
avtSpecMFFilter::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter: No arguments given.");
    }
    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if there's a second and third argument.
    if (nargs < 3)
    {
        debug5 << "avtSpecMFFilter: Expected three arguments." << endl;
        return;
    }

    // See if there are other arguments.
    if (nargs > 4)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter only expects three "
                   "or four arguments.  To specify more than one species,"
                   " , use a list (e.g. [1,4,5:9].");
    }

    // Pull off the second argument and see if it's a constant.
    ArgExpr *secondarg = (*arguments)[1];
    ExprParseTreeNode *secondTree = secondarg->GetExpr();
    string secondtype = secondTree->GetTypeName();
    if (secondtype != "IntegerConst" && secondtype != "StringConst")
    {
        debug5 << "avtSpecMFFilter: Second argument is not a string or integer constant: " << secondtype.c_str() << endl;
        EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter: Second argument is not a string or integer constant.");
    }

    // It's a single constant.
    AddMaterial(dynamic_cast<ConstExpr*>(secondTree));

    // Pull off the third argument and see if it's a constant or a list.
    ArgExpr *thirdarg = (*arguments)[2];
    ExprParseTreeNode *thirdTree = thirdarg->GetExpr();
    string thirdtype = thirdTree->GetTypeName();
    if ((thirdtype != "IntegerConst") && (thirdtype != "StringConst") && (thirdtype != "List"))
    {
        debug5 << "avtSpecMFFilter: Third argument is not a string/int constant or a list: " << thirdtype.c_str() << endl;
        EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter: Third argument is not a string/int constant or a list.");
    }

    if (thirdtype == "IntegerConst" || thirdtype == "StringConst")
    {
        // It's a single constant.
        AddSpecies(dynamic_cast<ConstExpr*>(thirdTree));
    }
    else
    {
        // It's a list.  Process all of them.
        ListExpr *list = dynamic_cast<ListExpr*>(thirdTree);
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
                    EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter: "
                               "Range must contain integers.");
                }

                int beg  = dynamic_cast<IntegerConstExpr*>(begExpr)->GetValue();
                int end  = dynamic_cast<IntegerConstExpr*>(endExpr)->GetValue();
                int skip = !skipExpr ? 1 : 
                           dynamic_cast<IntegerConstExpr*>(skipExpr)->GetValue();

                if (skip <= 0 || beg > end)
                {
                    EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter: "
                               "Range must be of the form beg:end[:skip].");
                }

                for (int m = beg; m <= end ; m += skip)
                    specIndices.push_back(m);
            }
            else
            {
                ExprNode *item = (*elems)[i]->GetItem();
                string type = item->GetTypeName();
                if (type != "IntegerConst" && type != "StringConst")
                {
                    debug5 << "avtSpecMFFilter: List element is not an "
                              "integer constant, a string constant, "
                              "or a list: " << type.c_str() << endl;
                    EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter: "
                               "List element is not a string/int constant or a list.");
                }

                AddSpecies(dynamic_cast<ConstExpr*>(item));
            }
        }
    }

    // Check to see if they passed in the fourth optional "weight by volume
    // fraction" boolean value.  Default is false.
    weightByVF = false;
    if (nargs == 4)
    {
        ArgExpr *fourtharg = (*arguments)[3];
        ExprParseTreeNode *fourthTree = fourtharg->GetExpr();
        string fourthtype = fourthTree->GetTypeName();
        if ((fourthtype != "BooleanConst"))
        {
            EXCEPTION2(ExpressionException, outputVariableName, "avtSpecMFFilter: Fourth argument is not a constant boolean (true, false).");
        }
        weightByVF = dynamic_cast<BooleanConstExpr*>(fourthTree)->GetValue();
    }
}

// ****************************************************************************
//  Method:  avtSpecMFFilter::AddMaterial
//
//  Purpose:
//    Add a material by name or index to the list
//
//  Arguments:
//    c          The expression to turn into a material
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Jun 13 11:42:38 PDT 2005
//    Changed the way constant expressions work.
//
// ****************************************************************************
void
avtSpecMFFilter::AddMaterial(ConstExpr *c)
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
//  Method:  avtSpecMFFilter::AddSpecies
//
//  Purpose:
//    Add a species by index to the list
//
//  Arguments:
//    c          The expression to turn into a species
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Jun 13 11:42:38 PDT 2005
//    Changed the way constant expressions work.
//
// ****************************************************************************
void
avtSpecMFFilter::AddSpecies(ConstExpr *c)
{
    if (c->GetConstantType() == ConstExpr::String)
    {
        string specname = dynamic_cast<StringConstExpr*>(c)->GetValue();
        specNames.push_back(specname);
    }
    else // c->GetConstantType() == ConstExpr::Integer
    {
        int specindex = dynamic_cast<IntegerConstExpr*>(c)->GetValue();
        specIndices.push_back(specindex);
    }
}


// ****************************************************************************
//  Method: avtSpecMFFilter::PerformRestriction
//
//  Purpose:
//      This routine allows the filter to change the data specification.
//      It is only re-defined for specmf to get around a shortcoming in the
//      generic database.  The problem is that if ghost zones are communicated,
//      the new avtMaterial object (if it exists at all), is not stored 
//      anywhere for this filter to get.  Instead, this filter will get the
//      original avtMaterial object and quickly have an internal error.
//
//      This could be solved in the generic database, but require a fairly
//      large re-organization.  For more information, look at '3939.
//
//  Note: Copied verbatim by JSM June 8, 2004 from avtMatvfFilter.
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
avtSpecMFFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    spec->GetDataSpecification()->SetMaintainOriginalConnectivity(true);
    return spec;
}


