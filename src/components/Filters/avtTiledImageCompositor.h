// ************************************************************************* //
//                            avtTiledImageCompositor.h                      //
// ************************************************************************* //

#ifndef AVT_TILED_IMAGE_COMPOSITOR_H
#define AVT_TILED_IMAGE_COMPOSITOR_H
#include <filters_exports.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtImageCompositer.h>

// ****************************************************************************
//  Class: avtTiledImageCompositor
//
//  Purpose:
//      Composes images that have disparate spatial regions.  In other words,
//      it assumes each processor owns some screen-space tile of the whole
//      image and compositing need not use the Z-buffer at all.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2004
//
// ****************************************************************************

class AVTFILTERS_API avtTiledImageCompositor : public avtImageCompositer
{
   public:
                              avtTiledImageCompositor();
      virtual                ~avtTiledImageCompositor();

      const char             *GetType(void);
      const char             *GetDescription(void);

      void                    SetChunkSize(const int chunkSize);
      int                     GetChunkSize() const;

      void                    Execute();

   private:
      int                     chunkSize;
};

inline const char* avtTiledImageCompositor::GetType()
{ return "avtTiledImageCompositor";}

inline const char* avtTiledImageCompositor::GetDescription()
{ return "performing tiled-image composite"; }

inline void avtTiledImageCompositor::SetChunkSize(const int _chunkSize)
{ chunkSize = _chunkSize; }

inline int avtTiledImageCompositor::GetChunkSize() const
{ return chunkSize; }

#endif
