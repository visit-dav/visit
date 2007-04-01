// ************************************************************************* //
//                              avtCompositeRF.h                             //
// ************************************************************************* //

#ifndef AVT_COMPOSITE_RF_H
#define AVT_COMPOSITE_RF_H
#include <pipeline_exports.h>


#include <avtRangeMaxTable.h>
#include <avtRayFunction.h>


class     avtOpacityMap;
struct    RGBA;


// ****************************************************************************
//  Class: avtCompositeRF
//
//  Purpose:
//      A derived type of ray function, this will take sample points and
//      composite them together using an opacity map.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Notes:      The two opacity maps are redundant.  If I had more time, they
//              would be split into a formal opacity map as one and a color map
//              as the other.
//    
//  Modifications:
//
//    Hank Childs, Sun Dec  2 15:55:28 PST 2001
//    Added support for opacity variables.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C files because certain compilers 
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtCompositeRF : public avtRayFunction
{
  public:
                       avtCompositeRF(avtLightingModel *, avtOpacityMap *,
                                      avtOpacityMap *);
    virtual           ~avtCompositeRF();

    virtual void       GetRayValue(const avtRay *, const avtGradients *,
                                   unsigned char rgb[3], float);

    void               SetColorVariableIndex(int cvi)
                                               { colorVariableIndex = cvi; };
    void               SetOpacityVariableIndex(int ovi)
                                               { opacityVariableIndex = ovi; };
    virtual bool       CanContributeToPicture(int,
                                          const float (*)[AVT_VARIABLE_LIMIT]);
    virtual float      ClassifyForShading(float);

  protected:
    avtOpacityMap     *map;
    avtOpacityMap     *secondaryMap;
    const RGBA        *table;
    const RGBA        *secondaryTable;
    int                colorVariableIndex;
    int                opacityVariableIndex;

    avtRangeMaxTable   rangeMaxTable;
    virtual bool       NeedsGradientsForFunction(void);
};


#endif


