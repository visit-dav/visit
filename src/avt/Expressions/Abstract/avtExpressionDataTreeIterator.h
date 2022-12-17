// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtExpressionDataTreeIterator.h                       //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_DATA_TREE_ITERATOR_H
#define AVT_EXPRESSION_DATA_TREE_ITERATOR_H

#include <vtkBitArray.h>

#include <expression_exports.h>

#include <avtExpressionFilter.h>
#include <avtDataTreeIterator.h>

#include <string>

class avtVtkmDataSet;

// ****************************************************************************
//  Class: avtExpressionDataTreeIterator
//
//  Purpose:
//      This is an abstract type that allows derived types to create 
//      expressions one VTK dataset at a time.
//
//  Notes:      The streaming functionality used to be part of 
//              avtExpressionFilter.  The creation date corresponds to when the
//              class was split.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
//  Modifications:
//
//    David Camp, Tue May 21 13:56:12 PDT 2013
//    Remove these variables because the worker threads will call
//    the ExecuteData function and they all can not share these variables.
//    I will change the code to pass the currentDomainsIndex into the
//    DeriveVariable function. I have found that no one uses the 
//    currentDomainsLabel variable, so I am not going to pass it.
//     std::string              currentDomainsLabel;
//     int                      currentDomainsIndex;
//
//    Eric Brugger, Wed Aug 20 16:24:51 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eddie Rusu, Tue Jan 14 13:56:24 PST 2020
//    Added volumeDependent array that tracks if a generated expression is
//    volume-dependent.
//
// ****************************************************************************

class EXPRESSION_API avtExpressionDataTreeIterator 
                                    : virtual public avtDataTreeIterator, 
                                      virtual public avtExpressionFilter
{
  public:
                             avtExpressionDataTreeIterator();
    virtual                 ~avtExpressionDataTreeIterator();

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);

    avtDataRepresentation *        ExecuteData_VTK(avtDataRepresentation *in_dr);
    virtual vtkDataArray          *DeriveVariable(vtkDataSet *, int currentDomainsIndex) = 0;

    avtDataRepresentation *        ExecuteData_VTKm(avtDataRepresentation *in_dr);
    virtual void                   DeriveVariableVTKm(avtVtkmDataSet *, 
                                                      int currentDomainsIndex,
                                                      const std::string &activeVar,
                                                      const std::string &outputVar);

    // Track if the created array is volume-dependent.
    vtkBitArray              *volumeDependent;
};

#endif

