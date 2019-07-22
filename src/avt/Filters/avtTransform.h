// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtTransform.h                               //
// ************************************************************************* //

#ifndef AVT_TRANSFORM_H
#define AVT_TRANSFORM_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

class     vtkMatrix4x4;
class     vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtTransform
//
//  Purpose:
//      Does any 4x4 matrix transformation.  Derived types specify the matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Changed ExecuteDomain to ExecuteData.
//
//    Hank Childs, Wed Jun  6 09:12:58 PDT 2001
//    Removed CalcDomainList.
//
//    Jeremy Meredith, Mon Sep 24 14:21:13 PDT 2001
//    Added UpdateDataObjectInfo.  Made inheritance virtual.
//
//    Hank Childs, Mon Jan 14 09:01:18 PST 2002
//    Added data member for transform filter to fix memory leak.
//
//    Hank Childs, Mon Aug  5 10:29:22 PDT 2002
//    Added individual routines to transform rectilinear grids based on what
//    type of transformation is done.
//
//    Hank Childs, Tue Sep 10 15:21:28 PDT 2002
//    Perform all memory management at the domain (ExecuteData) level.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Fri Mar 28 14:33:55 PDT 2008 
//    Added TransformData method.
//
//    Hank Childs, Tue Sep 22 20:26:35 PDT 2009
//    Add a method for transforming vectors ... we need to not transform
//    vectors when volume rendering.
//
//    Dave Pugmire, Fri May 14 08:04:43 EDT 2010
//    Flag for vector transformations.
//
//    Eric Brugger, Tue Jul 22 12:07:11 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtTransform : public virtual avtDataTreeIterator
{
  public:
                                avtTransform();
    virtual                    ~avtTransform();

    virtual const char         *GetType(void) { return "avtTransform"; };
    virtual const char         *GetDescription(void)
                                        { return "Transforming data"; };
    void                        SetVectorTransform(bool b)
                                               { transformVectors = b; }

  protected:
    bool transformVectors;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual vtkMatrix4x4       *GetTransform() = 0;
    virtual void                UpdateDataObjectInfo(void);

    vtkDataSet                 *TransformRectilinear(vtkRectilinearGrid *);
    bool                        OutputIsRectilinear(vtkMatrix4x4 *);
    vtkDataSet                 *TransformRectilinearToRectilinear(
                                                         vtkRectilinearGrid *);
    vtkDataSet                 *TransformRectilinearToCurvilinear(
                                                         vtkRectilinearGrid *);
    virtual void                TransformData(vtkRectilinearGrid *) {;};
};


#endif


