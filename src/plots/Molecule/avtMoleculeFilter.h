// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMoleculeFilter.h                           //
// ************************************************************************* //

#ifndef AVT_MOLECULE_FILTER_H
#define AVT_MOLECULE_FILTER_H

#include <avtDataTreeIterator.h>

#include <MoleculeAttributes.h>

#include <set>
#include <string>
#include <vector>

class  vtkMoleculeReduceFilter;


// ****************************************************************************
//  Class: avtMoleculeFilter
//
//  Purpose:
//      Do some work to make the molecule plot happy.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 14, 2006
//
//  Modifications:
//    Eric Brugger, Tue Aug 19 10:59:44 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtMoleculeFilter : public avtDataTreeIterator
{
  public:
                              avtMoleculeFilter();
    virtual                  ~avtMoleculeFilter();

    virtual const char       *GetType(void)   { return "avtMoleculeFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating molecules"; };

    virtual void              ReleaseData(void);
    void                      SetAtts(const MoleculeAttributes*);

  protected:
    MoleculeAttributes        atts;

    std::string               name;
    std::set<int>             used_values;


    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p
                              ModifyContract(avtContract_p);
};


#endif


