// ************************************************************************* //
//                            avtImageCompositer.h                           //
// ************************************************************************* //

#ifndef AVT_IMAGE_COMPOSITER_H
#define AVT_IMAGE_COMPOSITER_H
#include <filters_exports.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtImage.h>
#include <avtImageToImageFilter.h>

// ****************************************************************************
//  Class: avtImageCompositer
//
//  Purpose:
//      This is a base class to support various kinds of image compositing
//      algorithms. This class is intended to provide an asbtract
//      interface for compositing images. When used within a parallel client,
//      this object can support composition of a single image rendered on
//      different processors into a complete image. Alternatively, it can also
//      handle sequential compositing of images from multiple sources on the
//      same processor, for example when two different engines are running and
//      serving up images to the viewer.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Sun Dec  4 17:07:52 PST 2005
//    Added description method.
//
// ****************************************************************************

class AVTFILTERS_API avtImageCompositer : public avtImageToImageFilter
{
   public:
                              avtImageCompositer();
      virtual                ~avtImageCompositer();

      virtual const char     *GetType(void) { return "avtImageCompositer"; };
      virtual const char     *GetDescription(void)
                                   { return "Compositing images."; };

      void                    SetOutputImageSize(const int numRows,
                                                 const int numCols);
      void                    GetOutputImageSize(int *numRows,
                                                 int *numCols) const;

      void                    AddImageInput(avtImage_p subImage,
                                 int rowOrigin, int colOrigin);

      void                    SetShouldOutputZBuffer(bool outputZBuffer);
      bool                    GetShouldOutputZBuffer();

      int                     SetRoot(const int mpiRank);
      int                     GetRoot() const;
      int                     GetRank() const;

#ifdef PARALLEL
      MPI_Comm                SetMPICommunicator(const MPI_Comm comm);
      MPI_Comm                GetMPICommunicator() const;
#endif

   protected:
      int                     outRows, outCols;
      bool                    shouldOutputZBuffer;
      std::vector<avtImage_p> inputImages;
      int                     mpiRoot;
      int                     mpiRank;
#ifdef PARALLEL
      MPI_Comm                mpiComm;
#endif

};

inline void avtImageCompositer::SetOutputImageSize(int numRows, int numCols)
{ outRows = numRows; outCols = numCols; }

inline void avtImageCompositer::SetShouldOutputZBuffer(bool outputZBuffer)
{ shouldOutputZBuffer = outputZBuffer; }

inline bool avtImageCompositer::GetShouldOutputZBuffer()
{ return shouldOutputZBuffer; }

inline int avtImageCompositer::GetRoot() const
{ return mpiRoot; }

inline int avtImageCompositer::GetRank() const
{ return mpiRank; }

#ifdef PARALLEL
inline MPI_Comm avtImageCompositer::GetMPICommunicator() const
{ return mpiComm; }
#endif

#endif
