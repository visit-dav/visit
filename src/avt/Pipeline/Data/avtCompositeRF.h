// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Hank Childs, Sat Jan  7 17:50:22 PST 2006
//    Add support for kernel based sampling.
//
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Add a lighting model.
//
//    Aliter Maguire, Mon Jun  3 15:40:31 PDT 2019
//    Added viewDistance with setter. 
//
//    Alister Maguire, Wed Oct  7 16:30:23 PDT 2020
//    Removed viewDistance.
//
// ****************************************************************************

class PIPELINE_API avtCompositeRF : public avtRayFunction
{
  public:
                       avtCompositeRF(avtLightingModel *, avtOpacityMap *,
                                      avtOpacityMap *);
    virtual           ~avtCompositeRF();

    virtual void       GetRayValue(const avtRay *,
                                   unsigned char rgb[3], double);

    void               SetColorVariableIndex(int cvi)
                                               { colorVariableIndex = cvi; };
    void               SetOpacityVariableIndex(int ovi)
                                               { opacityVariableIndex = ovi; };
    virtual int        GetOpacityVariableIndex() const
                                               { return opacityVariableIndex; }
    void               SetWeightVariableIndex(int wvi)
                                               { weightVariableIndex = wvi; };
    virtual int        GetWeightVariableIndex() const
                                               { return weightVariableIndex; }
    virtual bool       CanContributeToPicture(int,
                                          const double (*)[AVT_VARIABLE_LIMIT]);

    void               SetTrilinearSampling(bool r) { trilinearSampling = r; };
    void               SetMaterial(double mat[4]){for (int i=0; i<4; i++) matProperties[i]=mat[i];}

  protected:
    avtOpacityMap     *map;
    avtOpacityMap     *secondaryMap;
    const RGBA        *table;
    const RGBA        *secondaryTable;
    int                colorVariableIndex;
    int                opacityVariableIndex;
    int                weightVariableIndex;

    avtRangeMaxTable   rangeMaxTable;
    bool               trilinearSampling;
    double             matProperties[4];  //ambient, diffuse, specular, shininess
};


#endif


