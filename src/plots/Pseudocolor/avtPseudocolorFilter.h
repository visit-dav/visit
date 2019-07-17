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
//                           avtPseudocolorFilter.h                          //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLOR_FILTER_H
#define AVT_PSEUDOCOLOR_FILTER_H

#include <avtDataTreeIterator.h>

#include <PseudocolorAttributes.h>

#include <string>


// ****************************************************************************
//  Class: avtPseudocolorFilter
//
//  Purpose:  To set specific flags in the pipeline that may be necessary
//            when picking on point meshes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 29, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 11:35:11 PST 2004
//    Added PlotAtts.
//
//    Kathleen Biagas, Fri Nov  2 10:25:25 PDT 2012
//    Added primaryVar.
//
//    Eric Brugger, Tue Aug 19 11:10:33 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Alister Maguire, Tue Jul 16 14:12:20 PDT 2019
//    Added mustRemoveFacesBeforeGhosts. 
//
// ****************************************************************************

class avtPseudocolorFilter : public avtDataTreeIterator
{
  public:
                              avtPseudocolorFilter();
    virtual                  ~avtPseudocolorFilter();

    virtual const char       *GetType(void)   { return "avtPseudocolorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Preparing pipeline"; };
    void                      SetPlotAtts(const PseudocolorAttributes *);

  protected:
    bool                      keepNodeZone;
    bool                      mustRemoveFacesBeforeGhosts;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p
                              ModifyContract(avtContract_p);
  private:
    PseudocolorAttributes     plotAtts;
    std::string               primaryVar;
};


#endif


