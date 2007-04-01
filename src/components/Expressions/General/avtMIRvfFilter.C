// ************************************************************************* //
//                               avtMIRvfFilter.C                            //
// ************************************************************************* //

#include <avtMIRvfFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>

#include <avtExprNode.h>
#include <ExprToken.h>

#include <avtCallback.h>
#include <avtMaterial.h>
#include <avtMetaData.h>
#include <avtSILRestrictionTraverser.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtMIRvfFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

avtMIRvfFilter::avtMIRvfFilter()
{
    issuedWarning = false;
}


// ****************************************************************************
//  Method: avtMIRvfFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

avtMIRvfFilter::~avtMIRvfFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMIRvfFilter::PreExecute
//
//  Purpose:
//      Reset the "issuedWarning" flag.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

void
avtMIRvfFilter::PreExecute(void)
{
    issuedWarning = false;
    avtSingleInputExpressionFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtMIRvfFilter::DeriveVariable
//
//  Purpose:
//      Assigns the volume fraction to each point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     December 31, 2004
//
// ****************************************************************************

vtkDataArray *
avtMIRvfFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i;

    vtkDataArray *volume = in_ds->GetCellData()->GetArray(volume_name.c_str());
    vtkDataArray *zoneid = in_ds->GetCellData()->GetArray(zoneid_name.c_str());
    if (volume == NULL || zoneid == NULL)
    {
        EXCEPTION1(ExpressionException, "The arguments to MIR VF were not "
                      "created properly.");
    }
    vtkIntArray *matnum = (vtkIntArray *)
                           in_ds->GetCellData()->GetArray("avtSubsets");
    if (matnum == NULL)
    {
        EXCEPTION1(ExpressionException, "MIR VF not able to locate materials");
    }

    //
    // Determine the biggest zone id.
    //
    int ncells = in_ds->GetNumberOfCells();
    int biggest_zone = -1;
    for (i = 0 ; i < ncells ; i++)
    {
        int z = (int) zoneid->GetTuple1(i);
        biggest_zone = (z > biggest_zone ? z : biggest_zone);
    }
    float *total_vol = new float[biggest_zone+1];
    float *mat_vol   = new float[biggest_zone+1];
    for (i = 0 ; i < biggest_zone+1 ; i++)
    {
        total_vol[i] = 0.;
        mat_vol[i]   = 0.;
    }

    //
    // Whole zones were broken into subzones by the MIR.  We want our
    // values to correspond to the whole zones.  So iterate over the subzones
    // and try to determine what the volumes are for the whole zones.  We want
    // to know the total volume for each whole zone and the volume devoted to
    // the materials specified.
    // 
    std::vector<bool> useMat;
    GetMaterialList(useMat);
    for (i = 0 ; i < ncells ; i++)
    {
        int z = (int) zoneid->GetTuple1(i);
        float vol = volume->GetTuple1(i);
        int mat = matnum->GetValue(i);
        total_vol[z] += vol;
        if (useMat[mat])
            mat_vol[z] += vol;
    }

    //
    // Now that we know the total volume and the volume for each material
    // for each whole zone, we can calculate the volume fraction and then
    // assign that volume fraction to each subzone.
    //
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(ncells);
    for (i = 0 ; i < ncells ; i++)
    {
        int z = (int) zoneid->GetTuple1(i);
        if (total_vol[z] != 0.)  // should always be non-zero, but who knows?
            rv->SetTuple1(i, mat_vol[z] / total_vol[z]);
        else
            rv->SetTuple1(i, 0.);
    }

    delete [] mat_vol;
    delete [] total_vol;

    return rv;
}


// ****************************************************************************
//  Method: avtMIRvfFilter::GetMaterialList
//
//  Purpose:
//      Determines the materials we should use when calculating the MIR VF.
//
//  Notes:      This routine was taken wholly from avtMatvfFilter.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

void
avtMIRvfFilter::GetMaterialList(std::vector<bool> &useMat)
{
    int    i, j;

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
        EXCEPTION1(ExpressionException, "Unable to match up material names.");

    //
    // Try to match up the materials in the avtMaterial object with the
    // materials requested by the users.
    //
    int nmats = mat->GetNMaterials();
    useMat.resize(nmats, false);
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
}


