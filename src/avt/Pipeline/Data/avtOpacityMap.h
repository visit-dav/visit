// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtOpacityMap.h                               //
// ************************************************************************* //

#ifndef AVT_OPACITY_MAP_H
#define AVT_OPACITY_MAP_H

#include <pipeline_exports.h>
#include <visitstream.h>
#include <visit-config.h> // For LIB_VERSION_GE

struct RGBA
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    float         A;
};

struct RGBAF
{
    float R;
    float G;
    float B;
    float A;
};

// added by Qi, for using RGBAF nicely outside the function

// ****************************************************************************
//  Class: avtOpacityMap
//
//  Purpose:
//      Defines a mapping over an interval [low, high] into the
//      interval [0, 1].  This is used by the volume rendering module.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 11:46:33 PST 2001
//    Made map be table based for performance reasons.
//
//    Hank Childs, Mon Feb 19 09:01:31 PST 2001
//    Made opacity a float instead of an unsigned char.
//
//    Hank Childs, Tue Dec 21 16:38:33 PST 2004
//    Add support for attenuation.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Qi WU, Sat Jun 10 22:21:27 MST 2018
//    Add function 'SetTableFloatNOC' to generate a transfer function without
//    the opacity correction term. This function will be used for OSPRay
//    volume rendering
//
// ****************************************************************************

class PIPELINE_API avtOpacityMap
{
  public:
                                 avtOpacityMap(int = 1024);
                                 avtOpacityMap(const avtOpacityMap &obj);
    virtual                     ~avtOpacityMap();

    void operator = (const avtOpacityMap &obj);

    const RGBA                  *GetTable(void) { return table; };
#if LIB_VERSION_LE(VTK,8,1,0)
    const RGBAF                 *GetTableFloat(void) { return transferFn1D; };
    void                         SetTableFloat(unsigned char *arr, int te,
                                               double attenuation, float over);
    void                         SetTable(unsigned char *arr, int te,
                                          double attenuation, float over);
    void                         SetTableFloatNOC(unsigned char *arr, int te,
                                                  double attenuation);
#else
    const RGBAF                 *GetTransferFunc(void) { return transferFn1D; };
    void                         SetTableComposite(unsigned char *arr, int te,
                                               double attenuation, float over);
#endif

    void                         SetTable(unsigned char *, int, double = 1.);
    void                         SetTable(RGBA *, int, double = 1.);

    const RGBA                  &GetOpacity(double);

    void                         AddRange(double lo, double hi, RGBA &rgba);

    void                         SetMin(double);
    void                         SetMax(double);
    double                       GetMin(void)     { return min; };
    double                       GetMax(void)     { return max; };
    double                       GetMinVisibleScalar();
    double                       GetMaxVisibleScalar();

    void                         ComputeVisibleRange();

    inline int                   Quantize(const double &);
    int                          GetNumberOfTableEntries(void)
                                                      { return tableEntries; };

    float                        QuantizeValF(const double &val);
    int                          QueryTF(double scalarValue,
                                         double color[4]) const;
    float                        QueryAlpha(double scalarValue) const;

    friend PIPELINE_API ostream &operator <<(ostream &, const avtOpacityMap &);
  protected:
    RGBA                        *table;
    RGBAF                       *transferFn1D;
    int                          tableEntries;

    double                       max, min;
    double                       range, inverseRange, multiplier;
    int                          minVisibleScalarIndex, maxVisibleScalarIndex;
    double                       minVisibleScalar, maxVisibleScalar;

    void                         SetIntermediateVars(void);
};


// ****************************************************************************
//  Method: avtOpacityMap::Quantize
//
//  Purpose:
//      Quantizes the value based on the min/max.
//
//  Arguments:
//      val     The value.
//
//  Returns:    The index into the table.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Feb 27 20:55:30 PST 2001
//    Fix error in casting.
//
//    Hank Childs, Sun Jul 17 16:56:52 PDT 2011
//    If the true data range is _really_ large, and the user set data range is
//    small, then you can get overflows.  Set explicit checks for min and max,
//    to prevent misclassification in the case of overflows.
//
// ****************************************************************************

