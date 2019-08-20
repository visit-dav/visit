// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtStaggeringFilter.h                           //
// ************************************************************************* //

#ifndef AVT_STAGGERING_FILTER_H
#define AVT_STAGGERING_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

#include <string>


// ****************************************************************************
//  Class: avtStaggeringFilter
//
//  Purpose:  Translates plot positions as dictated by node offset values
//
//  Programmer: Marc Durant
//  Creation:   April 24, 2012 
//
//  Modifications:
//      Eric Brugger, Tue Jul 22 08:56:12 PDT 2014
//      Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtStaggeringFilter : public avtDataTreeIterator
{
  public:
                              avtStaggeringFilter();
    virtual                  ~avtStaggeringFilter();

    virtual const char       *GetType(void)   { return "avtStaggeringFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Preparing pipeline"; };

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p
                              ModifyContract(avtContract_p);
    virtual void              PostExecute(void);
    virtual bool              FilterUnderstandsTransformedRectMesh();

};

#endif


