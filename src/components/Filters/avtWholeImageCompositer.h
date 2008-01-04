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
//                            avtWholeImageCompositer.h                      //
// ************************************************************************* //

#ifndef AVT_WHOLE_IMAGE_COMPOSITER_H
#define AVT_WHOLE_IMAGE_COMPOSITER_H

#include <filters_exports.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtImageCompositer.h>

// ****************************************************************************
//  Class: avtWholeImageCompositer
//
//  Purpose: Base class for whole image compositors. That is a compositor in
//      which every piece of image to be composited has origin 0,0 and size
//      of the intended output image. Every piece being composited is a
//      whole image. All the algorithms for chunking and message passing
//      depend on this being the case.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
//  Modifications:
//
//    Mark C. Miller, Tue Oct 19 15:35:12 PDT 2004
//    Turned into a base class
//
//    Jeremy Meredith, Mon Aug 30 16:14:13 PDT 2004
//    I added the ability for the avtWholeImageCompositer to use an
//    Allreduce in the event this is used as the end of the first stage
//    in a two-pass compositing scheme.
//
// ****************************************************************************

class AVTFILTERS_API avtWholeImageCompositer : public avtImageCompositer
{
   public:
                              avtWholeImageCompositer() {
                                  chunkSize = 1000000;
                                  bg_r = 255; bg_g = 255; bg_b = 255; };

      virtual                ~avtWholeImageCompositer() {};

      void                    SetChunkSize(const int chunkSize);
      int                     GetChunkSize() const;
      void                    SetBackground(unsigned char r,
                                            unsigned char g,
                                            unsigned char b);
      void                    SetAllProcessorsNeedResult(bool);

      virtual void            Execute() = 0;

   protected:

      bool                    allReduce;
      int                     chunkSize;
      unsigned char           bg_r;
      unsigned char           bg_g;
      unsigned char           bg_b;

};

inline void avtWholeImageCompositer::SetChunkSize(const int _chunkSize)
{ chunkSize = _chunkSize; }

inline int avtWholeImageCompositer::GetChunkSize() const
{ return chunkSize; }

inline void avtWholeImageCompositer::SetBackground(unsigned char r,
                                                   unsigned char g,
                                                   unsigned char b)
{ bg_r = r; bg_g = g; bg_b = b; }

inline void avtWholeImageCompositer::SetAllProcessorsNeedResult(bool all)
{
    allReduce = all;
}

#endif
