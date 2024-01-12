// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtLineGlyphFilter.h
// ****************************************************************************

#ifndef AVT_LINE_GLYPH_FILTER_H
#define AVT_LINE_GLYPH_FILTER_H

#include <avtSIMODataTreeIterator.h>

#include <LineGlyphAttributes.h>
#include <filters_exports.h>

class vtkPolyData;

// ****************************************************************************
//  Class: avtLineGlyphFilter
//
//  Purpose:  Extracts line cells, applies tubes/ribbon to lines and/or glyphs
//            to line endpoints based on the LineGlyphAttributes settings.
//
//            If KeepNonLine is set then non-line cells are returned in
//            separate dataset from line cells.
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
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

class AVTFILTERS_API avtLineGlyphFilter : public avtSIMODataTreeIterator
{
  public:
                            avtLineGlyphFilter();
    virtual                ~avtLineGlyphFilter();

    virtual const char     *GetType(void)
                              { return "avtLineGlyphFilter"; }
    virtual const char     *GetDescription(void)
                              { return "Applying Line Geo filter"; }
    void                    SetLineGlyphAtts(const LineGlyphAttributes *);
    void                    SetKeepNonLine(const bool);
    void                    SetLabelPrefix(const std::string &);

  protected:

    virtual avtDataTree_p   ExecuteDataTree(avtDataRepresentation *);
    virtual void            UpdateDataObjectInfo(void);
    virtual void            PostExecute(void);

  private:
    LineGlyphAttributes     lineGlyphAtts;
    bool                    keepNonLine;
    std::string             labelPrefix;

    void                    ProcessLines(const vtkDataSet *, vtkPolyData *&,
                                         bool &);

    void                    AddTubes(vtkPolyData *, vtkPolyData *,
                                     double);
    void                    AddRibbons(vtkPolyData *, vtkPolyData *,
                                     double);
    void                    AddEndPoints(vtkPolyData *, vtkPolyData *,
                                     double);
};


#endif


