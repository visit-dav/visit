#ifndef EXPR_PIPELINE_STATE_H
#define EXPR_PIPELINE_STATE_H

#include <avtDataObject.h>

class avtExpressionFilter;

class ExprPipelineState
{
public:
                    ExprPipelineState() {dataObject=NULL;}
                   ~ExprPipelineState() {}
    void            PushName(std::string s) {name_stack.push_back(s);}
    std::string     PopName()
        {std::string ret = name_stack.back();
         name_stack.pop_back();
         return ret;}
    void            SetDataObject(avtDataObject_p d) {dataObject = d;}
    avtDataObject_p GetDataObject() {return dataObject;}
    void            AddFilter(avtExpressionFilter *f) {filters.push_back(f);}
    std::vector<avtExpressionFilter*>& GetFilters() {return filters;}

protected:
    std::vector<std::string>    name_stack;
    avtDataObject_p             dataObject;
    std::vector<avtExpressionFilter*> filters;
};

#endif
