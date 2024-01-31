// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtVectorGlyphMapper.h                         //
// ************************************************************************* //

#ifndef AVT_VECTOR_GLYPH_MAPPER_H
#define AVT_VECTOR_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtMapper.h>
#include <LineAttributes.h>

class     vtkLookupTable;
class     vtkVisItGlyph3D;


// ****************************************************************************
//  Class: avtVectorGlyphMapper
//
//  Purpose:
//      A mapper for glyph.  This extends the functionality of a mapper by
//      mapping a glyph onto a dataset with a vector variable.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Jun 15 16:00:24 PST 2001
//    Added SetLUTColors method.
//
//    Kathleen Bonnell, Mon Jun 25 12:45:06 PDT 2001 
//    Added SetLineStyle method, stipplePattern data member.
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk. 
//
//    Kathleen Bonnell, Mon Aug 20 18:19:25 PDT 2001 
//    Removed int parameter from InsertFilters, SetUpFilters as it
//    was associated with a mode no longer supported by avtMapper.
//    Also removed member glyphFilterStride, as it is no longer necessary.  
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Added vtkLookupTable member and Set method.  Removed SetLUTColors. 
//    
//    Hank Childs, Wed Sep 24 09:42:29 PDT 2003
//    Renamed to vector glyph mapper.
//
//    Hank Childs, Tue May  4 16:47:29 PDT 2004
//    Allow for normals-generation.
//
//    Kathleen Bonnell, Mon Aug  9 13:54:42 PDT 2004 
//    Added flag 'colorByScalar', 'scalarName', and method ColorByScalarOn. 
//
//    Kathleen Bonnell, Tue Oct 12 16:18:37 PDT 2004
//    Use VisIt's version of vtkGlyph3d. 
//
//    Eric Brugger, Tue Nov 23 12:26:05 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004 
//    Added methods and vars to support setting of limits for coloring. 
//
//    Brad Whitlock, Mon Jul 24 13:55:10 PST 2006
//    Added override of SetFullFrameScaling.
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PST 2013
//    Changed signature of InsertFilters.  Changed signature of constructor
//    to accept vtkAlgorithmOutput for pipeline connections with vtk-6.
//
//    Eric Brugger, Wed Mar  8 16:50:08 PST 2023
//    Moved GetVarRange to avtMapper.
//
// ****************************************************************************

class PLOTTER_API  avtVectorGlyphMapper : public avtMapper
{
  public:
                               avtVectorGlyphMapper(vtkAlgorithmOutput *);
    virtual                   ~avtVectorGlyphMapper();

    void                       SetLineWidth(_LineWidth lw);
    void                       ColorByMagOn(void);
    void                       ColorByScalarOn(const std::string &);
    void                       ColorByMagOff(const unsigned char [3]);
    void                       SetScale(double);
    void                       SetScaleByMagnitude(bool);
    void                       SetAutoScale(bool);
    void                       SetLookupTable(vtkLookupTable *lut);
    virtual bool               SetFullFrameScaling(bool, const double *);
   
    // methods for setting limits for coloring
    void                       SetLimitsMode(const int);
    void                       SetMin(double);
    void                       SetMinOff(void);
    void                       SetMax(double);
    void                       SetMaxOff(void);
    virtual bool               GetRange(double &, double &);
    virtual bool               GetCurrentRange(double &, double &);

  protected:
    vtkAlgorithmOutput        *glyph;
    vtkLookupTable            *lut;
    _LineWidth                 lineWidth;
    bool                       colorByMag;
    bool                       colorByScalar;
    std::string                scalarName;
    unsigned char              glyphColor[3];
    double                     scale;
    bool                       scaleByMagnitude;
    bool                       autoScale;

    double                     min, max;
    bool                       setMin, setMax;
    int                        limitsMode;

    vtkVisItGlyph3D           **glyphFilter;
    int                        nGlyphFilters;

    virtual void               CustomizeMappers(void);
    void                       SetMappersMinMax(void);

    virtual vtkAlgorithmOutput *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);
};


#endif


