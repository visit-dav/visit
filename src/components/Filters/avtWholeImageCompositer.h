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

class AVTFILTERS_API avtWholeImageCompositer : public avtImageCompositer
{
   public:
                              avtWholeImageCompositer();
      virtual                ~avtWholeImageCompositer();

      const char             *GetType(void);
      const char             *GetDescription(void);

      void                    SetChunkSize(const int chunkSize);
      int                     GetChunkSize() const;
      void                    SetBackground(unsigned char r,
                                            unsigned char g,
                                            unsigned char b);

      void                    Execute();

   private:

      friend void             MergeBuffers(avtWholeImageCompositer *thisObj,
                                 int npixels, bool doParallel,
                                 const float *inz, const unsigned char *inrgb,
                                 float *ioz, unsigned char *iorgb);

      static int              objectCount;

      int                     chunkSize;
      unsigned char           bg_r;
      unsigned char           bg_g;
      unsigned char           bg_b;

      static void             InitializeMPIStuff();
      static void             FinalizeMPIStuff();

#ifdef PARALLEL
      static MPI_Datatype     mpiTypeZFPixel;
      static MPI_Op           mpiOpMergeZFPixelBuffers;
#endif

};

inline const char* avtWholeImageCompositer::GetType()
{ return "avtWholeImageCompositer";}

inline const char* avtWholeImageCompositer::GetDescription()
{ return "performing whole-image composite"; }

inline void avtWholeImageCompositer::SetChunkSize(const int _chunkSize)
{ chunkSize = _chunkSize; }

inline int avtWholeImageCompositer::GetChunkSize() const
{ return chunkSize; }

inline void avtWholeImageCompositer::SetBackground(unsigned char r,
                                                   unsigned char g,
                                                   unsigned char b)
{ bg_r = r; bg_g = g; bg_b = b; }

#endif
