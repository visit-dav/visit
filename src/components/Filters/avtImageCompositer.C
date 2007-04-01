// ************************************************************************* //
//                            avtImageCompositer.C                           //
// ************************************************************************* //
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtImageCompositer.h>
#include <avtParallel.h>
#include <ImproperUseException.h>

// ****************************************************************************
//  Method: avtImageCompositer constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
// ****************************************************************************

avtImageCompositer::avtImageCompositer()
{
   outRows = -1;
   outCols = -1;
   shouldOutputZBuffer = false;
   mpiRoot = -1;
   mpiRank = 0;
#ifdef PARALLEL
   mpiComm = MPI_COMM_WORLD;
   MPI_Comm_rank(mpiComm, &mpiRank);
   mpiRoot = 0;
#endif
}


// ****************************************************************************
//  Method: avtImageCompositer::GetOutputImageSize
//
//  Purpose:
//      Gets the output image size.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003 
// ****************************************************************************
void avtImageCompositer::GetOutputImageSize(int *numRows, int *numCols) const
{
   if (numRows != NULL)
      *numRows = outRows;
   if (numCols != NULL)
      *numCols = outCols;
}

// ****************************************************************************
//  Method: avtImageCompositer::AddImageInput
//
//  Purpose:
//      Sets an input's image data and offset in the output.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003 
// ****************************************************************************
void avtImageCompositer::AddImageInput(avtImage_p image,
                                      int rowOffset, int colOffset)
{
   avtImageRepresentation& imageRep = image->GetImage();
   int imageRows, imageCols;
   imageRep.GetSize(&imageRows, &imageCols);
   if ((imageRows + rowOffset > outRows) || (imageCols + colOffset > outCols))
      EXCEPTION0(ImproperUseException);
   imageRep.SetOrigin(rowOffset, colOffset);
   inputImages.push_back(image);
}

// ****************************************************************************
//  Method: avtImageCompositer::SetRoot
//
//  Purpose:
//      Set the root for a distributed composite
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003 
// ****************************************************************************
int avtImageCompositer::SetRoot(const int newRoot)
{
   int oldRoot = mpiRoot;
   if (newRoot < 0)
      mpiRoot = -1;
   else
   {   int commSize = 1;
#ifdef PARALLEL
       MPI_Comm_size(mpiComm, &commSize);
#endif
      if (newRoot >= commSize)
         EXCEPTION0(ImproperUseException);
      mpiRoot = newRoot;
   }
   return oldRoot;
}

#ifdef PARALLEL
// ****************************************************************************
//  Method: avtImageCompositer::SetMPICommunicator
//
//  Purpose:
//     Set the MPI communicator to be used in a distributed composite
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003 
// ****************************************************************************
MPI_Comm
avtImageCompositer::SetMPICommunicator(const MPI_Comm _mpiComm)
{
   MPI_Comm oldComm = mpiComm;
   mpiComm = _mpiComm;
   MPI_Comm_rank(mpiComm, &mpiRank);
   return oldComm;
}
#endif
