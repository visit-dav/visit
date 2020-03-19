// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtSpreadsheetFilter.h                       //
// ************************************************************************* //

#ifndef AVT_Spreadsheet_FILTER_H
#define AVT_Spreadsheet_FILTER_H

#include <SpreadsheetAttributes.h>
#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtSpreadsheetFilter
//
//  Purpose:
//      This operator is the implied operator associated with a Spreadsheet plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
//  Modifications:
//    Brad Whitlock, Thu Dec 16 11:52:11 PST 2010
//    I added PostExecute.
//
// ****************************************************************************

class avtSpreadsheetFilter : public avtDatasetToDatasetFilter
{
public:
                              avtSpreadsheetFilter();
    virtual                  ~avtSpreadsheetFilter();

    void SetAtts(const SpreadsheetAttributes &);
    virtual const char       *GetType(void)   { return "avtSpreadsheetFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Performing Visual spreadsheet"; };

protected:
    SpreadsheetAttributes      atts;

    virtual avtContract_p     ModifyContract(avtContract_p);
    virtual void              Execute(void);
    virtual void              PostExecute(void);
};


#endif


