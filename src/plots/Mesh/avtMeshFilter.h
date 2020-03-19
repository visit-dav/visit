// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMeshFilter.h                               //
// ************************************************************************* //

#ifndef AVT_MESH_FILTER_H
#define AVT_MESH_FILTER_H

#include <avtSIMODataTreeIterator.h>

#include <MeshAttributes.h>

#include <string>

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
//    Added ModifyContract method.
//
//    Kathleen Bonnell, Thu Feb  5 10:34:16 PST 2004 
//    Added vtkExtractEdges, removed vtkUniqueFeatureEdges.
//
//    Kathleen Bonnell, Tue Nov  2 10:41:33 PST 2004 
//    Added keepNodeZone. 
//
//    Hank Childs, Thu Mar 10 09:13:03 PST 2005
//    Removed data member filters to simplify memory management.  Also removed
//    ReleaseData.
//
//    Eric Brugger, Tue Aug 19 10:55:03 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtMeshFilter : public avtSIMODataTreeIterator
{
  public:
                               avtMeshFilter(const MeshAttributes &);
    virtual                   ~avtMeshFilter();

    virtual const char        *GetType(void)  { return "avtMeshFilter"; };
    virtual const char        *GetDescription(void)  
                                   { return "Constructing mesh"; };

  protected:
    MeshAttributes             atts;
    bool                       keepNodeZone;

    virtual avtDataTree_p      ExecuteDataTree(avtDataRepresentation *);
    virtual void               UpdateDataObjectInfo(void);
    virtual void               PostExecute(void);
    virtual avtContract_p      ModifyContract(avtContract_p spec);
};


#endif


