// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Tom Fogal, Fri Oct 24 20:04:04 MDT 2008
//    Add GetAllProcessorsNeedResult method.
//
//    Burlen Loring, Mon Aug 24 16:07:26 PDT 2015
//    Initialize all meber vars via initializer list
//
//    Burlen Loring, Mon Aug 31 14:00:48 PDT 2015
//    Added method for setting background color from double[3]
//
// ****************************************************************************

class AVTFILTERS_API avtWholeImageCompositer : public avtImageCompositer
{
   public:
                              avtWholeImageCompositer() :
                                allReduce(false), chunkSize(1000000),
                                bg_r(255), bg_g(255), bg_b(255) {}

      virtual                ~avtWholeImageCompositer() {};

      void                    SetChunkSize(const int _chunkSize)
                              { chunkSize = _chunkSize; }

      int                     GetChunkSize() const
                              { return chunkSize; }

      void                    SetBackground(const double *rgb)
                              { bg_r = rgb[0]*255.0;
                                bg_g = rgb[1]*255.0;
                                bg_b = rgb[2]*255.0; }

      void                    SetBackground(unsigned char r,
                                            unsigned char g,
                                            unsigned char b)
                              { bg_r = r; bg_g = g; bg_b = b; }

      void                    SetAllProcessorsNeedResult(bool all)
                              { allReduce = all; }

      bool                    GetAllProcessorsNeedResult() const
                              { return allReduce; }

      virtual void            Execute() = 0;

   protected:
      bool                    allReduce;
      int                     chunkSize;
      unsigned char           bg_r;
      unsigned char           bg_g;
      unsigned char           bg_b;
};

#endif
