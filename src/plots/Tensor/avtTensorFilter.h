// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtTensorFilter.h                             //
// ************************************************************************* //

#ifndef AVT_TENSOR_FILTER_H
#define AVT_TENSOR_FILTER_H

#include <avtDataTreeIterator.h>

class vtkTensorReduceFilter;
class vtkVertexFilter;


// ****************************************************************************
//  Class: avtTensorFilter
//
//  Purpose:
//      A filter that takes in tensor data and creates tensor glyphs as poly
//      data.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005
//    Added keepNodeZone & ModifyContract.
//
//    Eric Brugger, Tue Aug 19 11:28:07 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtTensorFilter : public avtDataTreeIterator
{
  public:
                              avtTensorFilter(bool, int);
    virtual                  ~avtTensorFilter();

    virtual const char       *GetType(void)   { return "avtTensorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating tensors"; };

    void                      SetStride(int);
    void                      SetNTensors(int);
    void                      SetLimitToOriginal(bool);

  protected:
    bool                      useStride;
    int                       stride;
    int                       nTensors;
    bool                      origOnly;

    bool                      keepNodeZone;
    int                       approxDomains;
  
    virtual void              PreExecute(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p     ModifyContract(avtContract_p);
};

#endif
