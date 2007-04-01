// ************************************************************************* //
//                         ExprPipelineState.C                               //
// ************************************************************************* //

#include <ExprPipelineState.h>
#include <vector>
#include <avtExpressionFilter.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Method: ExprPipelineState constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
//  Modifications:
//
// ****************************************************************************

ExprPipelineState::ExprPipelineState() 
{
    dataObject=NULL;
}


// ****************************************************************************
//  Method: ExprPipelineState destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Sean Ahearn 
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 22 14:55:09 PDT 2004
//    Delete the filters.
//
// ****************************************************************************

ExprPipelineState::~ExprPipelineState() 
{
    for (int i = 0; i < filters.size(); ++i)
    {
        delete filters[i];
    }
}


// ****************************************************************************
//  Method: ExprPipelineState::PopName
//
//  Purpose:
//    Removes a name from the back of the name stack and returns it. 
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
//  Modifications:
//
// ****************************************************************************

string     
ExprPipelineState::PopName()
{
    string ret = name_stack.back();
    name_stack.pop_back();
    return ret;
}


// ****************************************************************************
//  Method: ExprPipelineState::ReleaseData
//
//  Purpose:
//    Releases the data associated the expression filters.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 22, 2004 
//
//  Modifications:
//
// ****************************************************************************

void 
ExprPipelineState::ReleaseData()
{
    for (int i = 0; i < filters.size(); ++i)
        filters[i]->ReleaseData();
}


// ****************************************************************************
//  Method: ExprPipelineState::Clear
//
//  Purpose:
//      Clears out the pipeline state.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

void
ExprPipelineState::Clear()
{
    for (int i = 0; i < filters.size(); ++i)
        delete filters[i];
    filters.clear();
    name_stack.clear();
    dataObject = NULL;
}


