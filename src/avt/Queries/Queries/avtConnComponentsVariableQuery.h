// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtConnComponentsVariableQuery.h                     //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_VARIABLE_QUERY_H
#define AVT_CONN_COMPONENTS_VARIABLE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <string>
#include <vector>

class vtkDataSet;


// ****************************************************************************
//  Class: avtConnComponentsVariableQuery
//
//  Purpose:
//      Finds the sum of a specified variable for each connected component. 
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007 
//
// ****************************************************************************

class QUERY_API avtConnComponentsVariableQuery 
: public avtConnComponentsQuery
{
  public:
                                    avtConnComponentsVariableQuery();
    virtual                        ~avtConnComponentsVariableQuery();

    virtual const char             *GetType(void)
                                 { return "avtConnComponentsVariableQuery"; };
    virtual const char             *GetDescription(void)
                                 { return "Finding per component sum."; };

  protected:
    
    std::string                     variableName;
    std::vector<double>             sumPerComp;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual void                    VerifyInput(void);


};


#endif



