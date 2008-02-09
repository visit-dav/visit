/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                         avtSubsetFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_SUBSET_FILTER_H
#define AVT_SUBSET_FILTER_H


#include <avtDataTreeStreamer.h>
#include <SubsetAttributes.h>


// ****************************************************************************
//  Class: avtSubsetFilter
//
//  Purpose:  Ensures that the correct subset names are passed along
//            as labels.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//    Eric Brugger, Fri Dec 14 13:08:12 PST 2001
//    I modified the class to inherit from avtDataTreeStreamer so that I
//    could implement an ExecuteDataTree method instead of the Execute
//    Method.
//
//    Kathleen Bonnell, Mon Apr 29 17:31:22 PDT 2002
//    I added PostExecute so that accurate labels could be passed via
//    the DataAttributes object. 
//
//    Kathleen Bonnell, Fri Nov 12 11:50:33 PST 2004
//    Moved constructor to source code, added keepNodeZone data member. 
//
// ****************************************************************************

class avtSubsetFilter : public avtDataTreeStreamer
{
  public:
                          avtSubsetFilter();
    virtual              ~avtSubsetFilter(){}; 

    virtual const char   *GetType(void) {return "avtSubsetFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Setting subset names"; };

    void                  SetPlotAtts(const SubsetAttributes *);

  protected:
    SubsetAttributes      plotAtts;

    virtual avtDataTree_p ExecuteDataTree(vtkDataSet *, int, string);
    virtual void          UpdateDataObjectInfo(void);
    virtual avtContract_p
                          ModifyContract(avtContract_p);

    virtual void          PostExecute(void);

  private:
    bool                  keepNodeZone;
};


#endif


