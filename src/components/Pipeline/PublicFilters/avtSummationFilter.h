// ************************************************************************* //
//                            avtSummationFilter.h                           //
// ************************************************************************* //

#ifndef AVT_SUMMATION_FILTER_H
#define AVT_SUMMATION_FILTER_H


#include <avtStreamer.h>

#include <string>


// ****************************************************************************
//  Class: avtSummationFilter
//
//  Purpose:
//      This should really be a query, not a filter.  It sums all of the values
//      for a variable.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2002
//
// ****************************************************************************

class PIPELINE_API avtSummationFilter : public avtStreamer
{
  public:
                                    avtSummationFilter();
    virtual                        ~avtSummationFilter() {;};

    void                            SetVariableName(std::string &);
    void                            SetSumType(std::string &);

    virtual const char             *GetType(void)
                                             { return "avtSummationFilter"; };
    virtual const char             *GetDescription(void)
                                             { return descriptionBuffer; };

    void                            PassData(bool);
    void                            SumGhostValues(bool);
    void                            IssueWarning(bool);

  protected:
    double                          sum;
    std::string                     variableName;
    std::string                     sumType;
    bool                            passData;
    bool                            sumGhostValues;
    bool                            issueWarning;
    char                            descriptionBuffer[1024];

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);

    virtual vtkDataSet             *ExecuteData(vtkDataSet *, int,std::string);
};


#endif


