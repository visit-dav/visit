// ************************************************************************* //
//                             avtMeshFilter.h                               //
// ************************************************************************* //

#ifndef AVT_MESH_FILTER_H
#define AVT_MESH_FILTER_H


#include <avtDataTreeStreamer.h>
#include <MeshAttributes.h>

class vtkExtractEdges;
class vtkGeometryFilter;
class vtkLinesFromOriginalCells;
class vtkDataSet;


// ****************************************************************************
//  Class: avtMeshFilter
//
//  Purpose:
//      A filter that extracts the mesh edges of an avtDataSet.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 8, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Added string argument to Execute method.
//
//    Kathleen Bonnell, Tue Mar 26 15:23:11 PST 2002 
//    Added PerformRestriction method.
//
//    Kathleen Bonnell, Thu Feb  5 10:34:16 PST 2004 
//    Added vtkExtractEdges, removed vtkUniqueFeatureEdges.
//
// ****************************************************************************

class avtMeshFilter : public avtDataTreeStreamer
{
  public:
                               avtMeshFilter(const MeshAttributes &);
    virtual                   ~avtMeshFilter();

    virtual const char        *GetType(void)  { return "avtMeshFilter"; };
    virtual const char        *GetDescription(void)  
                                   { return "Constructing mesh"; };
    virtual void               ReleaseData(void);

  protected:

    MeshAttributes             atts;

    vtkGeometryFilter         *geometryFilter;
    vtkExtractEdges           *extractEdges;
    vtkLinesFromOriginalCells *lineFilter;

    virtual avtDataTree_p      ExecuteDataTree(vtkDataSet *, int, string);
    virtual void               RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p     
                               PerformRestriction(avtPipelineSpecification_p spec);
};


#endif


