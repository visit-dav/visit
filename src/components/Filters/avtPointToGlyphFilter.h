// ************************************************************************* //
//                         avtPointToGlyphFilter.h                           //
// ************************************************************************* //

#ifndef AVT_POINT_TO_GLYPH_FILTER_H
#define AVT_POINT_TO_GLYPH_FILTER_H

#include <pipeline_exports.h>

#include <avtStreamer.h>


class vtkDataSet;
class vtkPolyData;


// ****************************************************************************
//  Class: avtPointToGlyphFilter
//
//  Purpose:
//      Creates a glyph for each point in a VTK dataset.  Current this only
//      creates hexahedrons/quads.
//
//  Programmer: Hank Childs
//  Creation:   June 22, 2002
//
//  Modifications:
//
//    Jeremy Meredith, Fri Dec 20 11:32:51 PST 2002
//    Added code to scale by a variable.
//
//    Hank Childs, Thu Aug 21 22:03:57 PDT 2003
//    Added more point glyphs.
//
// ****************************************************************************

class PIPELINE_API avtPointToGlyphFilter : public avtStreamer
{
  public:
                         avtPointToGlyphFilter();
    virtual             ~avtPointToGlyphFilter();

    void                 SetPointSize(double);
    void                 SetScaleVariable(const std::string&);
    void                 SetScaleByVariableEnabled(bool);
    void                 SetGlyphType(int);

    virtual const char  *GetType(void) { return "avtPointToGlyphFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Creating point glyphs"; };

  protected:
    double               pointSize;
    int                  glyphType;
    std::string          scaleVar;
    bool                 scaleByVarEnabled;
    vtkPolyData         *glyph3D;
    vtkPolyData         *glyph2D;

    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
    virtual void         RefashionDataObjectInfo(void);

    void                 SetUpGlyph();
};


#endif


