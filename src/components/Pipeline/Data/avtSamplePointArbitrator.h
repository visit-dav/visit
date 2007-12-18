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
//                         avtSamplePointArbitrator.h                        //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_ARBITRATOR_H
#define AVT_SAMPLE_POINT_ARBITRATOR_H

#include <pipeline_exports.h>

class    avtOpacityMap;


// ****************************************************************************
//  Class: avtSamplePointArbitrator
//
//  Purpose:
//      Decides which sample point to use when multiple cells map to the same
//      sample.  The algorithm it uses is defined by its derived type.
//
//  Programmer: Hank Childs
//  Creation:   January 23, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Sat Jan 29 10:22:56 PST 2005
//    Made ShouldOverwrite be pure virtual.  This is now an abstract type.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointArbitrator
{
  public:
                             avtSamplePointArbitrator(int);
    virtual                 ~avtSamplePointArbitrator();

    int                      GetArbitrationVariable(void)
                                               { return arbitrationVariable; };
    virtual bool             ShouldOverwrite(float, float) = 0;

  protected:
    int                      arbitrationVariable;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                              avtSamplePointArbitrator(const 
                                               avtSamplePointArbitrator &) {;};
    avtSamplePointArbitrator &operator=(const avtSamplePointArbitrator &) 
                                                             { return *this; };
};


#endif



