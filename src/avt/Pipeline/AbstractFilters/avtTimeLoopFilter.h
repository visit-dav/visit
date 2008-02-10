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
//                       avtTimeLoopFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_TIME_LOOP_FILTER_H
#define AVT_TIME_LOOP_FILTER_H

#include <pipeline_exports.h>
#include <vectortypes.h>
#include <avtFilter.h>



// ****************************************************************************
//  Method: avtTimeLoopFilter
//
//  Purpose:
//    An abstract filter that loops over timesteps in the Update portion.
//    Allows derived combine output from different timesteps into a single
//    output (avtDataObject, avtDataset, avtDataTree). 
//
//    It is up to the derived types to save necessary data for each timestep
//    and to properly combine into an output during CreateFinalOutput.
//
//    Derived types must report whether or not a given exection cycle was
//    successful.  This parent class keeps track of timesteps that executed
//    successfully and those that did not.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   December 29, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005
//    Added SetStartTime, SetEndTime, SetStride and FinalizeTimeLoop.
//  
// ****************************************************************************

class PIPELINE_API avtTimeLoopFilter : virtual public avtFilter
{
  public:
                                        avtTimeLoopFilter();
    virtual                            ~avtTimeLoopFilter();

    virtual bool                        Update(avtContract_p);
    virtual void                        ReleaseData(void);

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
    intVector                           validTimes;
    intVector                           skippedTimes;
    int                                 currentTime;
    avtSILRestriction_p                 currentSILR;
    std::string                         errorMessage;

    virtual void                        CreateFinalOutput(void) = 0;
    virtual bool                        ExecutionSuccessful(void) = 0;

  private:
    int                                 startTime;
    int                                 endTime;
    int                                 stride;
    int                                 nFrames;
    int                                 actualEnd;
    void                                FinalizeTimeLoop(void);
};


#endif


