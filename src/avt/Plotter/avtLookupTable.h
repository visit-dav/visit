/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                          avtLookupTable.h                                 //
// ************************************************************************* //

#ifndef AVT_LOOKUP_TABLE_H
#define AVT_LOOKUP_TABLE_H
#include <plotter_exports.h>


class vtkLookupTable;
class vtkLogLookupTable;
class vtkSkewLookupTable;


// ****************************************************************************
//  Class: avtLookupTable
//
//  Purpose:  
//    Serves as a wrapper for the various types of vtkLookupTable.
//    Provides consistent and central place for vtkLookupTable manipulation.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 27, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 20 11:18:31 EST 2009
//    Allowed SetColorTable to optionally also use the color table's opacity.
//    Added new SetLUTColorsAndOpacity method to actually perform the work.
//
// ****************************************************************************

class PLOTTER_API avtLookupTable
{
  public:
                                  avtLookupTable();
    virtual                      ~avtLookupTable();

    void                          SetSkewFactor(const double);
    bool                          SetColorTable(const char *ctName, bool,
                                                bool useOpacities = false);
    void                          SetLUTColors(const unsigned char *, int);
    void                          SetLUTColorsWithOpacity(
                                      const unsigned char *, int);
    void                          SetLUTColorsAndOpacity(
                                      const unsigned char *,
                                      const unsigned char *,
                                      int);

    int                           GetNumberOfColors(void);

    vtkLookupTable               *GetLookupTable(void) 
                                      { return stdLUT; };
    vtkLookupTable               *GetLogLookupTable(void)
                                      { return (vtkLookupTable*) logLUT; };
    vtkLookupTable               *GetSkewLookupTable(void)
                                      { return (vtkLookupTable*) skewLUT; };

  protected:
    vtkLookupTable               *stdLUT;
    vtkLogLookupTable            *logLUT;
    vtkSkewLookupTable           *skewLUT;
};


#endif


