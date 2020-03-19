// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtSimilarityTransformFilter.h
// ************************************************************************* //

#ifndef AVT_SimilarityTransform_FILTER_H
#define AVT_SimilarityTransform_FILTER_H
#include <filters_exports.h>

#include <avtPluginDataTreeIterator.h>
#include <avtTransform.h>
#include <SimilarityTransformAttributes.h>
#include <vtkMatrix4x4.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtSimilarityTransformFilter
//
//  Purpose:
//      A filter that performs transformations where the matrix is a 
//      similarity matrix.  This allows for rotations, translation, and
//      scaling.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 24, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Nov 28 16:59:53 PST 2001
//    Added UpdateDataObjectInfo, ModifyContract.
//
//    Hank Childs, Tue Mar  5 16:05:25 PST 2002
//    Removed UpdateDataObjectInfo, since what is was doing is also
//    applicable to the base class, so I pushed the logic into it.
//
//    Kathleen Bonnell, Thu Apr 10 11:07:48 PDT 2003 
//    Store inverse matrix for possible use later in the pipeline. 
//    Added PostExecute method.
//    
//    Hank Childs, Mon Jun 30 15:03:32 PDT 2003
//    Made this filter no longer be a streamer.  Renamed with Similarity.
//    Also moved to components directory.
//
//    Kathleen Bonnell, Fri Mar 28 14:33:55 PDT 2008 
//    Added TransformData method.
//
// ****************************************************************************

class AVTFILTERS_API avtSimilarityTransformFilter : public avtTransform
{
  public:
                         avtSimilarityTransformFilter();
    virtual             ~avtSimilarityTransformFilter();

    virtual const char  *GetType(void)  
                             { return "avtSimilarityTransformFilter"; };
    virtual const char  *GetDescription(void) { return "Transforming"; };

    virtual void         SetAtts(const AttributeGroup*);

  protected:
    SimilarityTransformAttributes   atts;
    vtkMatrix4x4         *M;
    vtkMatrix4x4         *invM;

    void                  SetupMatrix();
    virtual vtkMatrix4x4 *GetTransform() { SetupMatrix(); return M; };
    virtual avtContract_p
                          ModifyContract(avtContract_p);
    virtual void          UpdateDataObjectInfo(void);
    virtual void          PostExecute(void);
    virtual void          TransformData(vtkRectilinearGrid *);
};


#endif
