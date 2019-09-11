// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtCreateBondsFilter.h
// ************************************************************************* //

#ifndef AVT_CreateBonds_FILTER_H
#define AVT_CreateBonds_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <CreateBondsAttributes.h>

class vtkPoints;
class vtkDataSet;
class vtkPolyData;


// ****************************************************************************
//  Class: avtCreateBondsFilter
//
//  Purpose:
//      A plugin operator for CreateBonds.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:39:51 EST 2008
//    The manual bonding matches now supports wildcards, alleviating the need
//    for a "simple" mode.  (The default for the manual mode is actually
//    exactly what the simple mode was going to be anyway.)
//
//    Jeremy Meredith, Tue Jan 26 16:30:40 EST 2010
//    Separated bond distance check to a more manual version.  This allows
//    for some optimizations, particularly in the new "periodic bond check"
//    feature.
//
//    Jeremy Meredith, Wed Jan 27 16:47:10 EST 2010
//    Added a new, fast bond creation routine.  Kept the old one around
//    as a fallback in case bonding distance paramemters make the fast
//    routine a poor choice, or in case we need to add some feature which
//    be too hard to add in the more complex fast routine.
//
//    Eric Brugger, Wed Jul 23 11:24:54 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtCreateBondsFilter : public avtPluginDataTreeIterator
{
  public:
                         avtCreateBondsFilter();
    virtual             ~avtCreateBondsFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtCreateBondsFilter"; };
    virtual const char  *GetDescription(void)
                             { return "CreateBonds"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    CreateBondsAttributes   atts;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    vtkDataSet           *ExecuteData_Fast(vtkPolyData*, float maxBondDist,
                                          float,float,float,float,float,float);
    vtkDataSet           *ExecuteData_Slow(vtkPolyData*);
    virtual avtContract_p
                          ModifyContract(avtContract_p spec);
    virtual void          UpdateDataObjectInfo(void);

    bool AtomBondDistances(int eA, int eB, double &dmin, double &dmax);
};


#endif
