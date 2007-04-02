/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                     avtLocalizedCompactnessExpression.h                    //
// ************************************************************************* //

#ifndef AVT_LOCALIZED_COMPACTNESS_EXPRESSION_H
#define AVT_LOCALIZED_COMPACTNESS_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtLocalizedCompactnessExpression
//
//  Purpose:
//      Calculates localized compactness on a per point basis.
//      This is defined as the percent of the surrounding spherical region that
//      is the same material.
//      
//  Caveats:
//      1) It is assumed that the input is a rectilinear grid.
//      2) User settable knobs are:
//         2a) the radius to consider
//         2b) a weighting factor that, for a given point, allows portions of
//             the spherical region that are closer to count more heavily.
//      3) This calculation works in 2D, but it assumes that the 2D data set
//         is in cylindrical coordinates and it will revolve the data set
//         around the X-axis.
//
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

class EXPRESSION_API avtLocalizedCompactnessExpression
    : public avtSingleInputExpressionFilter
{
  public:
                              avtLocalizedCompactnessExpression();
    virtual                  ~avtLocalizedCompactnessExpression();

    virtual const char       *GetType(void)
                                      { return "avtLocalizedCompactnessExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Calculating localized compactness"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };
    virtual int               GetVariableDimension(void) { return 1; };
};



#endif


