/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
    void               SetWeightVariableIndex(int wvi)
                                               { weightVariableIndex = wvi; };
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
    int                weightVariableIndex;

    avtRangeMaxTable   rangeMaxTable;
    virtual bool       NeedsGradientsForFunction(void);
};


#endif


