// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtVertexExtractor.h
// ****************************************************************************

#ifndef AVT_VERTEX_EXTRACTOR_H
#define AVT_VERTEX_EXTRACTOR_H

#include <avtSIMODataTreeIterator.h>

#include <GlyphTypes.h>
#include <filters_exports.h>

class vtkPolyData;

// ****************************************************************************
//  Class: avtVertexExtractor
//
//  Purpose:  Extracts vertex cells.
//            If ConvertAllPoints is set, will convert all points into vertex
//            cells, and no other output is generated.
//            If KeepNonVertex is set, then non-vertex cells are returned in
//            separate dataset from vertex cells.
//            
//            Labels will be applied to output datasets so that
//            avtCompactTreeFilter won't recombine the outputs.
//
//            Labels also serve to provide a clue for a plot as to how to set
//            up its mapper(s).
//
//  Notes:    Pulled out from (now defunct) avtPseudocolorGeometryFilter.
//             
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 202
//
//  Modifications:
//
// ****************************************************************************

class AVTFILTERS_API avtVertexExtractor : public avtSIMODataTreeIterator
{
  public:
                            avtVertexExtractor();
    virtual                ~avtVertexExtractor();

    virtual const char     *GetType(void)
                              { return "avtVertexExtractor"; }
    virtual const char     *GetDescription(void)
                              { return "Applying Vertex Extractor"; }
    void                    SetPointType(const GlyphType);
    void                    SetKeepNonVertex(const bool);
    void                    SetConvertAllPoints(const bool);
    void                    SetLabelPrefix(const std::string &);

  protected:

    virtual avtDataTree_p   ExecuteDataTree(avtDataRepresentation *);
    virtual void            UpdateDataObjectInfo(void);
    virtual void            PostExecute(void);

  private:
    GlyphType               pointType;
    bool                    keepNonVertex;
    bool                    convertAllPoints;
    std::string             labelPrefix;

    void                    ProcessPoints(const vtkDataSet *, vtkDataSet *&,
                                          bool &, bool &);
};


#endif


