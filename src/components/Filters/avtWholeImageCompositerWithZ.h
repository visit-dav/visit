// ************************************************************************* //
//                       avtWholeImageCompositerWithZ.h                      //
// ************************************************************************* //

#ifndef AVT_WHOLE_IMAGE_COMPOSITER_WITHZ_H
#define AVT_WHOLE_IMAGE_COMPOSITER_WITHZ_H

#include <filters_exports.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtWholeImageCompositer.h>

// ****************************************************************************
//  Class: avtWholeImageCompositerWithZ
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

class AVTFILTERS_API avtWholeImageCompositerWithZ :
    public avtWholeImageCompositer
{
   public:
                              avtWholeImageCompositerWithZ();
      virtual                ~avtWholeImageCompositerWithZ();

      const char             *GetType(void);
      const char             *GetDescription(void);

      void                    Execute();

   private:

      void                    MergeBuffers(int npixels, bool doParallel,
                                 const float *inz, const unsigned char *inrgb,
                                 float *ioz, unsigned char *iorgb);

      static void             InitializeMPIStuff();
      static void             FinalizeMPIStuff();

      static int              objectCount;

#ifdef PARALLEL
      static MPI_Datatype     mpiTypeZFPixel;
      static MPI_Op           mpiOpMergeZFPixelBuffers;
#endif

};

inline const char* avtWholeImageCompositerWithZ::GetType()
{ return "avtWholeImageCompositerWithZ";}

inline const char* avtWholeImageCompositerWithZ::GetDescription()
{ return "performing whole-image composite with zbuffer"; }

#endif
