// ************************************************************************* //
//                              avtMatvfFilter.C                            //
// ************************************************************************* //

#include <Token.h>
#include <EngineExprNode.h>
#include <avtMatvfFilter.h>
#include <DebugStream.h>
#include <ExpressionException.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>


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
// ****************************************************************************

vtkDataArray *
avtMatvfFilter::DeriveVariable(vtkDataSet *in_ds)
{
    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells   = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        // Set the tuple to the value of the material fraction at that cell.
        //vtkCell *cell = in_ds->GetCell(i);
        //float matfv = (float)GetMatvf(cell);
        //arr->SetTuple1(i, matfv);
        arr->SetTuple1(i, 1337);
    }

    cerr << "avtMatvfFilter: materials" << endl;
    for(int i=0;i<materials.size();i++)
    {
        cerr << "    " << materials[i] << endl;
    }

    return arr;
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
