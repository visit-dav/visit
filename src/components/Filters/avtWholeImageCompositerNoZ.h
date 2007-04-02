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
//                         avtWholeImageCompositerNoZ.h                      //
// ************************************************************************* //

#ifndef AVT_WHOLE_IMAGE_COMPOSITER_NOZ_H
#define AVT_WHOLE_IMAGE_COMPOSITER_NOZ_H

#include <filters_exports.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtWholeImageCompositer.h>

// ****************************************************************************
//  Class: avtWholeImageCompositerNoZ
//
//  Purpose:
//      An image compositor based largely on MeshTV's image compositer.
//      The key limitation of this image compositer is that it assumes that
//      every piece of image to be composited has origin 0,0 and size of the
//      intended output image. That is, every piece being composited is a
//      whole image. All the algorithms for chunking and message passing
//      depend on this being the case.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
// ****************************************************************************

class AVTFILTERS_API avtWholeImageCompositerNoZ : public avtWholeImageCompositer
{
   public:
                              avtWholeImageCompositerNoZ();
      virtual                ~avtWholeImageCompositerNoZ();

      const char             *GetType(void);
      const char             *GetDescription(void);

      void                    Execute();

   private:

      void                    MergeBuffers(int npixels, bool doParallel,
                                 const unsigned char *inrgb,
                                 unsigned char *iorgb);

      static int              objectCount;

      static void             InitializeMPIStuff();
      static void             FinalizeMPIStuff();

#ifdef PARALLEL
      static MPI_Datatype     mpiTypeZFPixel;
      static MPI_Op           mpiOpMergeZFPixelBuffers;
#endif

};

inline const char* avtWholeImageCompositerNoZ::GetType()
{ return "avtWholeImageCompositerNoZ";}

inline const char* avtWholeImageCompositerNoZ::GetDescription()
{ return "performing whole-image composite without zbuffer"; }

#endif
