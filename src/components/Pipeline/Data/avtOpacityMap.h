// ************************************************************************* //
//                             avtOpacityMap.h                               //
// ************************************************************************* //

#ifndef AVT_OPACITY_MAP_H
#define AVT_OPACITY_MAP_H
#include <pipeline_exports.h>


struct RGBA
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    float         A;
};


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
// ****************************************************************************

class PIPELINE_API avtOpacityMap
{
  public:
                                 avtOpacityMap(int = 1024);
    virtual                     ~avtOpacityMap();

    const RGBA                  *GetTable(void) { return table; };
    void                         SetTable(unsigned char *, int, double = 1.);
    void                         SetTable(RGBA *, int, double = 1.);
    const RGBA                  &GetOpacity(double);

    void                         AddRange(double lo, double hi, RGBA &rgba);

    void                         SetMin(double);
    void                         SetMax(double);
    double                       GetMin(void)     { return min; };
    double                       GetMax(void)     { return max; };

    inline int                   Quantize(const double &);
    int                          GetNumberOfTableEntries(void)
                                                      { return tableEntries; };

  protected:
    RGBA                        *table;
    int                          tableEntries;

    double                       max, min;
    double                       range, inverseRange, multiplier;

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
// ****************************************************************************

inline int
avtOpacityMap::Quantize(const double &val)
{
    int index = (int) ((val-min)*multiplier); 

    //
    // The normal case -- what we calculated was in the range.
    //
    if (index >= 0 && index < tableEntries)
    {
        return index;
    }

    return (index < 0 ? 0 : tableEntries-1);
}


#endif


