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
//                        avtStreamlinePolyDataFilter.h                      //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_POLY_DATA_FILTER_H
#define AVT_STREAMLINE_POLY_DATA_FILTER_H

#include <avtStreamlineFilter.h>


// ****************************************************************************
// Class: avtStreamlinePolyDataFilter
//
// Purpose:
//     This class inherits from avtStreamlineFilter and its sole job is to
//     implement CreateStreamlineOutput, which it does by creating vtkPolyData.
//
// Notes:  The original implementation of CreateStreamlineOutput was in
//         avtStreamlineFilter and was by Dave Pugmire.  That code was moved to
//         this module by Hank Childs during a later refactoring that allowed
//         the avtStreamlineFilter to be used in more places.
//
// Programmer: Hank Childs (refactoring) / Dave Pugmire (actual code)
// Creation:   December 2, 2008
//
//   Dave Pugmire, Mon Feb  2 14:39:35 EST 2009
//   Moved GetVTKPolyData from avtStreamlineWrapper to here.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Add tangentsArrayName (for Christoph Garth).
//
// ****************************************************************************

class AVTFILTERS_API avtStreamlinePolyDataFilter : public avtStreamlineFilter
{
  public:
                              avtStreamlinePolyDataFilter() {}
    virtual                  ~avtStreamlinePolyDataFilter() {}
    static                    std::string colorvarArrayName;
    static                    std::string paramArrayName;
    static                    std::string opacityArrayName;
    static                    std::string thetaArrayName;
    static                    std::string tangentsArrayName;

  protected:
    vtkPolyData*              GetVTKPolyData(avtStreamline *sl, int id);
    void                      CreateStreamlineOutput( 
                                 vector<avtStreamlineWrapper *> &streamlines );
};


#endif


