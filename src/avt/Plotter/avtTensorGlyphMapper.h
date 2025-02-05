// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtTensorGlyphMapper.h                         //
// ************************************************************************* //

#ifndef AVT_TENSOR_GLYPH_MAPPER_H
#define AVT_TENSOR_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtMapper.h>

class     vtkVisItTensorGlyph;
class     vtkAlgorithmOutput;
class     vtkLookupTable;
class     vtkPolyData;


// ****************************************************************************
//  Class: avtTensorGlyphMapper
//
//  Purpose:
//      A mapper for tensor.  This extends the functionality of a mapper by
//      glyphing tensors onto ellipsoids.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Wed May  5 16:23:29 PDT 2004
//    Add normals calculation.
//
//    Eric Brugger, Wed Nov 24 12:58:22 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005 
//    Use VisIt's version of TensorGlyph so that orignal zone and node
//    arrays can be copied through.
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PST 2013
//    Changed signature of InsertFilters.
//
//    Kathleen Biagas, Thu Feb 7 08:45:03 PST 2013
//    Changed signature of constructor to accept vtkAlgorithmInput in order
//    to preserve pipeline connections with vtk-6.
//
//    Kathleen Biagas, Thu Mar 14 13:03:50 PDT 2013
//    Remove normalsFilter.
//
//    Eric Brugger, Wed Mar  8 16:50:08 PST 2023
//    Moved GetVarRange to avtMapper.
//
// ****************************************************************************

class PLOTTER_API  avtTensorGlyphMapper : public avtMapper
{
  public:
                               avtTensorGlyphMapper(vtkAlgorithmOutput *);
    virtual                   ~avtTensorGlyphMapper();

    void                       ColorByMagOn(void);
    void                       ColorByMagOff(const unsigned char [3]);
    void                       SetScale(double);
    void                       SetScaleByMagnitude(bool);
    void                       SetAutoScale(bool);
    void                       SetLookupTable(vtkLookupTable *lut);

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
    bool                       colorByMag;
    unsigned char              glyphColor[3];
    double                     scale;
    bool                       scaleByMagnitude;
    bool                       autoScale;

    double                     min, max;
    bool                       setMin, setMax;
    int                        limitsMode;

    vtkVisItTensorGlyph      **tensorFilter;
    int                        nTensorFilters;

    virtual void               CustomizeMappers(void);
    void                       SetMappersMinMax(void);

    virtual vtkAlgorithmOutput *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);
};


#endif


