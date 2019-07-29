// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtNamedSelectionFilter.h
// ************************************************************************* //

#ifndef AVT_NamedSelection_FILTER_H
#define AVT_NamedSelection_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

#include <vtkDataSet.h>


// ****************************************************************************
//  Class: avtNamedSelectionFilter
//
//  Purpose:
//      A filter that removes all identifiers that aren't part of the named
//      selection.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 21:27:00 PST 2009
//    Added data member selectionId.
//
//    Brad Whitlock, Thu Oct 27 16:26:30 PDT 2011
//    I added SelectedData.
//
//    Eric Brugger, Mon Jul 21 16:41:47 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtNamedSelectionFilter : public avtDataTreeIterator
{
  public:
                         avtNamedSelectionFilter();
    virtual             ~avtNamedSelectionFilter();

    virtual const char  *GetType(void)  { return "avtNamedSelectionFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Applying named selection"; };

    void                 SetSelectionName(const std::string &s)
                                { selName = s; };

  protected:
    std::string           selName;
    int                   selectionId;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual avtContract_p ModifyContract(avtContract_p);
    virtual void          UpdateDataObjectInfo(void);

    vtkDataSet           *SelectedData(vtkDataSet *in_ds, 
                                       const std::vector<vtkIdType> &ids);
};


#endif
