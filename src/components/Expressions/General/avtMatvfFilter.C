// ************************************************************************* //
//                               avtMatvfFilter.C                            //
// ************************************************************************* //

#include <avtMatvfFilter.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedIntArray.h>

#include <Token.h>
#include <EngineExprNode.h>

#include <avtMaterial.h>
#include <avtMetaData.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtMatvfFilter::DeriveVariable
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
// ****************************************************************************

vtkDataArray *
avtMatvfFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i, j;

    int ncells = in_ds->GetNumberOfCells();

    //
    // The 'currentDomainsIndex' is a data member of the base class that is
    // set to be the id of the current domain right before DeriveVariable is
    // called.  We need that index to make sure we are getting the right mat.
    //
    avtMaterial *mat = GetMetaData()->GetMaterial(currentDomainsIndex);
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
    vf_for_orig_cells = vtkFloatArray::New();
    int norigcells = mat->GetNZones();
    vf_for_orig_cells->SetNumberOfTuples(norigcells);

    //
    // Try to match up the materials in the avtMaterial object with the
    // materials requested by the users.  Some of the materials requested
    // by the user may not be there.  This is probably okay -- there are
    // tricks played by the avtMaterial object where it throws out
    // materials that are not used in that domain.  
    //
    int nmats = mat->GetNMaterials();
    std::vector<bool>  useMat(nmats, false);
    for (i = 0 ; i < nmats ; i++)
    {
        std::string currentMat = mat->GetMaterials()[i];
        for (j = 0 ; j < materials.size() ; j++)
        {
            if (currentMat == materials[j])
                useMat[i] = true;
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
            int start = -matlist[i]-1;
            for (j = start ; j < mix_next[start] ; j++)
            {
                if (useMat[mixmat[j]])
                    vf += mixvf[j];
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
//  Method: avtMatvfFilter::ProcessArguments
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
// ****************************************************************************
void
avtMatvfFilter::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION1(ExpressionException, "avtMatvfFilter: No arguments given.");
    }
    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    EngineExprNode *firstTree = dynamic_cast<EngineExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        debug5 << "avtMatvfFilter: No second argument." << endl;
        return;
    }

    // See if there are other arguments.
    if (nargs > 2)
    {
        EXCEPTION1(ExpressionException, "avtMatvfFilter: Too many arguments.");
    }

    // Pull off the second argument and see if it's a string or a list.
    ArgExpr *secondarg = (*arguments)[1];
    ExprNode *secondTree = secondarg->GetExpr();
    string type = secondTree->GetTypeName();
    if ((type != "Const") && (type != "List"))
    {
        debug5 << "avtMatvfFilter: Second argument is not a string or a list: " << type << endl;
        EXCEPTION1(ExpressionException, "avtMatvfFilter: Second argument is not a string or a list.");
    }

    if (type == "Const")
    {
        // It's a single constant.
        AddMaterial(dynamic_cast<ConstExpr*>(secondTree));
    } else
    {
        // It's a list.  Process all of them.
        ListExpr *list = dynamic_cast<ListExpr*>(secondTree);
        std::vector<ListElemExpr*> *elems = list->GetElems();
        for(int i=0;i<elems->size();i++)
        {
            ExprNode *item = (*elems)[i]->GetItem();
            string type = item->GetTypeName();
            if (type != "Const")
            {
                debug5 << "avtMatvfFilter: List element is not a string or a list: " << type << endl;
                EXCEPTION1(ExpressionException, "avtMatvfFilter: List element is not a string or a list.");
            }

            AddMaterial(dynamic_cast<ConstExpr*>(item));
        }
    }
}

void
avtMatvfFilter::AddMaterial(ConstExpr *c)
{
    // Check that it's a string.
    Token *t = c->GetToken();
    if (t->GetType() != TT_StringConst)
    {
        debug5 << "avtMatvfFilter: Matfv argument is not a string: "
               << GetTokenTypeString(t->GetType()) << endl;
        EXCEPTION1(ExpressionException, "avtMatvfFilter: Matfv argument is not a string.");
    }

    string matname = dynamic_cast<StringConst*>(t)->GetValue();
    materials.push_back(matname);
}
