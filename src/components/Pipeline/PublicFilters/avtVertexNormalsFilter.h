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
//
//    Hank Childs, Tue Sep 10 15:17:21 PDT 2002
//    Removed data members to conform with new memory management paradigm.
//
// ****************************************************************************

class PIPELINE_API avtVertexNormalsFilter : public avtStreamer
{
  public:
                         avtVertexNormalsFilter() {;};
    virtual             ~avtVertexNormalsFilter() {;};

    virtual const char  *GetType(void) { return "avtVertexNormalsFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Calculating normals"; };

  protected:
    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


