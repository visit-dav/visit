// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <sstream>
using std::ostringstream;

// ****************************************************************************
//  Method: avtImageCompositer constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

avtImageCompositer::avtImageCompositer()
{
   outRows = -1;
   outCols = -1;
   shouldOutputZBuffer = false;
   mpiRoot = -1;
   mpiRank = 0;
#ifdef PARALLEL
   mpiComm = VISIT_MPI_COMM;
   MPI_Comm_rank(mpiComm, &mpiRank);
   mpiRoot = 0;
#endif
}


// ****************************************************************************
//  Method: avtImageCompositer destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageCompositer::~avtImageCompositer()
{
    inputImages.clear();
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
//
//  Modifications:
//
//      Burlen Loring, Thu Oct  8 10:47:58 PDT 2015
//      Report some information in the error message about what
//      the problem is.
//
// ****************************************************************************
void avtImageCompositer::AddImageInput(avtImage_p image,
                                      int rowOffset, int colOffset)
{
    avtImageRepresentation& imageRep = image->GetImage();
    int imageRows, imageCols;
    imageRep.GetSize(&imageRows, &imageCols);
    if ((imageRows + rowOffset > outRows) || (imageCols + colOffset > outCols))
    {
        ostringstream oss;
        oss << "Bounds error in avtImageCompositer::AddImageInput "
            << "imageRows = " << imageRows << " rowOffset = " << rowOffset
            << " outRows = " << outRows << " imageCols = " << imageCols
            << " colOffset = " << colOffset << " outCols = " <<  outCols;
        EXCEPTION1(ImproperUseException, oss.str().c_str());
    }
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
