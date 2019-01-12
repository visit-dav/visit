/*****************************************************************************
*
* Copyright (c) 2010, University of New Hampshire Computer Science Department
* All rights reserved.
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
//                          avtResolutionSelection.h                         //
// ************************************************************************* //
#ifndef _AVT_RESOLUTION_SELECTION_H_
#define _AVT_RESOLUTION_SELECTION_H_
#include <pipeline_exports.h>

#include "avtDataSelection.h"

// ****************************************************************************
//  Class: avtResolutionSelection
//
//  Purpose:
//
//    This class is used to communicate a selected resolution from the
//    VisIt UI.
//
//  Programmer: Andrew Foulks <rafoulks@cs.unh.edu>
//  Creation:   Winter 2009
//
//  Modifications:
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
// ****************************************************************************
class PIPELINE_API avtResolutionSelection : public avtDataSelection
{
public:
                         avtResolutionSelection() {}
    virtual              ~avtResolutionSelection() {}
    // base class api
    virtual const char*  GetType() const { return "avtResolutionSelection"; }
    virtual std::string     DescriptionString(void);

    // added functionality
    virtual void         setResolution(int r) { mResolution = r; }
    virtual int          resolution() const { return mResolution; }

private:
    int mResolution;
};

#endif // _AVT_RESOLUTION_SELECTION_H_

