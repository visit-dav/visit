// ************************************************************************* //
//                           ExprPipelineState                               //
// ************************************************************************* //

#ifndef EXPR_PIPELINE_STATE_H
#define EXPR_PIPELINE_STATE_H

#include <avtDataObject.h>

class avtExpressionFilter;

// ****************************************************************************
//   Class: ExprPipelineState
//
//   Purpose:
//     Holds information about the pipeline state for expressions.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 22 14:42:38 PDT 2004
//    Moved code to new Source file.  Added ReleaseData method.
//
// ****************************************************************************

class ExprPipelineState
{
public:
                    ExprPipelineState();
                   ~ExprPipelineState();

    void            PushName(std::string s) {name_stack.push_back(s);} 
    std::string     PopName();

    void            SetDataObject(avtDataObject_p d) {dataObject = d;}
    avtDataObject_p GetDataObject() {return dataObject;}
    void            AddFilter(avtExpressionFilter *f) {filters.push_back(f);}
    std::vector<avtExpressionFilter*>& GetFilters() {return filters;}

    void            ReleaseData(void);

protected:
    std::vector<std::string>    name_stack;
    avtDataObject_p             dataObject;
    std::vector<avtExpressionFilter*> filters;
};

#endif
