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
//                  avtExecuteThenTimeLoopFilter.h                           //
// ************************************************************************* //

#ifndef AVT_EXECUTE_THEN_TIME_LOOP_FILTER_H
#define AVT_EXECUTE_THEN_TIME_LOOP_FILTER_H

#include <pipeline_exports.h>

#include <vectortypes.h>

#include <avtDatasetToDatasetFilter.h>


// ****************************************************************************
//  Method: avtExecuteThenTimeLoopFilter
//
//  Purpose:
//    An abstract filter that is similar to the time loop filter.  It
//    allows the pipeline to execute for the current time slice, and then
//    starts iterating over other time slices.  A key difference between
//    this filter and the avtTimeLoopFilter is that this filter treats
//    the current time slice as "special" ... allowing it to examine the
//    current time slice as a way to direct the analysis when it does
//    time iteration.
//
//    The other key difference between the execute-then-time-loop-filter
//    with the avtTimeLoopFilter is that it doesn't use upstream portion of
//    the pipeline when executing.  That is, if the user added a slice 
//    operator, the slice operator will be ignored for the time iteration
//    portion, with the data set retrieval occurring directly from the
//    database (with an EEF).
//   
//  Programmer: Hank Childs
//  Creation:   January 24, 2008
//
// ****************************************************************************

class PIPELINE_API avtExecuteThenTimeLoopFilter 
    : virtual public avtDatasetToDatasetFilter
{
  public:
                                        avtExecuteThenTimeLoopFilter();
    virtual                            ~avtExecuteThenTimeLoopFilter();

    void                                SetTimeLoop(int b, int e, int s)
                                            { startTime = b; 
                                              endTime = e; 
                                              stride = s; };
    void                                SetStartFrame(int b)
                                            { startTime = b; };
    void                                SetEndFrame(int e)
                                            { endTime = e; };
    void                                SetStride(int s)
                                            { stride = s; };

  protected:
    virtual void                        Execute(void);
    virtual void                        InspectPrincipalData(void) {;};
    virtual void                        Iterate(int, avtDataTree_p) = 0;
    virtual void                        Finalize(void) = 0;

    void                                FinalizeTimeLoop(void);
  private:
    int                                 startTime;
    int                                 endTime;
    int                                 stride;
    int                                 nFrames;
    int                                 actualEnd;
};


#endif


