// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtPickByNodeQuery.h                              //
// ************************************************************************* //

#ifndef AVT_PICKBYNODE_QUERY_H
#define AVT_PICKBYNODE_QUERY_H
#include <query_exports.h>

#include <avtPickQuery.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtPickByNodeQuery
//
//  Purpose:
//    This query gathers information about a node picked by the user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDataAttributes arg to Preparation.
//    
// ****************************************************************************

class QUERY_API avtPickByNodeQuery : public avtPickQuery
{
  public:
                                    avtPickByNodeQuery();
    virtual                        ~avtPickByNodeQuery();


    virtual const char             *GetType(void)
                                             { return "avtPickByNodeQuery"; };

    virtual void                    SetTransform(const avtMatrix *m);


  protected:

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    Preparation(const avtDataAttributes &);
};


#endif


