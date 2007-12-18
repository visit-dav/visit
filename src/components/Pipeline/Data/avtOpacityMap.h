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
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
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

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtOpacityMap(const avtOpacityMap &) {;};
    avtOpacityMap       &operator=(const avtOpacityMap &) { return *this; };
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


