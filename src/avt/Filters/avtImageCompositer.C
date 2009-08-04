/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
