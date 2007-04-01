// ************************************************************************* //
//                         avtVertexNormalsFilter.h                          //
// ************************************************************************* //

#ifndef AVT_VERTEX_NORMALS_FILTER_H
#define AVT_VERTEX_NORMALS_FILTER_H

#include <filters_exports.h>

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
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class AVTFILTERS_API avtVertexNormalsFilter : public avtStreamer
{
  public:
                         avtVertexNormalsFilter();
    virtual             ~avtVertexNormalsFilter();

    virtual const char  *GetType(void) { return "avtVertexNormalsFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Calculating normals"; };

    void SetPointNormals(bool pn) { pointNormals = pn; }

  protected:
    bool pointNormals;
    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


