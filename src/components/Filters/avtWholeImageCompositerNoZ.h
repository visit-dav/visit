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
