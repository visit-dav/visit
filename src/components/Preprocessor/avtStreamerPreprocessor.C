// ************************************************************************* //
//                           avtStreamerPreprocessor.C                       //
// ************************************************************************* //

#include <avtStreamerPreprocessor.h>

#include <vtkDataSet.h>

#include <avtDataTree.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtStreamerPreprocessor constructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtStreamerPreprocessor::avtStreamerPreprocessor()
{
    ;
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor destructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtStreamerPreprocessor::~avtStreamerPreprocessor()
{
    ;
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::Preprocess
//
//  Purpose:
//      Does the actual preprocessing work.  Since most derived types do not
//      want to code up how to unwrap a data tree, this does it for them.  It
//      then calls ProcessDomain for each data tree, which the derived types
//      must define.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtStreamerPreprocessor::Preprocess(void)
{
    avtDataTree_p tree = GetInputDataTree();
    int totalNodes = tree->GetNumberOfLeaves();

    debug3 << "Preprocessing with " << totalNodes << " nodes." << endl;
    Initialize(totalNodes);

    debug3 << "Preprocessing the top level tree" << endl;
    PreprocessTree(tree);

    debug3 << "Allowing preprocessing module to finalize." << endl;
    Finalize();
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::PreprocessTree
//
//  Purpose:
//      Unwraps a data tree and calls ProcessDomain on each leaf. 
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtStreamerPreprocessor::PreprocessTree(avtDataTree_p tree)
{
    if (*tree == NULL)
    {
        return;
    }

    int numChildren = tree->GetNChildren();

    if ( (numChildren <= 0) && (!(tree->HasData())) )
    {
        return;
    }

    if (numChildren == 0)
    {
        //
        // There is only one dataset to process (the leaf).
        //
        vtkDataSet *in_ds = tree->GetDataRepresentation().GetDataVTK();
        int dom = tree->GetDataRepresentation().GetDomain();

        //
        // Ensure that there is no funny business when we do an Update.
        //
        in_ds->SetSource(NULL);

        ProcessDomain(in_ds, dom);
    }
    else
    {
        for (int i = 0 ; i < numChildren ; i++)
        {
            if (tree->ChildIsPresent(i))
            {
                PreprocessTree(tree->GetChild(i));
            }
        }
    }
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::Initialize
//
//  Purpose:
//      Gives the preprocessor module a chance to initialize itself.  This
//      implementation does nothing, but is defined so derived types can
//      initalize themselves if appropriate.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtStreamerPreprocessor::Initialize(int)
{
    ;
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::Finalize
//
//  Purpose:
//      Gives the preprocessor module a chance to finalize itself.  This
//      implementation does nothing, but is defined so derived types can
//      finalize themselves if appropriate.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtStreamerPreprocessor::Finalize(void)
{
    ;
}