inline int
avtOpacityMap::Quantize(const double &val)
{
    int index = (int) ((val-min)*multiplier);

    // This should be handled by the logic below ... but if we have
    // a large range for the variable, but the user set's a narrow range
    // of min/max, then we can have overflows that lead to bad pictures.
    if (val < min)
        return 0;
    if (val > max)
        return tableEntries-1;

    //
    // The normal case -- what we calculated was in the range.
    //
    if (index >= 0 && index < tableEntries)
    {
        return index;
    }

    return (index < 0 ? 0 : tableEntries-1);
}

inline float
avtOpacityMap::QuantizeValF(const double &val){
    float testVal = ((val-min)*multiplier);

    if (val < min)
        return 0;
    if (val > max)
        return (float)(tableEntries-1);

    //
    // The normal case -- what we calculated was in the range.
    //
    if (testVal >= 0 && testVal < tableEntries)
    {
        return testVal;
    }

    return (testVal < 0 ? 0 : tableEntries-1);
}


// ****************************************************************************
//  Method: avtOpacityMap::QueryTF
//
//  Purpose:
//      Queries a Transfer function for the color based on the scalr value
//      passed in
//
//  Arguments:
//      scalarValue     scalar value
//      color           the color queried from the transfer function based on
//                      the scalar value
//
//  Returns:
//
//  Programmer: Pascal Grosset
//  Creation:   June 3, 2013
//
//  Modifications:
//
//    Qi Wu, Tue Aug 8 12:47:52 MT 2017
//    Fixed index overflow problem when the scalar value reaches its maximum
//
//    Qi WU, Sat Jun 10 22:21:27 MST 2018
//    Fix bad coding formats
//
// ****************************************************************************
inline int
avtOpacityMap::QueryTF(double scalarValue, double color[4]) const
{
    if (scalarValue <= min){
        int index = 0;

        RGBAF colorRGBA = transferFn1D[index];
        color[0] = colorRGBA.R;
        color[1] = colorRGBA.G;
        color[2] = colorRGBA.B;
        color[3] = colorRGBA.A;

        return 0;
    }

    if (scalarValue >= max){
        int index = tableEntries-1;
        RGBAF colorRGBA = transferFn1D[index];
        color[0] = colorRGBA.R;
        color[1] = colorRGBA.G;
        color[2] = colorRGBA.B;
        color[3] = colorRGBA.A;

        return 0;
    }

    int indexLow, indexHigh;
    RGBAF colorRGBALow, colorRGBAHigh;
    double colorLow[4], colorHigh[4];
    float indexPos, indexDiff;

    indexPos  = (scalarValue-min)/(max-min) * (tableEntries-1);
    // ^^^^ multiplier = 1.0/(max-min) * tableEntries
    indexLow  = (int)indexPos;
    indexHigh = (int)(indexPos+1.0);

    indexDiff = indexPos - indexLow;

    colorRGBALow = transferFn1D[indexLow];
    colorLow[0] = colorRGBALow.R;
    colorLow[1] = colorRGBALow.G;
    colorLow[2] = colorRGBALow.B;
    colorLow[3] = colorRGBALow.A;

    colorRGBAHigh = transferFn1D[indexHigh];
    colorHigh[0] = colorRGBAHigh.R;
    colorHigh[1] = colorRGBAHigh.G;
    colorHigh[2] = colorRGBAHigh.B;
    colorHigh[3] = colorRGBAHigh.A;

    color[0] = (1.0-indexDiff)*colorLow[0] + indexDiff*colorHigh[0];
    color[1] = (1.0-indexDiff)*colorLow[1] + indexDiff*colorHigh[1];
    color[2] = (1.0-indexDiff)*colorLow[2] + indexDiff*colorHigh[2];
    color[3] = (1.0-indexDiff)*colorLow[3] + indexDiff*colorHigh[3];

    return 1;
}

inline float
avtOpacityMap::QueryAlpha(double scalarValue) const
{
    if (scalarValue <= min)
    {
        return transferFn1D[0].A;
    }

    if (scalarValue >= max)
    {
        int index = tableEntries-1;
        return transferFn1D[index].A;
    }

    float indexPos  = static_cast<float>((scalarValue-min) * multiplier);
    int   indexLow  = static_cast<int>(indexPos);
    int   indexHigh = static_cast<int>(indexPos+1.0);
    float indexDiff = indexPos - indexLow;

    float a0 = transferFn1D[indexLow].A;
    float a1 = transferFn1D[indexHigh].A;

    float alpha = a0 + indexDiff * (a1 - a0);

    return alpha;
}

#endif