// ****************************************************************************
//  Method: avtMIRvfFilter::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  Parses the fourth
//      argument into a list of material names.
//
//  Notes:        This routine was directly taken from avtMatvfFilter.
//
//  Programmer:   Hank Childs
//  Creation:     December 31, 2004
//
// ****************************************************************************
void
avtMIRvfFilter::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs != 4)
    {
        EXCEPTION1(ExpressionException, "avtMIRvfFilter: Incorrect # of "
             "arguments.  (mat-name, zone-id, volume, material list)."
             "To specify more than one material, use a list (e.g. [1,4,5:9].");
    }
    // Tell the first argument (material name) to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Tell the second argument (zone ids) to create its filters.
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    secondTree->CreateFilters(state);
    zoneid_name = secondarg->GetText();

    // Tell the third argument (volume) to create its filters.
    ArgExpr *thirdarg = (*arguments)[2];
    avtExprNode *thirdTree = dynamic_cast<avtExprNode*>(thirdarg->GetExpr());
    thirdTree->CreateFilters(state);
    volume_name = thirdarg->GetText();

    // Pull off the fourth argument and see if it's a string or a list.
    ArgExpr *fourtharg = (*arguments)[3];
    ExprParseTreeNode *fourthTree = fourtharg->GetExpr();
    string type = fourthTree->GetTypeName();
    if ((type != "Const") && (type != "List"))
    {
        debug5 << "avtMIRvfFilter: Second argument is not a constant or a list: " << type.c_str() << endl;
        EXCEPTION1(ExpressionException, "avtMIRvfFilter: Second argument is not a constant or a list.");
    }

    if (type == "Const")
    {
        // It's a single constant.
        AddMaterial(dynamic_cast<ConstExpr*>(fourthTree));
    }
    else
    {
        // It's a list.  Process all of them.
        ListExpr *list = dynamic_cast<ListExpr*>(fourthTree);
        std::vector<ListElemExpr*> *elems = list->GetElems();
        for(int i=0;i<elems->size();i++)
        {
            if ((*elems)[i]->GetEnd())
            {
                // it's a range
                ExprNode *begExpr  = (*elems)[i]->GetBeg();
                ExprNode *endExpr  = (*elems)[i]->GetEnd();
                ExprNode *skipExpr = (*elems)[i]->GetSkip();
                
                if (begExpr->GetTypeName() != "Const" ||
                    endExpr->GetTypeName() != "Const" ||
                    (skipExpr && skipExpr->GetTypeName() != "Const"))
                {
                    EXCEPTION1(ExpressionException, "avtMIRvfFilter: "
                               "Range must contain integers.");
                }

                ExprToken *begTok  = dynamic_cast<ConstExpr*>(begExpr)->GetToken();
                ExprToken *endTok  = dynamic_cast<ConstExpr*>(endExpr)->GetToken();
                ExprToken *skipTok = !skipExpr ? NULL :
                                     dynamic_cast<ConstExpr*>(skipExpr)->GetToken();

                if (begTok->GetType() != TT_IntegerConst ||
                    endTok->GetType() != TT_IntegerConst ||
                    (skipTok && skipTok->GetType() != TT_IntegerConst))
                {
                    EXCEPTION1(ExpressionException, "avtMIRvfFilter: "
                               "Range must contain integers.");
                }

                int beg  = dynamic_cast<IntegerConst*>(begTok)->GetValue();
                int end  = dynamic_cast<IntegerConst*>(endTok)->GetValue();
                int skip = !skipTok ? 1 : 
                           dynamic_cast<IntegerConst*>(skipTok)->GetValue();

                if (skip <= 0 || beg > end)
                {
                    EXCEPTION1(ExpressionException, "avtMIRvfFilter: "
                               "Range must be of the form beg:end[:skip].");
                }

                for (int m = beg; m <= end ; m += skip)
                    matIndices.push_back(m);
            }
            else
            {
                ExprNode *item = (*elems)[i]->GetItem();
                string type = item->GetTypeName();
                if (type != "Const")
                {
                    debug5 << "avtMIRvfFilter: List element is not a constant "
                              "or a list: " << type.c_str() << endl;
                    EXCEPTION1(ExpressionException, "avtMIRvfFilter: "
                               "List element is not a constant or a list.");
                }

                AddMaterial(dynamic_cast<ConstExpr*>(item));
            }
        }
    }
}

// ****************************************************************************
//  Method:  avtMIRvfFilter::AddMaterial
//
//  Purpose:
//    Add a material by name or index to the list
//
//  Arguments:
//    c          The expression to turn into a material
//
//  Notes:        This routine was directly taken from avtMatvfFilter.
//
//  Programmer:  Hank Childs
//  Creation:    December 31, 2004
//
// ****************************************************************************

void
avtMIRvfFilter::AddMaterial(ConstExpr *c)
{
    // Check that it's a string.
    Token *t = c->GetToken();
    if (t->GetType() != TT_StringConst && t->GetType() != TT_IntegerConst)
    {
        debug5 << "avtMIRvfFilter: Matfv argument is not a string or integer: "
               << GetTokenTypeString(t->GetType()).c_str() << endl;
        EXCEPTION1(ExpressionException, "avtMIRvfFilter: "
                   "Matfv argument is not a string or interger.");
    }

    if (t->GetType() == TT_StringConst)
    {
        string matname = dynamic_cast<StringConst*>(t)->GetValue();
        matNames.push_back(matname);
    }
    else // t->GetType() == TT_IntegerConst
    {
        int matindex = dynamic_cast<IntegerConst*>(t)->GetValue();
        matIndices.push_back(matindex);
    }
}


// ****************************************************************************
//  Method: avtMIRvfFilter::PerformRestriction
//
//  Purpose:
//      This routine allows the filter to change the data specification.
//      Tell the database that it must do MIR so that we can calculate the
//      volume fractions.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtMIRvfFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtSILRestriction_p silr = spec->GetDataSpecification()->GetRestriction();
    avtSILRestrictionTraverser trav(silr);
    if (!trav.UsesAllMaterials())
    {
        if (!issuedWarning)
        {
           std::string warningString = "This expression can only be calculated"
                    " if no material selection has been applied.  Results may"
                    " be misleading.  Please try again with all materials on.";
           avtCallback::IssueWarning(warningString.c_str());
           issuedWarning = true;
        }
    } 

    spec->GetDataSpecification()->ForceMaterialInterfaceReconstructionOn();
    return spec;
}


// ****************************************************************************
//  Method: avtMIRvfFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicates the zones no longer correspond to the original problem.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

void
avtMIRvfFilter::RefashionDataObjectInfo(void)
{
    avtSingleInputExpressionFilter::RefashionDataObjectInfo();
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


