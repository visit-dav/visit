// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtTotalLengthQuery.h                              //
// ************************************************************************* //

#ifndef AVT_TOTAL_LENGTH_QUERY_H
#define AVT_TOTAL_LENGTH_QUERY_H

#include <query_exports.h>

#include <avtSummationQuery.h>


class     avtEdgeLength;


// ****************************************************************************
//  Class: avtTotalLengthQuery
//
//  Purpose:
//      A query for total length.
//
//  Programmer: Hank Childs
//  Creation:   November 3, 2006
//
// ****************************************************************************

class QUERY_API avtTotalLengthQuery : public avtSummationQuery
{
  public:
                         avtTotalLengthQuery();
    virtual             ~avtTotalLengthQuery();

    virtual const char  *GetType(void)  { return "avtTotalLengthQuery"; };
    virtual const char  *GetDescription(void)
                             { return "Total length"; };

  protected:
    virtual void               VerifyInput(void); 
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    avtEdgeLength             *length_expr;
};


#endif


