// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           ExprPipelineState                               //
// ************************************************************************* //

#ifndef EXPR_PIPELINE_STATE_H
#define EXPR_PIPELINE_STATE_H

#include <avtDataObject.h>
#include <expression_exports.h>

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
//    Hank Childs, Fri Dec 31 11:50:07 PST 2004
//    Add a Clear method.
//
//    Hank Childs, Fri Aug 25 17:26:59 PDT 2006
//    Add method GetNumNames.
//
// ****************************************************************************

class EXPRESSION_API ExprPipelineState
{
public:
                    ExprPipelineState();
                   ~ExprPipelineState();

    void            PushName(std::string s) {name_stack.push_back(s);} 
    std::string     PopName();
    int             GetNumNames(void) const { return static_cast<int>(name_stack.size()); };

    void            SetDataObject(avtDataObject_p d) {dataObject = d;}
    avtDataObject_p GetDataObject() {return dataObject;}
    void            AddFilter(avtExpressionFilter *f) {filters.push_back(f);}
    std::vector<avtExpressionFilter*>& GetFilters() {return filters;}

    void            ReleaseData(void);
    void            Clear();

protected:
    std::vector<std::string>    name_stack;
    avtDataObject_p             dataObject;
    std::vector<avtExpressionFilter*> filters;
};

#endif
