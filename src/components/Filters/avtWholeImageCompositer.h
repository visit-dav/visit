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

      virtual void            Execute() = 0;

   protected:

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

#endif
