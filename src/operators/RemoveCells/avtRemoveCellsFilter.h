// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtRemoveCellsFilter.h
// ************************************************************************* //

#ifndef AVT_RemoveCells_FILTER_H
#define AVT_RemoveCells_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <RemoveCellsAttributes.h>

#include <vector>


// ****************************************************************************
//  Class: avtRemoveCellsFilter
//
//  Purpose:
//      A plugin operator for RemoveCells.
//
//  Programmer: haddox1 -- generated by xml2info
//  Creation:   Mon Jun 2 13:48:29 PST 2003
//
//  Modifications:
//    Eric Brugger, Thu Jul 31 19:27:19 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtRemoveCellsFilter : public avtPluginDataTreeIterator
{
  public:
                         avtRemoveCellsFilter();
    virtual             ~avtRemoveCellsFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtRemoveCellsFilter"; };
    virtual const char  *GetDescription(void)
                             { return "RemoveCells"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    RemoveCellsAttributes   atts;
    
    void ParseAtts(int domain, std::vector<int> &cells);
    
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);

    virtual void          UpdateDataObjectInfo(void);
};


#endif
