// ************************************************************************* //
//                         avtVertexNormalsFilter.h                          //
// ************************************************************************* //

#ifndef AVT_VERTEX_NORMALS_FILTER_H
#define AVT_VERTEX_NORMALS_FILTER_H

#include <pipeline_exports.h>

#include <avtStreamer.h>


// ****************************************************************************
//  Class: avtVertexNormalsFilter
//
//  Purpose:
//      Calculates vertex normals for a dataset. 
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2001
//
//  Modifications:
//    Hank Childs, Tue Sep 10 15:17:21 PDT 2002
//    Removed data members to conform with new memory management paradigm.
//
//    Jeremy Meredith, Tue Aug 12 10:55:35 PDT 2003
//    Added ability to have cell normals.
//
// ****************************************************************************

class PIPELINE_API avtVertexNormalsFilter : public avtStreamer
{
  public:
                         avtVertexNormalsFilter() { pointNormals=false;};
    virtual             ~avtVertexNormalsFilter() {;};

    virtual const char  *GetType(void) { return "avtVertexNormalsFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Calculating normals"; };

    void SetPointNormals(bool pn) { pointNormals = pn; }

  protected:
    bool pointNormals;
    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


