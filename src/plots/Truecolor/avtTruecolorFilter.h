// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtTruecolorFilter.h                           //
// ************************************************************************* //

#ifndef AVT_Truecolor_FILTER_H
#define AVT_Truecolor_FILTER_H

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtTruecolorFilter
//
//  Purpose:
//      This operator is the implied operator associated with an Truecolor plot.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday June 15, 2004
//
//  Modifications:
//
//    Chris Wojtan Mon Jun 21 15:45 PDT 2004
//    Added "variable_name" member variable
//    Added SetVarName member function
//
//    Hank Childs, Fri May 20 14:55:06 PDT 2005
//    Remove UpdateDataObjectInfo (it was empty).
//
//    Eric Brugger, Tue Aug 19 11:46:04 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtTruecolorFilter : public avtDataTreeIterator
{
  public:
                              avtTruecolorFilter();
    virtual                  ~avtTruecolorFilter();

    virtual const char       *GetType(void)   { return "avtTruecolorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Performing Truecolor"; };
    void                      SetVarName(const char*name)
                                  {variable_name = name;}

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    const char               *variable_name;
};


#endif


