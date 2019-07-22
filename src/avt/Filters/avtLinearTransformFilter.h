// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtLinearTransformFilter.h
// ************************************************************************* //

#ifndef AVT_LinearTransform_FILTER_H
#define AVT_LinearTransform_FILTER_H
#include <filters_exports.h>

#include <avtPluginDataTreeIterator.h>
#include <avtTransform.h>
#include <LinearTransformAttributes.h>
#include <vtkMatrix4x4.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtLinearTransformFilter
//
//  Purpose:
//      A filter that performs transformations where the matrix is a 
//      3x3 matrix.  This allows for rotations, scaling, and shearing.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

class AVTFILTERS_API avtLinearTransformFilter : public avtTransform
{
  public:
                         avtLinearTransformFilter();
    virtual             ~avtLinearTransformFilter();

    virtual const char  *GetType(void)  
                             { return "avtLinearTransformFilter"; };
    virtual const char  *GetDescription(void) { return "Transforming"; };

    virtual void         SetAtts(const AttributeGroup*);

  protected:
    LinearTransformAttributes   atts;
    vtkMatrix4x4         *M;
    vtkMatrix4x4         *invM;

    void                  SetupMatrix();
    virtual vtkMatrix4x4 *GetTransform() { SetupMatrix(); return M; };
    virtual avtContract_p
                          ModifyContract(avtContract_p);
    virtual void          PostExecute(void);
};


#endif
