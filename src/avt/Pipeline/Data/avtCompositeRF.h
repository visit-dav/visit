/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Add a lighting model.
//
//    Aliter Maguire, Mon Jun  3 15:40:31 PDT 2019
//    Added viewDistance with setter. 
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
    void               SetDistance(int dist)
                                               { viewDistance = dist; }; 

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
    int                viewDistance;

    avtRangeMaxTable   rangeMaxTable;
    bool               trilinearSampling;
    double             matProperties[4];  //ambient, diffuse, specular, shininess
};


#endif


